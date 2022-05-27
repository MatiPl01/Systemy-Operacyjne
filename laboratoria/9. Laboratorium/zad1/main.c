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
// Condition variables
pthread_cond_t santa_wake_up_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t presents_delivered_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_solved_problem_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_started_solving_problem_condition = PTHREAD_COND_INITIALIZER;

// Mutexes
pthread_mutex_t elves_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elves_problem_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_delivery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_sleep_mutex = PTHREAD_MUTEX_INITIALIZER;


pthread_t create_thread(void* routine, void* args);
pthread_t create_elf(int id);
pthread_t create_reindeer(int id);
pthread_t create_santa(void);


int main(void) {
    // Create character threads
    pthread_t threads[total_elves_count + total_reindeer_count];

    pthread_t santa_thread = create_santa();
    if (santa_thread == -1) return EXIT_FAILURE;

    for (int i = 0; i < total_elves_count; i++) {
        if ((threads[i] = create_elf(i)) == -1) return EXIT_FAILURE;
    }

    for (int i = 0; i < total_reindeer_count; i++) {
        if ((threads[i + total_elves_count] = create_reindeer(i)) == -1) return EXIT_FAILURE;
    }

    // Wait for the Santa Clause thread to finish
    if (pthread_join(santa_thread, NULL) != 0 ) {
        fprintf(stderr, "Cannot join the Santa Clause thread\n");
    }

    // Wait for elf and reindeer threads to finish
    for (int i = 0; i < total_elves_count + total_reindeer_count; i++) {
        void* returned_value = NULL;

        if (pthread_join(threads[i], &returned_value) != 0) {
            fprintf(stderr, "Cannot join the %s thread with idx %d\n",
                i < total_elves_count ? "elf" : "reindeer", i
            );
        }
    }

    // Clear mutexes and condition variables
    pthread_mutex_destroy(&elves_wait_mutex);
    pthread_mutex_destroy(&reindeer_wait_mutex);
    pthread_mutex_destroy(&reindeer_delivery_mutex);
    pthread_mutex_destroy(&elves_problem_mutex);
    pthread_mutex_destroy(&santa_sleep_mutex);
    pthread_cond_destroy(&santa_wake_up_condition);
    pthread_cond_destroy(&presents_delivered_condition);
    pthread_cond_destroy(&santa_solved_problem_condition);
    pthread_cond_destroy(&santa_started_solving_problem_condition);
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

pthread_t create_elf(int id) {
    elf_args_t *args;
    if (!(args = malloc(sizeof(elf_args_t)))) {
        perror("Cannot allocate memory for the elf thread args\n");
        return -1;
    }

    args->id = id;
    args->min_working_time = min_elf_working_time,
    args->max_working_time = max_elf_working_time,
    args->max_elves_waiting_for_help = max_elves_waiting_for_help,
    args->elves_waiting_for_help_count = &elves_waiting_for_help_count,
    args->elves_waiting_for_help_ids = elves_waiting_for_help_ids,
    args->elves_wait_mutex = &elves_wait_mutex,
    args->elves_problem_mutex = &elves_problem_mutex,
    args->santa_sleep_mutex = &santa_sleep_mutex,
    args->santa_wake_up_condition = &santa_wake_up_condition,
    args->santa_solved_problem_condition = &santa_solved_problem_condition;
    args->santa_started_solving_problem_condition = &santa_started_solving_problem_condition;

    return create_thread(elf, (void*) args);
}

pthread_t create_reindeer(int id) {
    reindeer_args_t *args;
    if (!(args = malloc(sizeof(reindeer_args_t)))) {
        perror("Cannot allocate memory for the reindeer thread args\n");
        return -1;
    }

    args->id = id;
    args->min_vacation_time = min_reindeer_vacation_time;
    args->max_vacation_time = max_reindeer_vacation_time;
    args->total_reindeer_count = total_reindeer_count;
    args->reindeer_waiting_count = &reindeer_waining_count;
    args->reindeer_delivery_mutex = &reindeer_delivery_mutex;
    args->reindeer_wait_mutex = &reindeer_wait_mutex;
    args->santa_sleep_mutex = &santa_sleep_mutex;
    args->santa_wake_up_condition = &santa_wake_up_condition;
    args->presents_delivered_condition = &presents_delivered_condition;

    return create_thread(reindeer, (void*) args);
}

pthread_t create_santa(void) {
    santa_args_t *args;
    if (!(args = malloc(sizeof(santa_args_t)))) {
        perror("Cannot allocate memory for the Santa Claus thread args\n");
        return -1;
    }

    args->deliveries_count = 0;
    args->total_reindeer_count = total_reindeer_count;
    args->min_delivering_time = min_delivering_time;
    args->max_delivering_time = max_delivering_time;
    args->max_deliveries_count = max_deliveries_count;
    args->max_elves_waiting_for_help = max_elves_waiting_for_help;
    args->min_solving_problem_time = min_solving_problem_time;
    args->max_solving_problem_time = max_solving_problem_time;
    args->elves_waiting_for_help_count = &elves_waiting_for_help_count;
    args->reindeer_waining_count = &reindeer_waining_count;
    args->elves_waiting_for_help_ids = elves_waiting_for_help_ids;
    args->santa_sleep_mutex = &santa_sleep_mutex;
    args->reindeer_wait_mutex = &reindeer_wait_mutex;
    args->reindeer_delivery_mutex = &reindeer_delivery_mutex;
    args->elves_wait_mutex = &elves_wait_mutex;
    args->elves_problem_mutex = &elves_problem_mutex;
    args->santa_wake_up_condition = &santa_wake_up_condition;
    args->presents_delivered_condition = &presents_delivered_condition;
    args->santa_solved_problem_condition = &santa_solved_problem_condition;
    args->santa_started_solving_problem_condition = &santa_started_solving_problem_condition;

    return create_thread(santa, (void*) args);
}
