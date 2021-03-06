#include "libcommon.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


int randint(int a, int b) {
    return a + rand() % (b - a);
}

void randsleep(int a, int b) {
    sleep(randint(a, b));
}

long double current_timestamp(void) {
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    time_t s = spec.tv_sec;
    int ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

    return s + ms / 1000.;
}

int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int)) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_handler = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        perror(" Nie można ustawić akcji dla sygnału\n");
        return -1;
    }

    return 0;
}

int find_empty_slot(int arr[], int arr_length) {
    for (int i = 0; i < arr_length; i++) {
        if (arr[i] == -1) return i;
    }
    return -1;
}

void fill_array(int arr[], int arr_length, int value) {
    for (int i = 0; i < arr_length; i++) arr[i] = value;
}

key_t generate_key(char proj_id) {
    char* home_path = getenv("HOME");
    key_t key = ftok(home_path, proj_id);
    if (key == -1) {
        perror("Nie można wygenerować klucza\n");
        return -1;
    }
    return key;
}

Pizzeria *load_pizzeria_data(int *sem_id) {
    // Open existing semaphores
    key_t key;
    if ((key = generate_key(SEMAPHORE_PROJ_ID)) == -1) return NULL;
    if ((*sem_id = semget(key, 0, 0)) == -1) {
        perror("Nie można otworzyć semaforów\n");
        return NULL;
    }

    // Open existing shared memory
    if ((key = generate_key(MEMORY_PROJ_ID)) == -1) return NULL;
    int shm_id;
    if ((shm_id = shmget(key, 0, 0)) == -1) {
        perror("Nie można otworzyć pamięci współdzielonej\n");
        return NULL;
    }

    // Load pizzeria data
    Pizzeria *pizzeria;
    if ((pizzeria = shmat(shm_id, NULL, 0)) == (void*) -1) {
        perror("Nie można załadować danych z pamięci współdzielonej\n");
        return NULL;
    }

    return pizzeria;
}

void sigint_handler(int sig_no) {
    printf("Otrzymałem sygnał %d. Kończę pracę...\n", sig_no);
    exit(EXIT_SUCCESS);
}
