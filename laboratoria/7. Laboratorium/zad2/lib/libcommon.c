#include "libcommon.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>


int randint(int a, int b) {
    return a + rand() % (b - a);
}

void randsleep(int a, int b) {
    sleep(randint(a, b));
}

long double current_timestamp(void) {
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    time_t s = spec.tv_sec;
    int ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

    return s + ms / 1000.;
}

int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int)) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_handler = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        perror(" Nie można ustawić akcji dla sygnału\n");
        return -1;
    }

    return 0;
}

int find_empty_slot(int arr[], int arr_length) {
    for (int i = 0; i < arr_length; i++) {
        if (arr[i] == -1) return i;
    }
    return -1;
}

void fill_array(int arr[], int arr_length, int value) {
    for (int i = 0; i < arr_length; i++) arr[i] = value;
}

Pizzeria *load_pizzeria_data(void) {
    // Open existing shared memory
    int shm_id;
    if ((shm_id = shm_open(SHARED_MEMORY_PATH, O_RDWR, 0)) == -1) {
        perror("Nie można otworzyć segmentu pamięci współdzielonej\n");
        return NULL;
    }

    // Load pizzeria data
    Pizzeria *pizzeria;
    if ((pizzeria = mmap(NULL, sizeof(Pizzeria), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == (void*) -1) {
        perror("Nie można załadować danych z pamięci współdzielonej\n");
        return NULL;
    }

    return pizzeria;
}

void sigint_handler(int sig_no) {
    printf("Otrzymałem sygnał %d. Kończę pracę...\n", sig_no);
    exit(EXIT_SUCCESS);
}
