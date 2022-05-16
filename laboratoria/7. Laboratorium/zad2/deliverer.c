#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "lib/libcommon.h"


int pizza_type = -1;
Pizzeria *pizzeria;
sem_t *available_places_on_the_table_id;
sem_t *no_waiting_pizzas_id;
sem_t *no_pizzas_in_delivery_id;
sem_t *memory_lock_id;

void exit_handler(void);
int open_semaphores(void);

int get_pizza_from_the_table(void);
void travel_to_the_client(void);
int finish_delivery(void);
void travel_to_the_pizzeria(void);
int work(void);


int main(void) {
    srand(time(NULL) + getpid());

    // Exit handler
    if (atexit(exit_handler) == -1) {
        perror("Nie można ustawić funkcji obsługi wyjścia\n");
        return EXIT_FAILURE;
    }

    // SIGINT handler
    if (set_sa_handler(SIGINT, 0, sigint_handler) == -1) {
        return EXIT_FAILURE;
    }

    // Open semaphores and load pizzeria data
    if (open_semaphores() == -1 || !(pizzeria = load_pizzeria_data())) {
        return EXIT_FAILURE;
    }

    // Start working
    if (work() == -1) return EXIT_FAILURE;
}


int open_semaphores(void) {
    if ((available_places_on_the_table_id = sem_open(AVAILABLE_PLACES_ON_THE_TABLE, O_RDWR)) == SEM_FAILED
     || (no_waiting_pizzas_id = sem_open(NO_WAITING_PIZZAS, O_RDWR)) == SEM_FAILED
     || (no_pizzas_in_delivery_id = sem_open(NO_PIZZAS_IN_DELIVERY, O_RDWR)) == SEM_FAILED
     || (memory_lock_id = sem_open(MEMORY_LOCK, O_RDWR)) == SEM_FAILED) {
        perror("Nie można otworzyć semaforów\n");
        return -1;
    }

    return 0;
}

void exit_handler(void) {
    // Close semaphores
    if ((available_places_on_the_table_id && sem_close(available_places_on_the_table_id) == -1)
     || (no_waiting_pizzas_id && sem_close(no_waiting_pizzas_id) == -1)
     || (no_pizzas_in_delivery_id && sem_close(no_pizzas_in_delivery_id) == -1)
     || (memory_lock_id && sem_close(memory_lock_id) == -1)) {
        perror("Nie można zamknąć semaforów");
        exit(EXIT_FAILURE);
    }

    // Detach shared memory
    if (munmap(pizzeria, sizeof(Pizzeria)) == -1) {
        perror("Nie można odłączyć pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    printf("Dostawca %d zakończył pracę\n", getpid());
}

int get_pizza_from_the_table(void) {
    // Lock memory, get pizza from the table, remove pizza from waiting and start delivering
    if (sem_wait(no_waiting_pizzas_id) == -1                // decrement
     || sem_wait(memory_lock_id) == -1
     || sem_post(available_places_on_the_table_id) == -1 // increment
     || sem_post(no_pizzas_in_delivery_id) == -1) {      // increment
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    // Find pizza on the table
    for (int i = 0; i < TABLE_CAPACITY; i++) {
        if (pizzeria->table_slots[i] != -1) {
            pizza_type = pizzeria->table_slots[i];
            pizzeria->table_slots[i] = -1;
            pizzeria->no_table_pizzas--;
            break;
        }
    }

    // Print info
    printf("(%d %.3Lf) Pobieram pizzę: %d Liczba pizz na stole: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type,
           pizzeria->no_table_pizzas
    );

    // Unlock memory
    if (sem_post(memory_lock_id) == -1) {
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    return 0;
}

void travel_to_the_client(void) {
    randsleep(MIN_TRAVEL_TO_CLIENT_TIME, MAX_TRAVEL_TO_CLIENT_TIME);
}

int finish_delivery(void) {
    // Decrement the number of pizzas in delivery
    if (sem_wait(no_pizzas_in_delivery_id) == -1) { // decrement
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    // Print info
    printf("(%d %.3Lf) Dostarczam pizzę: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type
    );

    pizza_type = -1;

    return 0;
}

void travel_to_the_pizzeria(void) {
    randsleep(MIN_TRAVEL_TO_PIZZERIA_TIME, MAX_TRAVEL_TO_PIZZERIA_TIME);
}

int work(void) {
    while (true) {
        if (get_pizza_from_the_table() == -1) return -1;
        travel_to_the_client();
        if (finish_delivery() == -1) return -1;
        travel_to_the_pizzeria();
    }
}
