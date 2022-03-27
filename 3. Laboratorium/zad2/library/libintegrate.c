#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "libintegrate.h"

#define PART_FILE_PATH_TEMPLATE "./temp/w%d.txt"


typedef long long ll;
typedef long double ld;

/*
 * Library private functions
 */
static bool write_part_result(ld part_area, unsigned file_id);
static char* create_part_file_path(unsigned file_id);
static ld get_total_area(unsigned no_processes);
bool remove_file(char* path);


ld integrate(ld (*f)(ld), ld a, ld b, ld step) {
    ll i = 0;
    ld x;
    ld area = 0;

    // Calculate areas of subsequent rectangles which have
    // the length equal to step
    while (a + (i + 1) * step < b) {
        x = a + (i++ + .5) * step;
        area += f(x) * step;
    }

    // Calc the remaining part
    ld new_a = a + i * step;
    ld new_step = b - new_a;
    x = new_a + .5 * new_step;
    area += f(x) * new_step;

    return area;
}

ld integrate_async(ld (*f)(ld), ld a, ld b, ld step, unsigned no_processes) {
    if (no_processes < 1) {
        fprintf(stderr, "Error: A number of processes should be greater than 0.\n");
        return -1;
    }

    ld part_step = (b - a) / no_processes, part_a, part_b;
    unsigned pids[no_processes];

    // Divide the whole [a, b] range into no_processes number of
    // ranges of equal length and calculate an integral of the f
    // function in separate threads
    for (unsigned i = 0; i < no_processes; i++) {
        part_a = a + i * part_step;
        part_b = a + (i + 1) * part_step;

        // Perform calculation in the child process
        unsigned pid = fork();
        if (pid == 0) {
            ld part_area = integrate(f, part_a, part_b, step);

            if (!write_part_result(part_area, i + 1)) exit(1);
            exit(0);

        } else {
            pids[i] = pid;
        }
    }

    // Check if all children processes exited successfully
    int status;
    for (unsigned i = 0; i < no_processes; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            fprintf(stderr, "Error: Cannot wait for a child process.\n");
            return -1;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            fprintf(stderr, "Error: There was an error in a child process with PID %d\n", pids[i]);
            return -1;
        }
    }

    return get_total_area(no_processes);
}

/*
 * Library private functions
 */
static bool write_part_result(ld part_area, unsigned file_id) {
    char* path = create_part_file_path(file_id);
    if (path == NULL) return false;

    FILE *file_ptr = fopen(path, "w");

    if (file_ptr == NULL) {
        perror("Error: Cannot create a file.\n");
        return false;
    }

    fprintf(file_ptr, "%Lf", part_area);
    fclose(file_ptr);

    return true;
}

static char* create_part_file_path(unsigned file_id) {
    unsigned buffer_size = 32;
    char buffer[buffer_size];
    snprintf(buffer, buffer_size, PART_FILE_PATH_TEMPLATE, file_id);

    char* path = (char*) calloc(strlen(buffer) + 1, sizeof(char));

    if (path == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }

    strcpy(path, buffer);

    return path;
}

static ld get_total_area(unsigned no_processes) {
    unsigned buffer_size = 32;
    char buffer[buffer_size];
    FILE *file_ptr;

    ld area = 0;

    char* path;
    for (unsigned i = 1; i <= no_processes; i++) {
        if ((path = create_part_file_path(i)) == NULL) return false;

        file_ptr = fopen(path, "r");

        if (file_ptr == NULL) {
            perror("Error: Cannot open a file.\n");
            return -1;
        }
        if (fgets(buffer, buffer_size - 1, file_ptr) == NULL) {
            fprintf(stderr, "Error: Cannot read from a file.\n");
            return -1;
        }

        fclose(file_ptr);

        area += atof(buffer);

        if (!remove_file(path)) return -1;
    }

    return area;
}

bool remove_file(char* path) {
    if (remove(path) != 0) {
        fprintf(stderr, "Error: Cannot remove a file %s.\n", path);
        return false;
    }
    return true;
}
