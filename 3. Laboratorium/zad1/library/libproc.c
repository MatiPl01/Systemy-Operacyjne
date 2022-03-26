#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libproc.h"

void create_processes(int n) {
    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            printf("I'm a process with a PID = %d\n", getpid());
            exit(0);
        }
    }
}
