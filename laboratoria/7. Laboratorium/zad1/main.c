#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "lib/libcommon.h"


key_t key;
int sem_id = -1;
int shm_id = -1;
Pizzeria *pizzeria;

int no_chefs;
pid_t *chefs;
int no_deliverers;
pid_t *deliverers;

void sigint_handler(int sig_no);
void exit_handler(void);

void clear_pizzeria(void);
int initialize_semaphores(void);
int initialize_shared_memory(void);
int detach_shared_memory(void);
int employ_chefs(void);
int employ_deliverers(void);
int employ_people(char* program_path, int no_people, pid_t *pid_arr);
int wait_until_pizzeria_closes(void);
pid_t create_and_exec_child(char* path);


int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);

    // Exit handler
    if (atexit(exit_handler) == -1) {
        perror("Nie można ustawić funkcji obsługi wyjścia\n");
        return EXIT_FAILURE;
    }

    // Get N, M values
    if (argc != 3) {
        fprintf(stderr, "Zła liczba argumentów - powinny być 2\n");
        return EXIT_FAILURE;
    }

    no_chefs = (int) strtol(argv[1], NULL, 10);
    no_deliverers = (int) strtol(argv[2], NULL, 10);

    // Setup SIGINT handler
    set_sa_handler(SIGINT, 0, sigint_handler);

    // Create the restaurant
    bool is_error = false;
    if (initialize_semaphores() == -1
        || initialize_shared_memory() == -1
        || employ_chefs() == -1
        || employ_deliverers() == -1
        || wait_until_pizzeria_closes() == -1) {
        is_error = true;
    }

    // Detach shared memory
    if (shm_id && detach_shared_memory() == -1) is_error = true;

    return is_error ? EXIT_FAILURE : EXIT_SUCCESS;
}


void exit_handler(void) {
    // Kill chefs and clear the evidence (free the pid array)
    if (chefs) {
        for (int i = 0; i < no_chefs; i++) kill(chefs[i], SIGINT);
        free(chefs);
    }

    // Kill deliverers and clear the evidence (free the pid array)
    if (deliverers) {
        for (int i = 0; i < no_deliverers; i++) kill(deliverers[i], SIGINT);
        free(deliverers);
    }

    // Remove semaphores
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("Nie można usunąć semaforów\n");
        exit(EXIT_FAILURE);
    }

    // Remove share memory
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("Nie można usunąć pamięci współdzielonej\n");
        exit(EXIT_FAILURE);
    }

    usleep(100);
    puts("Pizzeria została pomyślnie zamknięta\n");
}

void sigint_handler(int sig_no) {
    printf("Otrzymałem sygnał %d. Zamykam pizzerię...\n", sig_no);
    exit(EXIT_SUCCESS);
}

void clear_pizzeria(void) {
    fill_array(pizzeria->oven_slots, OVEN_CAPACITY, -1);
    fill_array(pizzeria->table_slots, TABLE_CAPACITY, -1);
}

int initialize_semaphores(void) {
    // Generate the key
    if ((key = generate_key(SEMAPHORE_PROJ_ID)) == -1) return -1;

    // Open shared memory
    if ((sem_id = semget(key, 5, PERMISSIONS | IPC_CREAT)) == -1) {
        perror("Nie można stworzyć semaforów\n");
        return -1;
    }

    // Initialize semaphores
    if (semctl(sem_id, AVAILABLE_PLACES_IN_OVEN, SETVAL, (union semun) { .val = OVEN_CAPACITY }) == -1
        || semctl(sem_id, AVAILABLE_PLACES_ON_THE_TABLE, SETVAL, (union semun) { .val = TABLE_CAPACITY }) == -1
        || semctl(sem_id, NO_WAITING_PIZZAS, SETVAL, (union semun) { .val = 0 }) == -1
        || semctl(sem_id, NO_PIZZAS_IN_DELIVERY, SETVAL, (union semun) { .val = 0 }) == -1
        || semctl(sem_id, MEMORY_LOCK, SETVAL, (union semun) { .val = 1 }) == -1) {
        perror("Nie można zainicjalizować wartości semaforów\n");
        return -1;
    }

    return 0;
}

int initialize_shared_memory(void) {
    // Generate the key
    if ((key = generate_key(MEMORY_PROJ_ID)) == -1) return -1;

    // Create shared memory segment
    if ((shm_id = shmget(key, sizeof(Pizzeria), PERMISSIONS | IPC_CREAT)) == -1) {
        perror("Nie można stworzyć segmentu pamięci współdzielonej\n");
        return -1;
    }

    // Load pizzeria data struct
    if ((pizzeria = shmat(shm_id, NULL, 0)) == (void*) -1) {
        perror("Nie można załadować danych z pamięci współdzielonej\n");
        return -1;
    }

    // Make all slots in the oven and on the table empty
    clear_pizzeria();

    return 0;
}

int detach_shared_memory(void) {
    if (shmdt(pizzeria) == -1) {
        perror("Nie można odłączyć pamięci współdzielonej\n");
        return -1;
    }

    return 0;
}

int employ_chefs() {
    return employ_people(CHEF_PATH, no_chefs, chefs);
}

int employ_deliverers() {
    return employ_people(DELIVERER_PATH, no_deliverers, deliverers);
}

int employ_people(char* program_path, int no_people, pid_t *pid_arr) {
    if (!(pid_arr = (pid_t*) calloc(sizeof(pid_t), no_people))) {
        perror("Nie można zaalokować pamięci na id procesów pracowników\n");
        return -1;
    }

    for (int i = 0; i < no_people; i++) {
        if ((pid_arr[i] = create_and_exec_child(program_path)) == -1) {
            return -1;
        }
    }

    return 0;
}

pid_t create_and_exec_child(char* path) {
    pid_t pid;

    if ((pid = fork()) == -1) {
        perror("Nie można utworzyć procesu potomnego\n");
        return -1;
    }

    if (pid == 0) {
        execl(path, path, NULL);
        fprintf(stderr, "Wystąpił problem podczas uruchamiania procesu potomnego\n");
    }

    return pid;
}

int wait_until_pizzeria_closes() {
    for (int i = 0; i < no_chefs + no_deliverers; i++) {
        if (wait(NULL) == -1) {
            perror("Nie można zamknąć procesu pracownika\n");
            return -1;
        }
    }

    return 0;
}
