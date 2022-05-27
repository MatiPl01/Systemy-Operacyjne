#include "elf.h"


static void work(struct drand48_data rand_context, elf_args_t *elf_args);
static void wait_until_can_report_problem(elf_args_t *elf_args);
static void report_problem(elf_args_t *elf_args);
static void wait_until_santa_starts_solving_problem(elf_args_t *elf_args);


_Noreturn void* elf(void* args) {
    elf_args_t *elf_args = args;

    struct drand48_data rand_context;
    srand48_r(elf_args->seed, &rand_context);

    pthread_cleanup_push(thread_cleanup_handler, args)

    while (true) {
        work(rand_context, elf_args);
        wait_until_can_report_problem(elf_args);
        report_problem(elf_args);
        wait_until_santa_starts_solving_problem(elf_args);
    }

    // Free thread memory
    pthread_cleanup_pop(true);
}


static void work(struct drand48_data rand_context, elf_args_t *elf_args) {
    randsleep(&rand_context, elf_args->min_working_time, elf_args->max_working_time);
}

static void wait_until_can_report_problem(elf_args_t *elf_args) {
    pthread_mutex_lock(elf_args->elves_wait_mutex);
    // Wait until there are less than MAX_ELVES_WAITING_FOR_HELP waiting for help
    while (*(elf_args->elves_waiting_for_help_count) == elf_args->max_elves_waiting_for_help) {
        printcf(ELF_MSG_COLOR, "Elf: czeka na powrót elfów, %d\n", elf_args->id);
        pthread_cond_wait(elf_args->santa_solved_problem_condition, elf_args->elves_wait_mutex);
    }
    pthread_mutex_unlock(elf_args->elves_wait_mutex);
}

static void report_problem(elf_args_t *elf_args) {
    pthread_mutex_lock(elf_args->elves_problem_mutex);
    // Add the current elf to elves waiting for help
    if (*(elf_args->elves_waiting_for_help_count) < elf_args->max_elves_waiting_for_help) {
        elf_args->elves_waiting_for_help_ids[(*(elf_args->elves_waiting_for_help_count))++] = elf_args->id;
        printcf(ELF_MSG_COLOR, "Elf: czeka %d elfów na Mikołaja, %d\n",
                *(elf_args->elves_waiting_for_help_count),
                elf_args->id
        );

        // If the current elf is the last elf of MAX_ELVES_WAITING_FOR_HELP allowed to ask for help,
        // this elf is going to wake up Santa Claus
        if (*(elf_args->elves_waiting_for_help_count) == elf_args->max_elves_waiting_for_help) {
            printcf(ELF_MSG_COLOR, "Elf: wybudzam Mikołaja, %d\n", elf_args->id);
            // Let everyone know that Santa Claus was woken up
            pthread_mutex_lock(elf_args->santa_sleep_mutex);
            pthread_cond_broadcast(elf_args->santa_wake_up_condition);
            pthread_mutex_unlock(elf_args->santa_sleep_mutex);
        }
    }
    pthread_mutex_unlock(elf_args->elves_problem_mutex);
}

static void wait_until_santa_starts_solving_problem(elf_args_t *elf_args) {
    pthread_mutex_lock(elf_args->elves_problem_mutex);
    // Wait until Santa Claus solved the problem
    pthread_cond_wait(elf_args->santa_started_solving_problem_condition, elf_args->elves_problem_mutex);
    if (elf_args->id == elf_args->elves_waiting_for_help_ids[elf_args->max_elves_waiting_for_help - 1]) {
        printcf(ELF_MSG_COLOR, "Elf: Mikołaj rozwiązuje problem, %d\n", elf_args->id);
    }
    pthread_mutex_unlock(elf_args->elves_problem_mutex);
}
