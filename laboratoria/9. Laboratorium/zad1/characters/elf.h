#ifndef ELF_H
#define ELF_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "common.h"
#include "../lib/libprint.h"
#include "../lib/librandom.h"

static const Color ELF_MSG_COLOR = CYAN;

typedef struct elf_args_t {
    // Private parameters
    int id;
    long seed;

    // Config
    int min_working_time;
    int max_working_time;
    int max_elves_waiting_for_help;

    // Shared parameters
    int *elves_waiting_for_help_count; // int pointer (to allow and reflect changes in different threads)
    int* elves_waiting_for_help_ids;  // array of ints

    // Threads-related parameters
    pthread_mutex_t *elves_wait_mutex;
    pthread_mutex_t *elves_problem_mutex;
    pthread_mutex_t *santa_sleep_mutex;
    pthread_cond_t *santa_wake_up_condition;
    pthread_cond_t *santa_solved_problem_condition;
    pthread_cond_t *santa_started_solving_problem_condition;
} elf_args_t;

_Noreturn void* elf(void* id_arg);

#endif // ELF_H
