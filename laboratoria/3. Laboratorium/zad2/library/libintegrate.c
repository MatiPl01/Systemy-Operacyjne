#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "libintegrate.h"

#define TEMP_FILES_DIR_PATH "./temp"
#define PART_FILE_PATH_TEMPLATE TEMP_FILES_DIR_PATH "/w%d.txt"


typedef long long ll;
typedef long double ld;

/*
 * Library private functions
 */
static char* create_part_file_path(unsigned file_id);
static ld get_total_area(unsigned no_processes);
static bool write_part_result(ld part_area, unsigned file_id);
static bool remove_file(char* path);
static bool check_children_status(unsigned *pids, unsigned no_processes);


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

    // Create a directory for temporary files
    if (mkdir(TEMP_FILES_DIR_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        perror("Error: Cannot create a temporary files directory.\n");
        return -1;
    }

    // Create an array for children processes pids
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
    if (!check_children_status(pids, no_processes)) return -1;

    // Calculate a total area
    ld total = get_total_area(no_processes);

    // Remove a temporary files directory
    if (rmdir(TEMP_FILES_DIR_PATH) == -1) {
        perror("Error: Cannot remove a temporary files directory.\n");
        return -1;
    }

    return total;
}

/*
 * Library private functions
 */
static char* create_part_file_path(unsigned file_id) {
    unsigned buffer_size = 32;
    char buffer[buffer_size];
    snprintf(buffer, buffer_size, PART_FILE_PATH_TEMPLATE, file_id);

    char* path = (char*) calloc(strlen(buffer) + 1, sizeof(char));

    if (!path) {
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
        // Open a partial results temporary file
        if ((path = create_part_file_path(i)) == NULL) return false;
        file_ptr = fopen(path, "r");

        // Try to read a result of the partial calculation
        if (!file_ptr || !fgets(buffer, buffer_size - 1, file_ptr)) {
            fprintf(stderr, "Error: Cannot read from a file.\n");
            if (file_ptr) fclose(file_ptr);
            free(path);
            return -1;
        }

        fclose(file_ptr);
        area += atof(buffer);

        // Remove a temporary file
        bool was_removed = remove_file(path);
        free(path);
        if (!was_removed) return -1;
    }

    return area;
}

static bool write_part_result(ld part_area, unsigned file_id) {
    char* path = create_part_file_path(file_id);
    if (!path) return false;

    FILE *file_ptr = fopen(path, "w");

    if (!file_ptr) {
        perror("Error: Cannot create a file.\n");
        free(path);
        return false;
    }

    // Write a partial result to the temporary file
    fprintf(file_ptr, "%Lf", part_area);
    fclose(file_ptr);
    free(path);

    return true;
}

static bool check_children_status(unsigned *pids, unsigned no_processes) {
    int status;
    for (unsigned i = 0; i < no_processes; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            fprintf(stderr, "Error: Cannot wait for a child process.\n");
            return false;
        }
        // Check if a process finished with an error status code
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            fprintf(stderr, "Error: There was an error in a child process with PID %d\n", pids[i]);
            return false;
        }
    }
    return true;
}

static bool remove_file(char* path) {
    if (remove(path) != 0) {
        fprintf(stderr, "Error: Cannot remove a file %s.\n", path);
        return false;
    }
    return true;
}
