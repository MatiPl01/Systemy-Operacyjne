#include "santa.h"


static void reindeer_cleanup_handler(void* args) {
    santa_args_t *santa_args = args;

    // Unlock the mutex (if it is locked)
    pthread_mutex_unlock(santa_args->mutex);

    // Release thread args memory
    free(args);
}

void* santa(void* args) {
    santa_args_t *santa_args = args;

    struct drand48_data rand_context;
    srand48_r(santa_args->seed, &rand_context);

    pthread_cleanup_push(reindeer_cleanup_handler, args)

    while (true) {
        // Sleep until elves have a problem or all reindeer are back
        while (*(santa_args->elves_waiting_for_help_count) < santa_args->max_elves_waiting_for_help &&
              *(santa_args->reindeer_waining_count) < santa_args->total_reindeer_count) {
           pthread_cond_wait(santa_args->santa_wake_up_condition, santa_args->mutex);
        }

        // Santa Claus is waking up
        printcf(SANTA_MSG_COLOR, "Mikołaj: Budzę się\n");

        // Check who woke up Santa Claus
        // (check reindeer at first as children waiting for presents are more important than elves)
        if (*(santa_args->reindeer_waining_count) == santa_args->total_reindeer_count) {
            printcf(SANTA_MSG_COLOR, "Mikołaj: dostarczam zabawki\n");
            randsleep(&rand_context, santa_args->min_delivering_time, santa_args->max_delivering_time);
            // Reduce the number of reindeer waiting for Santa Claus to 0
            // (because all reindeer finished delivering presents and went on vacation)
            *(santa_args->reindeer_waining_count) = 0;
            // Let everyone know that presents were successfully delivered
            pthread_cond_broadcast(santa_args->presents_delivered_condition);
            // Increase the number of presents deliveries and check if Santa Claus
            // reached the max number of deliveries
            if (++santa_args->deliveries_count == santa_args->max_deliveries_count) break;

        // Otherwise, help elves to solve the problem in the workshop
        } else {
            printcf(SANTA_MSG_COLOR, "Mikołaj: rozwiązuję problemy elfów");
            for (int i = 0; i < *(santa_args->elves_waiting_for_help_count); i++) {
               printcf(SANTA_MSG_COLOR, " %d", santa_args->elves_waiting_for_help_ids[i]);
            }
            puts("");
            randsleep(&rand_context, santa_args->min_solving_problem_time, santa_args->max_solving_problem_time);
            // Reduce the number of elves waiting for help to 0
            // (because Santa Claus helped all elves waiting for help)
            *(santa_args->elves_waiting_for_help_count) = 0;
            // Let everyone know that Santa Claus helped elves solve the problem
            pthread_cond_broadcast(santa_args->elves_problem_solved_condition);
        }

        // Santa Claus is going back to sleep after having done all the work
        printcf(SANTA_MSG_COLOR, "Mikołaj: zasypiam\n");

        // Set the cancellation point
        pthread_testcancel();

        // Santa Claus is going back to sleep (the mutex can be unlocked)
        pthread_mutex_unlock(santa_args->mutex);
    }

    // Free thread memory
    pthread_cleanup_pop(true);

    return NULL;
}
