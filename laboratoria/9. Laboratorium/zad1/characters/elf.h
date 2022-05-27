#ifndef ELF_H
#define ELF_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "../lib/libprint.h"
#include "../lib/librandom.h"

static const Color ELF_MSG_COLOR = CYAN;

typedef struct elf_args_t {
    // Private parameters
    int id;
    long seed;

    // Constants
    const int min_working_time;
    const int max_working_time;
    const int max_elves_waiting_for_help;

    // Shared parameters
    int *elves_waiting_for_help_count; // int pointer (to allow and reflect changes in different threads)
    int* elves_waiting_for_help_ids;  // array of ints

    // Threads-related parameters
    pthread_mutex_t *mutex;
    pthread_cond_t *santa_wake_up_condition;
    pthread_cond_t *elves_problem_solved_condition;
} elf_args_t;

_Noreturn void* elf(void* id_arg);

#endif // ELF_H
