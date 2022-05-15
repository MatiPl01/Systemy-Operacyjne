#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "lib/libcommon.h"


int sem_id;
int pizza_type = -1;
Pizzeria *pizzeria;

void exit_handler(void);

void get_pizza_from_the_table(void);
void travel_to_the_client(void);
void finish_delivery(void);
void travel_to_the_pizzeria(void);
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

    return EXIT_SUCCESS;
}

void exit_handler(void) {
    if (pizzeria && shmdt(pizzeria) == -1) {
        perror("Nie można odłączyć pamięci współdzielonej\n");
        exit(EXIT_FAILURE);
    }
    printf("Dostawca %d zakończył pracę\n", getpid());
}

void get_pizza_from_the_table(void) {
    // Lock the table, get pizza from the table, remove pizza from waiting and start delivering
    semop(sem_id, (struct sembuf[]) {
            lock_memory_options,
            get_pizza_from_the_table_options,
            remove_pizza_from_waiting_options,
            start_pizza_delivery_options
    }, 4);

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
    printf("(%d %lld) Pobieram pizzę: %d Liczba pizz na stole: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type,
           pizzeria->no_table_pizzas
    );

    // Unlock the table
    semop(sem_id, (struct sembuf[]) {
            unlock_memory_options
    }, 1);

}

void travel_to_the_client(void) {
    randsleep(MIN_TRAVEL_TO_CLIENT_TIME, MAX_TRAVEL_TO_CLIENT_TIME);
}

void finish_delivery(void) {
    semop(sem_id, (struct sembuf[]) {
            finish_pizza_delivery_options
    }, 1);

    // Print info
    printf("(%d %lld) Dostarczam pizzę: %d\n",
           getpid(),
           current_timestamp(),
           pizza_type
    );

    pizza_type = -1;
}

void travel_to_the_pizzeria(void) {
    randsleep(MIN_TRAVEL_TO_PIZZERIA_TIME, MAX_TRAVEL_TO_PIZZERIA_TIME);
}

void work(void) {
    while (true) {
        get_pizza_from_the_table();
        travel_to_the_client();
        finish_delivery();
        travel_to_the_pizzeria();
    }
}
