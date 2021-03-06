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

    // Load pizzeria data
    if (!(pizzeria = load_pizzeria_data(&sem_id))) return EXIT_FAILURE;

    // Start working
    if (work() == -1) return EXIT_FAILURE;
}


void exit_handler(void) {
    if (pizzeria && shmdt(pizzeria) == -1) {
        perror("Nie można odłączyć pamięci współdzielonej\n");
        exit(EXIT_FAILURE);
    }
    printf("Dostawca %d zakończył pracę\n", getpid());
}

int get_pizza_from_the_table(void) {
    // Lock memory, get pizza from the table, remove pizza from waiting and start delivering
    if (semop(sem_id, (struct sembuf[]) {
            lock_memory_options,
            get_pizza_from_the_table_options,
            remove_pizza_from_waiting_options,
            start_pizza_delivery_options
    }, 4) == -1) {
        perror("Nie można wykonać operacji na zbiorze semaforów\n");
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
    if (semop(sem_id, (struct sembuf[]) {
            unlock_memory_options
    }, 1) == -1) {
        perror("Nie można wykonać operacji na zbiorze semaforów\n");
        return -1;
    }

    return 0;
}

void travel_to_the_client(void) {
    randsleep(MIN_TRAVEL_TO_CLIENT_TIME, MAX_TRAVEL_TO_CLIENT_TIME);
}

int finish_delivery(void) {
    // Decrement the number of pizzas in delivery
    if (semop(sem_id, (struct sembuf[]) {
            finish_pizza_delivery_options
    }, 1) == -1) {
        perror("Nie można wykonać operacji na zbiorze semaforów\n");
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
