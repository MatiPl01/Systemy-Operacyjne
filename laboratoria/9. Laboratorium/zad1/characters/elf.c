#include "elf.h"


static void elf_cleanup_handler(void* args) {
    elf_args_t *elf_args = args;

    // Unlock the mutex (if it is locked)
    pthread_mutex_unlock(elf_args->mutex);

    // Release thread args memory
    free(args);
}

_Noreturn void* elf(void* args) {
    elf_args_t *elf_args = args;
    int elf_id = elf_args->id;

    struct drand48_data rand_context;
    srand48_r(elf_args->seed, &rand_context);

    pthread_cleanup_push(elf_cleanup_handler, args)

    while (true) {
        // Elf is working
        randsleep(&rand_context, elf_args->min_working_time, elf_args->max_working_time);

        // Elf is going to report a problem
        // (therefore it will use the data structure shared between elf threads)
        pthread_mutex_lock(elf_args->mutex);

        // Wait until there are less than MAX_ELVES_WAITING_FOR_HELP waiting for help
        while (*(elf_args->elves_waiting_for_help_count) >= elf_args->max_elves_waiting_for_help) {
            printcf(ELF_MSG_COLOR, "Elf: czeka na powrót elfów, %d\n", elf_id);
            pthread_cond_wait(elf_args->elves_problem_solved_condition, elf_args->mutex);
        }

        // Add the current elf to elves waiting for help
        elf_args->elves_waiting_for_help_ids[*(elf_args->elves_waiting_for_help_count)++] = elf_id;
        printcf(ELF_MSG_COLOR, "Elf: czeka %d elfów na Mikołaja, %d\n",
                *(elf_args->elves_waiting_for_help_count),
                elf_id
        );

        // If the current elf is the last elf of MAX_ELVES_WAITING_FOR_HELP allowed to ask for help,
        // this elf is going to wake up Santa Claus
        if (*(elf_args->elves_waiting_for_help_count) == elf_args->max_elves_waiting_for_help) {
            printcf(ELF_MSG_COLOR, "Elf: wybudzam Mikołaja, %d\n", elf_id);
            // Let everyone know that Santa Claus was woken up
            pthread_cond_broadcast(elf_args->santa_wake_up_condition);
        }

        // Wait until Santa Claus solved the problem
        pthread_cond_wait(elf_args->elves_problem_solved_condition, elf_args->mutex);
        printcf(ELF_MSG_COLOR, "Elf: Mikołaj rozwiązuje problem, %d\n", elf_id);

        // Set the cancellation point
        pthread_testcancel();

        // Go back to work (unlock the mutex)
        pthread_mutex_unlock(elf_args->mutex);
    }

    // Free thread memory
    pthread_cleanup_pop(true);
}
