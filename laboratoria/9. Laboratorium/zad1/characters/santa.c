#include "santa.h"


static void santa_sleep(santa_args_t *santa_args);
static void santa_wake_up(void);
static void deliver_presents(struct drand48_data rand_context, santa_args_t *santa_args);
static void solve_elves_problem(struct drand48_data rand_context, santa_args_t *santa_args);
static void fall_asleep(void);


void* santa(void* args) {
    santa_args_t *santa_args = args;

    struct drand48_data rand_context;
    srand48_r(santa_args->seed, &rand_context);

    pthread_cleanup_push(thread_cleanup_handler, args)

    while (santa_args->deliveries_count < santa_args->max_deliveries_count) {
        santa_sleep(santa_args);

        // Check who woke up Santa Claus
        // (check reindeer at first as children waiting for presents are more important than elves)
        if (*(santa_args->reindeer_waining_count) == santa_args->total_reindeer_count) {
            deliver_presents(rand_context, santa_args);
        }
        // Otherwise, help elves to solve the problem in the workshop
        else if (*(santa_args->elves_waiting_for_help_count) == santa_args->max_elves_waiting_for_help) {
            solve_elves_problem(rand_context, santa_args);
        }

        fall_asleep();
    }

    // Free thread memory
    pthread_cleanup_pop(true);

    exit(EXIT_SUCCESS);
}


static void santa_sleep(santa_args_t *santa_args) {
    pthread_mutex_lock(santa_args->santa_sleep_mutex);
    // Sleep until elves have a problem or all reindeer are back
    while (*(santa_args->elves_waiting_for_help_count) < santa_args->max_elves_waiting_for_help &&
           *(santa_args->reindeer_waining_count) < santa_args->total_reindeer_count) {
        pthread_cond_wait(santa_args->santa_wake_up_condition, santa_args->santa_sleep_mutex);
        santa_wake_up();
    }
    pthread_mutex_unlock(santa_args->santa_sleep_mutex);
}

static void santa_wake_up(void) {
    // Santa Claus is waking up
    printcf(SANTA_MSG_COLOR, "Mikołaj: Budzę się\n");
}

static void deliver_presents(struct drand48_data rand_context, santa_args_t *santa_args) {
    printcf(SANTA_MSG_COLOR, "Mikołaj: dostarczam zabawki\n");
    randsleep(&rand_context, santa_args->min_delivering_time, santa_args->max_delivering_time);
    // Increase the number of presents deliveries
    santa_args->deliveries_count++;
    // Reduce the number of reindeer waiting for Santa Claus to 0
    // (because all reindeer finished delivering presents and went on vacation)
    pthread_mutex_lock(santa_args->reindeer_wait_mutex);
    *(santa_args->reindeer_waining_count) = 0;
    // Let everyone know that presents were successfully delivered
    pthread_cond_broadcast(santa_args->presents_delivered_condition);
    pthread_mutex_unlock(santa_args->reindeer_wait_mutex);
}

static void solve_elves_problem(struct drand48_data rand_context, santa_args_t *santa_args) {
    // Let everyone know that Santa Claus starts solving elves problem
    pthread_mutex_lock(santa_args->elves_problem_mutex);
    pthread_cond_broadcast(santa_args->santa_started_solving_problem_condition);
    printcf(SANTA_MSG_COLOR, "Mikołaj: rozwiązuję problemy elfów");
    for (int i = 0; i < *(santa_args->elves_waiting_for_help_count); i++) {
        printcf(SANTA_MSG_COLOR, " %d", santa_args->elves_waiting_for_help_ids[i]);
    }
    puts("");
    pthread_mutex_unlock(santa_args->elves_problem_mutex);

    randsleep(&rand_context, santa_args->min_solving_problem_time, santa_args->max_solving_problem_time);

    pthread_mutex_lock(santa_args->elves_wait_mutex);
    // Reduce the number of elves waiting for help to 0
    // (because Santa Claus helped all elves waiting for help)
    *(santa_args->elves_waiting_for_help_count) = 0;
    // Let everyone know that Santa Claus helped elves solve the problem
    pthread_cond_broadcast(santa_args->santa_solved_problem_condition);
    pthread_mutex_unlock(santa_args->elves_wait_mutex);
}

static void fall_asleep(void) {
    // Santa Claus is going back to sleep after having done all the work
    printcf(SANTA_MSG_COLOR, "Mikołaj: zasypiam\n");
}
