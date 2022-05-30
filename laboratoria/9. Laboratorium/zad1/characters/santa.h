#ifndef SANTA_H
#define SANTA_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "../lib/libcommon.h"
#include "../lib/libprint.h"
#include "../lib/librandom.h"

static const Color SANTA_MSG_COLOR = MAGENTA;

typedef struct santa_args_t {
    // Private parameters
    long seed;
    int deliveries_count;

    // Config
    int total_reindeer_count;
    int min_delivery_duration;
    int max_delivery_duration;
    int max_deliveries_count;

    int max_elves_waiting_for_help;
    int min_problem_solving_duration;
    int max_problem_solving_duration;

    // Shared parameters
    int *elves_waiting_for_help_count; // int pointer (to allow and reflect changes in different threads)
    int *reindeer_waining_count;      // int pointer (to allow and reflect changes in different threads)
    int* elves_waiting_for_help_ids; // array of ints

    // Threads-related parameters
    pthread_mutex_t *santa_sleep_mutex;
    pthread_mutex_t *reindeer_delivery_mutex;
    pthread_mutex_t *reindeer_wait_mutex;
    pthread_mutex_t *elves_wait_mutex;
    pthread_mutex_t *elves_problem_mutex;
    pthread_mutex_t *santa_started_solving_problem_mutex;
    pthread_cond_t *santa_wake_up_condition;
    pthread_cond_t *presents_delivered_condition;
    pthread_cond_t *santa_solved_problem_condition;
    pthread_cond_t *santa_started_solving_problem_condition;
} santa_args_t;

void* santa(void* id_arg);

#endif // SANTA_H
