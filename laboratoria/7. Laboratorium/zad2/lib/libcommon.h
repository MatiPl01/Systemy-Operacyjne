#ifndef SYSOPY_COMMONLIB_H
#define SYSOPY_COMMONLIB_H

#include <sys/sem.h>
#include <sys/shm.h>

#define SEMAPHORE_PROJ_ID 'S'
#define MEMORY_PROJ_ID 'M'
#define NO_SEMAPHORES 5

#define AVAILABLE_PLACES_IN_OVEN "/oven-sem"
#define AVAILABLE_PLACES_ON_THE_TABLE "/table-sem"
#define NO_WAITING_PIZZAS "/waiting-sem"
#define NO_PIZZAS_IN_DELIVERY "/delivery-sem"
#define MEMORY_LOCK "/memory-sem"

#define SHARED_MEMORY_PATH "/shared-mem"

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

union semun {
    int val;
};


int randint(int a, int b);
void randsleep(int a, int b);
long double current_timestamp(void);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int find_empty_slot(int arr[], int arr_length);
void fill_array(int arr[], int arr_length, int value);
Pizzeria *load_pizzeria_data(void);
void sigint_handler(int sig_no);

#endif //SYSOPY_COMMONLIB_H
