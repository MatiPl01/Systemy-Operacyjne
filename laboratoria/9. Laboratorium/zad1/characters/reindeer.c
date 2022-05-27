#include "reindeer.h"


static void rest_on_holiday(struct drand48_data rand_context, reindeer_args_t *reindeer_args);
static void return_to_the_north_pole(reindeer_args_t *reindeer_args);
static void deliver_presents_with_santa(reindeer_args_t *reindeer_args);


_Noreturn void* reindeer(void* args) {
    reindeer_args_t *reindeer_args = args;

    struct drand48_data rand_context;
    srand48_r(reindeer_args->seed, &rand_context);

    pthread_cleanup_push(thread_cleanup_handler, args)

    while (true) {
        rest_on_holiday(rand_context, reindeer_args);
        return_to_the_north_pole(reindeer_args);
        deliver_presents_with_santa(reindeer_args);
    }

    // Free thread memory
    pthread_cleanup_pop(true);
}


static void rest_on_holiday(struct drand48_data rand_context, reindeer_args_t *reindeer_args) {
    // Reindeer is on vacation
    randsleep(&rand_context, reindeer_args->min_vacation_time, reindeer_args->max_vacation_time);
}

static void return_to_the_north_pole(reindeer_args_t *reindeer_args) {
    // The reindeer returns to the North Pole
    pthread_mutex_lock(reindeer_args->reindeer_delivery_mutex);

    // The reindeer returned so the number of reindeer waiting for
    // the Santa Clause should be increased
    (*(reindeer_args->reindeer_waiting_count))++;
    printcf(REINDEER_MSG_COLOR, "Renifer: czeka %d reniferów na Mikołaja, %d\n",
            *(reindeer_args->reindeer_waiting_count),
            reindeer_args->id
    );

    // If all reindeer are back, the current reindeer, which returned last,
    // will wake up Santa Claus
    if (*(reindeer_args->reindeer_waiting_count) == reindeer_args->total_reindeer_count) {
        printcf(REINDEER_MSG_COLOR, "Renifer: wybudzam Mikołaja, %d\n", reindeer_args->id);
        // Let everyone know that Santa Claus was woken up
        pthread_mutex_lock(reindeer_args->santa_sleep_mutex);
        pthread_cond_broadcast(reindeer_args->santa_wake_up_condition);
        pthread_mutex_unlock(reindeer_args->santa_sleep_mutex);
    }

    pthread_mutex_unlock(reindeer_args->reindeer_delivery_mutex);
}

static void deliver_presents_with_santa(reindeer_args_t *reindeer_args) {
    pthread_mutex_lock(reindeer_args->reindeer_wait_mutex);
    // All reindeer deliver presents with Santa Claus
    // (waiting reindeer are delivering presents)
    pthread_cond_wait(reindeer_args->presents_delivered_condition, reindeer_args->reindeer_wait_mutex);
    pthread_mutex_unlock(reindeer_args->reindeer_wait_mutex);
}
