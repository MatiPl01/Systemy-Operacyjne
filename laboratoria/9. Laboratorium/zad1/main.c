#include <pthread.h>
#include "lib/libprint.h"
#include "lib/librandom.h"
#include "characters/elf.h"
#include "characters/santa.h"
#include "characters/reindeer.h"

/*
 * PROGRAM CONFIG
 */
// Elves
const int min_elf_working_time = 2;
const int max_elf_working_time = 5;
const int total_elves_count = 10;
const int max_elves_waiting_for_help = 3;

// Reindeer
const int min_reindeer_vacation_time = 5;
const int max_reindeer_vacation_time = 10;
const int total_reindeer_count = 9;

// Santa Claus
const int min_delivering_time = 2;
const int max_delivering_time = 4;
const int max_deliveries_count = 3;
const int min_solving_problem_time= 1;
const int max_solving_problem_time = 2;

/*
 * SHARED DATA
 */
int reindeer_waining_count = 0;
int elves_waiting_for_help_count = 0;
int elves_waiting_for_help_ids[3] = { 0 };

/*
 * THREADS-RELATED DATA
 */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_wake_up_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t presents_delivered_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_problem_solved_condition = PTHREAD_COND_INITIALIZER;

elf_args_t elf_args = {
        .min_working_time = min_elf_working_time,
        .max_working_time = max_elf_working_time,
        .max_elves_waiting_for_help = max_elves_waiting_for_help,
        .elves_waiting_for_help_count = &elves_waiting_for_help_count,
        .elves_waiting_for_help_ids = elves_waiting_for_help_ids,
        .mutex = &mutex,
        .santa_wake_up_condition = &santa_wake_up_condition,
        .elves_problem_solved_condition = &elves_problem_solved_condition
};

reindeer_args_t reindeer_args = {
        .min_vacation_time = min_reindeer_vacation_time,
        .max_vacation_time = max_reindeer_vacation_time,
        .total_reindeer_count = total_reindeer_count,
        .reindeer_waining_count = &reindeer_waining_count,
        .mutex = &mutex,
        .santa_wake_up_condition = &santa_wake_up_condition,
        .presents_delivered_condition = &presents_delivered_condition
};

santa_args_t santa_args = {
        .deliveries_count = 0,
        .total_reindeer_count = total_reindeer_count,
        .min_delivering_time = min_delivering_time,
        .max_delivering_time = max_delivering_time,
        .max_deliveries_count = max_deliveries_count,
        .max_elves_waiting_for_help = max_elves_waiting_for_help,
        .min_solving_problem_time = min_solving_problem_time,
        .max_solving_problem_time = max_solving_problem_time,
        .elves_waiting_for_help_count = &elves_waiting_for_help_count,
        .reindeer_waining_count = &reindeer_waining_count,
        .elves_waiting_for_help_ids = elves_waiting_for_help_ids,
        .mutex = &mutex,
        .santa_wake_up_condition = &santa_wake_up_condition,
        .presents_delivered_condition = &presents_delivered_condition,
        .elves_problem_solved_condition = &elves_problem_solved_condition
};


pthread_t create_thread(void* routine, void* args);
pthread_t create_elf(pthread_t threads[], int *thread_idx);
pthread_t create_reindeer(pthread_t threads[], int *thread_idx);


int main(void) {
//    printcf(ELF_MSG_COLOR, "Elf: czeka na powrót elfów, %d\n", 124);
//    printcf(ELF_MSG_COLOR, "Elf: czeka %d elfów na Mikołaja, %d\n", 245, 124);

    // Create character threads
    int thread_idx = 0;
    pthread_t threads[total_elves_count + total_reindeer_count];

    pthread_t santa_thread = create_thread(santa, (void*) &santa_args);
    if (santa_thread == -1) return EXIT_FAILURE;

    for (int i = 0; i < total_elves_count; i++) {
        if (create_elf(threads, &thread_idx) == -1) return EXIT_FAILURE;
    }

    for (int i = 0; i < total_reindeer_count; i++) {
        if (create_reindeer(threads, &thread_idx) == -1) return EXIT_FAILURE;
    }

    // Wait for the Santa Clause thread to finish
    if (pthread_join(santa_thread, NULL) != 0 ) {
        fprintf(stderr, "Cannot join the Santa Clause thread\n");
    }

    // Cancel all elf and reindeer threads
    for (int i = 0; i < total_elves_count + total_reindeer_count; i++) {
        if (pthread_cancel(threads[i]) != 0) {
            fprintf(stderr, "Cannot cancel thread\n");
        }
    }

    // Wait for elf and reindeer threads to finish
    for (int i = 0; i < total_elves_count + total_reindeer_count; i++) {
        void* returned_value = NULL;

        if (pthread_join(threads[i], &returned_value) != 0) {
            fprintf(stderr, "Cannot join the %s thread with idx %d\n",
                i < total_elves_count ? "elf" : "reindeer", i
            );
        } else if (returned_value != PTHREAD_CANCELED) {
            fprintf(stderr, "%s thread with idx %d finished unexpectedly before cancelling\n",
                    i < total_elves_count ? "elf" : "reindeer", i
            );
        }
    }

    puts("All done, it's time to rest!");

    return EXIT_SUCCESS;
}


pthread_t create_thread(void* routine, void* args) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, routine, args) != 0) {
        perror("Nie można utworzyć wątku\n");
        return -1;
    }
    return thread;
}

pthread_t create_elf(pthread_t threads[], int *thread_idx) {
    elf_args.id++;
    return threads[(*thread_idx)++] = create_thread(elf, (void*) &elf_args);
}

pthread_t create_reindeer(pthread_t threads[], int *thread_idx) {
    reindeer_args.id++;
    return threads[(*thread_idx)++] = create_thread(reindeer, (void*) &reindeer_args);
}
