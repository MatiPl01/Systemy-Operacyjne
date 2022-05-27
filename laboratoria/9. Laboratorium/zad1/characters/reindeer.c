#include "reindeer.h"


static void reindeer_cleanup_handler(void* args) {
    reindeer_args_t *reindeer_args = args;

    // Unlock the mutex (if it is locked)
    pthread_mutex_unlock(reindeer_args->mutex);

    // Release thread args memory
    free(args);
}

_Noreturn void* reindeer(void* args) {
    reindeer_args_t *reindeer_args = args;
    int reindeer_id = reindeer_args->id;

    struct drand48_data rand_context;
    srand48_r(reindeer_args->seed, &rand_context);

    pthread_cleanup_push(reindeer_cleanup_handler, args)

    while (true) {
        // Reindeer is on vacation
        randsleep(&rand_context, reindeer_args->min_vacation_time, reindeer_args->max_vacation_time);

        // The reindeer returns to the North Pole
        // (therefore it will use the data structure shared between reindeer threads)
        pthread_mutex_lock(reindeer_args->mutex);

        // The reindeer returned so the number of reindeer waiting for
        // the Santa Clause should be increased
        (*(reindeer_args->reindeer_waining_count))++;
        printcf(REINDEER_MSG_COLOR, "Renifer: czeka %d reniferów na Mikołaja, %d\n",
                *(reindeer_args->reindeer_waining_count),
                reindeer_id
        );

        // If all reindeer are back, the current reindeer, which returned last,
        // will wake up Santa Claus
        if (*(reindeer_args->reindeer_waining_count) == reindeer_args->total_reindeer_count) {
            printcf(REINDEER_MSG_COLOR, "Renifer: wybudzam Mikołaja, %d\n", reindeer_id);
            // Let everyone know that Santa Claus was woken up
            pthread_cond_broadcast(reindeer_args->santa_wake_up_condition);
        }

        // All reindeer deliver presents with Santa Claus
        pthread_cond_wait(reindeer_args->presents_delivered_condition, reindeer_args->mutex);

        // Set the cancellation point
        pthread_testcancel();

        // Reindeer are going back on vacation (the mutex can be unlocked)
        pthread_mutex_unlock(reindeer_args->mutex);
    }

    // Free thread memory
    pthread_cleanup_pop(true);
}
