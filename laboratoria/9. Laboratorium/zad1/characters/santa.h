#ifndef SANTA_H
#define SANTA_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "../lib/libprint.h"
#include "../lib/librandom.h"

static const Color SANTA_MSG_COLOR = MAGENTA;

typedef struct santa_args_t {
    // Private parameters
    long seed;
    int deliveries_count;

    // Constants
    const int total_reindeer_count;
    const int min_delivering_time;
    const int max_delivering_time;
    const int max_deliveries_count;

    const int max_elves_waiting_for_help;
    const int min_solving_problem_time;
    const int max_solving_problem_time;

    // Shared parameters
    int *elves_waiting_for_help_count; // int pointer (to allow and reflect changes in different threads)
    int *reindeer_waining_count;      // int pointer (to allow and reflect changes in different threads)
    int* elves_waiting_for_help_ids; // array of ints

    // Threads-related parameters
    pthread_mutex_t *mutex;
    pthread_cond_t *santa_wake_up_condition;
    pthread_cond_t *presents_delivered_condition;
    pthread_cond_t *elves_problem_solved_condition;
} santa_args_t;

void* santa(void* id_arg);

#endif // SANTA_H
