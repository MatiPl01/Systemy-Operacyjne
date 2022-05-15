#ifndef SYSOPY_COMMONLIB_H
#define SYSOPY_COMMONLIB_H

#include <sys/sem.h>
#include <sys/shm.h>

#define SEMAPHORE_PROJ_ID 'S'
#define MEMORY_PROJ_ID 'M'

#define CHEF_PATH "./chef"
#define DELIVERER_PATH "./deliverer"
#define PERMISSIONS 0666

#define OVEN_CAPACITY 5
#define TABLE_CAPACITY 5

#define MIN_PIZZA_TYPE 0
#define MAX_PIZZA_TYPE 9
#define MIN_PIZZA_PREPARATION_TIME 1
#define MAX_PIZZA_PREPARATION_TIME 2
#define MIN_PIZZA_BAKE_TIME 4
#define MAX_PIZZA_BAKE_TIME 5

#define MIN_TRAVEL_TO_CLIENT_TIME 4
#define MAX_TRAVEL_TO_CLIENT_TIME 5
#define MIN_TRAVEL_TO_PIZZERIA_TIME 4
#define MAX_TRAVEL_TO_PIZZERIA_TIME 5


typedef struct Pizzeria {
    int no_oven_pizzas;
    int oven_slots[OVEN_CAPACITY];
    int no_table_pizzas;
    int table_slots[TABLE_CAPACITY];
} Pizzeria;

typedef enum sem_ids {
    AVAILABLE_PLACES_IN_OVEN = 0,
    AVAILABLE_PLACES_ON_THE_TABLE = 1,
    NO_WAITING_PIZZAS = 2,
    NO_PIZZAS_IN_DELIVERY = 3,
    MEMORY_LOCK = 4
} sem_ids;

union semun {
    int val;
};


struct sembuf put_pizza_in_oven_options = { AVAILABLE_PLACES_IN_OVEN, -1, 0 };
struct sembuf take_pizza_out_of_oven_options = {AVAILABLE_PLACES_IN_OVEN, +1, 0 }; // +1 is not necessary and 1 could be used

struct sembuf put_pizza_on_the_table_options = { AVAILABLE_PLACES_ON_THE_TABLE, -1, 0 };
struct sembuf get_pizza_from_the_table_options = { AVAILABLE_PLACES_ON_THE_TABLE, +1, 0 };

struct sembuf add_pizza_to_waiting_options = { NO_WAITING_PIZZAS, +1, 0 };
struct sembuf remove_pizza_from_waiting_options = { NO_WAITING_PIZZAS, -1, 0 };

struct sembuf start_pizza_delivery_options = { NO_PIZZAS_IN_DELIVERY, +1, 0 };
struct sembuf finish_pizza_delivery_options = { NO_PIZZAS_IN_DELIVERY, -1, 0 };

struct sembuf lock_memory_options = { MEMORY_LOCK, -1, 0 };
struct sembuf unlock_memory_options = { MEMORY_LOCK, +1, 0 };


int randint(int a, int b);
void randsleep(int a, int b);
long long current_timestamp(void);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int find_empty_slot(int arr[], int arr_length);
void fill_array(int arr[], int arr_length, int value);
key_t generate_key(char proj_id);
Pizzeria *load_pizzeria_data(int *sem_id);
void sigint_handler(int sig_no);

#endif //SYSOPY_COMMONLIB_H
