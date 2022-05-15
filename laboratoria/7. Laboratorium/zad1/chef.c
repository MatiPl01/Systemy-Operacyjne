#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "lib/libcommon.h"


int sem_id;
int oven_idx;
int table_idx;
Pizzeria *pizzeria;

void exit_handler(void);

int choose_pizza_type(void);
void prepare_pizza(int pizza_type);
void put_pizza_in_oven(int pizza_type);
void bake_pizza(void);
void take_pizza_out_of_the_oven_and_put_on_the_table(void);
void work(void);


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

    // Load pizzeria data
    if (!(pizzeria = load_pizzeria_data(&sem_id))) return EXIT_FAILURE;

    // Start working
    work();
}

void exit_handler(void) {
    if (pizzeria && shmdt(pizzeria) == -1) {
        perror("Nie można odłączyć pamięci współdzielonej\n");
        exit(EXIT_FAILURE);
    }
    printf("Kucharz %d zakończył pracę\n", getpid());
}

int choose_pizza_type(void) {
    return randint(MIN_PIZZA_TYPE, MAX_PIZZA_TYPE);
}

void prepare_pizza(int pizza_type) {
    printf("(%d %lld) Przygotowuję pizzę: %d\n", getpid(), current_timestamp(), pizza_type);
    randsleep(MIN_PIZZA_PREPARATION_TIME, MAX_PIZZA_PREPARATION_TIME);
}

void put_pizza_in_oven(int pizza_type) {
    // Lock memory and put a pizza in the oven
    semop(sem_id, (struct sembuf[]) {
            lock_memory_options,
            put_pizza_in_oven_options
    }, 2);

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
    printf("(%d %lld) Dodałem pizzę: %d. Liczba pizz w piecu: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type,
           pizzeria->no_oven_pizzas
    );

    // Unlock memory
    semop(sem_id, (struct sembuf[]) {
            unlock_memory_options
    }, 1);
}

void bake_pizza(void) {
    randsleep(MIN_PIZZA_BAKE_TIME, MAX_PIZZA_BAKE_TIME);
}

void take_pizza_out_of_the_oven_and_put_on_the_table(void) {
    // Lock memory and take pizza out of the oven
    semop(sem_id, (struct sembuf[]) {
            lock_memory_options,
            take_pizza_out_of_oven_options,
            put_pizza_on_the_table_options,
            add_pizza_to_waiting_options
    },4);

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
    printf("(%d %lld) Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type,
           pizzeria->no_oven_pizzas,
           pizzeria->no_table_pizzas
    );

    // Unlock memory
    semop(sem_id, (struct sembuf[]) {
            unlock_memory_options
    }, 1);
}

void work(void) {
    while (true) {
        int pizza_type = choose_pizza_type();
        prepare_pizza(pizza_type);
        put_pizza_in_oven(pizza_type);
        bake_pizza();
        take_pizza_out_of_the_oven_and_put_on_the_table();
    }
}
