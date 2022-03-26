#include <stdio.h>
#include <stdlib.h>
#include "./library/libproc.h"


int get_no_processes(int argc, char* argv[]);


int main(int argc, char* argv[]) {
    if (argc > 2) {
        printf("Error: Too many arguments.\n");
        return 1;
    }

    int n = get_no_processes(argc, argv);
    if (n < 1) return 1;

    create_processes(n);

    return 0;
}


int get_no_processes(int argc, char* argv[]) {
    int n;

    if (argc == 2) {
        n = atoi(argv[1]);
    } else {
        printf("Please provide a number of processes\n>>> ");
        scanf("%d", &n);
    }

    if (n < 1) printf("Error: Number of processes should be at least 1.\n");

    return n;
}
