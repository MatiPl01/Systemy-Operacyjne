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


int oven_idx;
int table_idx;
Pizzeria *pizzeria;
sem_t *available_places_in_oven_id;
sem_t *available_places_on_the_table_id;
sem_t *no_waiting_pizzas_id;
sem_t *memory_lock_id;

void exit_handler(void);
int open_semaphores(void);

int choose_pizza_type(void);
void prepare_pizza(int pizza_type);
int put_pizza_in_oven(int pizza_type);
void bake_pizza(void);
int take_pizza_out_of_the_oven_and_put_on_the_table(void);  // Very short name :)
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
    if ((available_places_in_oven_id = sem_open(AVAILABLE_PLACES_IN_OVEN, O_RDWR)) == SEM_FAILED
     || (available_places_on_the_table_id = sem_open(AVAILABLE_PLACES_ON_THE_TABLE, O_RDWR)) == SEM_FAILED
     || (no_waiting_pizzas_id = sem_open(NO_WAITING_PIZZAS, O_RDWR)) == SEM_FAILED
     || (memory_lock_id = sem_open(MEMORY_LOCK, O_RDWR)) == SEM_FAILED) {
        perror("Nie można otworzyć semaforów\n");
        return -1;
    }

    return 0;
}

void exit_handler(void) {
    // Close semaphores
    if ((available_places_in_oven_id && sem_close(available_places_in_oven_id) == -1)
     || (available_places_on_the_table_id && sem_close(available_places_on_the_table_id) == -1)
     || (no_waiting_pizzas_id && sem_close(no_waiting_pizzas_id) == -1)
     || (memory_lock_id && sem_close(memory_lock_id) == -1)) {
        perror("Nie można zamknąć semaforów");
        exit(EXIT_FAILURE);
    }

    // Detach shared memory
    if (munmap(pizzeria, sizeof(Pizzeria)) == -1) {
        perror("Nie można odłączyć pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    printf("Kucharz %d zakończył pracę\n", getpid());
}

int choose_pizza_type(void) {
    return randint(MIN_PIZZA_TYPE, MAX_PIZZA_TYPE);
}

void prepare_pizza(int pizza_type) {
    printf("(%d %.3Lf) Przygotowuję pizzę: %d\n", getpid(), current_timestamp(), pizza_type);
    randsleep(MIN_PIZZA_PREPARATION_TIME, MAX_PIZZA_PREPARATION_TIME);
}

int put_pizza_in_oven(int pizza_type) {
    // Lock memory and put zad2 pizza in the oven
    if (sem_wait(available_places_in_oven_id) == -1  // decrement
     || sem_wait(memory_lock_id) == -1) {
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    // Find empty oven slot index
    if (pizzeria->no_oven_pizzas == OVEN_CAPACITY) {
        // This error should never happen
        fprintf(stderr, "\nPizzeria się spaliła!\n\n");
        exit(EXIT_FAILURE);
    }
    oven_idx = find_empty_slot(pizzeria->oven_slots, OVEN_CAPACITY);

    // Update the oven struct
    pizzeria->oven_slots[oven_idx] = pizza_type;
    pizzeria->no_oven_pizzas++;

    // Print info
    printf("(%d %.3Lf) Dodałem pizzę: %d. Liczba pizz w piecu: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type,
           pizzeria->no_oven_pizzas
    );

    // Unlock memory
    if (sem_post(memory_lock_id) == -1) {
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    return 0;
}

void bake_pizza(void) {
    randsleep(MIN_PIZZA_BAKE_TIME, MAX_PIZZA_BAKE_TIME);
}

int take_pizza_out_of_the_oven_and_put_on_the_table(void) {
    // Lock memory, take pizza out of the oven, put pizza on the table and mark as waiting for zad2 delivery
    if (sem_wait(available_places_on_the_table_id) == -1   // decrement
     || sem_wait(memory_lock_id) == -1
     || sem_post(available_places_in_oven_id) == -1        // increment
     || sem_post(no_waiting_pizzas_id) == -1) {            // increment
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    // Update the oven struct
    int pizza_type = pizzeria->oven_slots[oven_idx];
    pizzeria->oven_slots[oven_idx] = -1;
    pizzeria->no_oven_pizzas--;

    // Find empty table slot index
    if (pizzeria->no_table_pizzas == TABLE_CAPACITY) {
        // This error should never happen
        fprintf(stderr, "\nStół się ugiął od nadmiaru pizz!\n\n");
        exit(EXIT_FAILURE);
    }
    table_idx = find_empty_slot(pizzeria->table_slots, TABLE_CAPACITY);

    // Update the table struct
    pizzeria->table_slots[table_idx] = pizza_type;
    pizzeria->no_table_pizzas++;

    // Print info
    printf("(%d %.3Lf) Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type,
           pizzeria->no_oven_pizzas,
           pizzeria->no_table_pizzas
    );

    // Unlock memory
    if (sem_post(memory_lock_id) == -1) {
        perror("Nie można wykonać operacji na semaforach\n");
        return -1;
    }

    return 0;
}

int work(void) {
    while (true) {
        int pizza_type = choose_pizza_type();
        prepare_pizza(pizza_type);
        if (put_pizza_in_oven(pizza_type) == -1) return -1;
        bake_pizza();
        if (take_pizza_out_of_the_oven_and_put_on_the_table()  == -1) return -1;
    }
}
