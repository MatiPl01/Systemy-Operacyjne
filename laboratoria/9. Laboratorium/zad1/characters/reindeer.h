#ifndef REINDEER_H
#define REINDEER_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "common.h"
#include "../lib/libprint.h"
#include "../lib/librandom.h"

static const Color REINDEER_MSG_COLOR = YELLOW;

typedef struct reindeer_args_t {
    // Private parameters
    int id;
    long seed;

    // Config
    int min_vacation_time;
    int max_vacation_time;
    int total_reindeer_count;

    // Shared parameters
    int *reindeer_waiting_count; // int pointer (to allow and reflect changes in different threads)

    // Threads-related parameters
    pthread_mutex_t *reindeer_wait_mutex;
    pthread_mutex_t *reindeer_delivery_mutex;
    pthread_mutex_t *santa_sleep_mutex;
    pthread_cond_t *santa_wake_up_condition;
    pthread_cond_t *presents_delivered_condition;
} reindeer_args_t;

_Noreturn void* reindeer(void* id_arg);

#endif // REINDEER_H
