#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#define LENGTH(array, type) (sizeof(array) / sizeof(type))

#define MANY_TO_ONE 0
#define ONE_TO_MANY 1
#define MANY_TO_MANY 2

#define FIFO_PATH "fifo"

#define CONSUMER_READ_COUNT "5"
#define CONSUMER_EXE_PATH "./consumer"
#define CONSUMER_DIR_PATH "./files/consumer"

#define PRODUCER_WRITE_COUNT "5"
#define PRODUCER_EXE_PATH "./producer"
#define PRODUCER_DIR_PATH "./files/consumer"

char* CONSUMER_FILES[] = {
    CONSUMER_DIR_PATH"/many-to-one-res.txt",
    CONSUMER_DIR_PATH"/one-to-many-res.txt",
    CONSUMER_DIR_PATH"/many-to-many-res.txt"
};

char* PRODUCER_FILES[] = {
    PRODUCER_DIR_PATH"/num-file-1.txt",
    PRODUCER_DIR_PATH"/num-file-2.txt",
    PRODUCER_DIR_PATH"/num-file-3.txt",
    PRODUCER_DIR_PATH"/text-file-1.txt",
    PRODUCER_DIR_PATH"/text-file-2.txt",
    PRODUCER_DIR_PATH"/text-file-3.txt",
    PRODUCER_DIR_PATH"/text-file-4.txt",
    PRODUCER_DIR_PATH"/text-file-1.txt"
};


int many_to_one(int no_producers);
int one_to_many(int no_consumers);
int many_to_many(int no_producers, int no_consumers);
int get_mode(int no_consumers, int no_producers);
char* get_input_string(int *i, int argc, char* argv[], char* msg);
int get_input_num(int *i, int argc, char* argv[], char* msg);
void print_centered(char* text, int width, char fill_char);
pid_t exec_in_child(char* args[]);
int wait_for_processes(pid_t *pids, unsigned no_processes);


int main(int argc, char* argv[]) {
    if (argc > 3) {
        fprintf(stderr, "Too many arguments. Expected %d or %d, got %d.\n", 2, 3, argc - 1);
        return 1;
    }

    int i = 1, no_consumers, no_producers;
    if ((no_consumers = get_input_num(&i, argc, argv, "Please provide a number of consumers")) <= 0) {
        fprintf(stderr, "Number of consumers must be a positive integer.\n");
        return 1;
    }
    no_producers = get_input_num(&i, argc, argv, "Please provide a number of producers");
    int max_no_producers = LENGTH(PRODUCER_FILES, char*);
    if (no_producers <= 0 || no_consumers > max_no_producers) {
        fprintf(stderr, "Number of producers must be a positive integer not greater than %d.\n", max_no_producers);
        return 1;
    }

    if (mkfifo(FIFO_PATH, 0777) == -1) {
        if (errno != EEXIST) {
            perror("Unable to create a fifo.\n");
            return 2;
        }
    }

    switch (get_mode(no_consumers, no_producers)) {
        case MANY_TO_ONE: return many_to_one(no_producers) == -1 ? 3 : 0;
        case ONE_TO_MANY: return one_to_many(no_consumers) == -1 ? 3 : 0;
        case MANY_TO_MANY: return many_to_many(no_producers, no_consumers) == -1 ? 3 : 0;
        default: return 3;
    }
}


char* get_input_string(int *i, int argc, char* argv[], char* msg) {
    // If an argument was provided, return this argument
    if (*i < argc) return argv[(*i)++];

    // Otherwise, get an input from a user
    printf("%s\n>>> ", msg);
    char* line;
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

int get_input_num(int *i, int argc, char* argv[], char* msg) {
    char* str = get_input_string(i, argc, argv, msg);
    return (int) strtol(str, NULL, 10);
}

int get_mode(int no_consumers, int no_producers) {
    if (no_producers > 1 && no_consumers > 1) return MANY_TO_MANY;
    if (no_consumers == 1) return MANY_TO_ONE;
    if (no_producers == 1) return ONE_TO_MANY;
    fprintf(stderr, "Unable to find a suitable mode. Wrong number of consumers or producers.\n");
    return -1;
}

void print_centered(char* text, int width, char fill_char) {
    size_t length = strlen(text);

    if (length >= width) {
        puts(text);
        return;
    }

    size_t l_just = (width - length) / 2;
    for (int i = 0; i < l_just; i++) printf("%c", fill_char);
    printf("%s", text);
    for (int i = 0; i < width - length - l_just; i++) printf("%c", fill_char);
    printf("\n");
}

int many_to_one(int no_producers) {
    print_centered("Many producers to one customer", 30, '=');

    // Create a consumer process
    char *consumer_args[] = { CONSUMER_EXE_PATH, FIFO_PATH, CONSUMER_FILES[0], CONSUMER_READ_COUNT, NULL };
    if (exec_in_child(consumer_args) == -1) return -1;
    pid_t pids[no_producers];

    // Create producer processes
    for (int i = 0; i < no_producers; i++) {
        char buff[6];
        sprintf(buff, "%d", i + 1);
        char* producer_args[] = { PRODUCER_EXE_PATH, FIFO_PATH, buff, PRODUCER_FILES[i], PRODUCER_WRITE_COUNT, NULL };
        if ((pids[i] = exec_in_child(producer_args)) == -1) return -1;
    }

    // Wait for producer processes to finish
    return wait_for_processes(pids, no_producers);
}

int one_to_many(int no_consumers) {
    print_centered("One producer to many consumers", 30, '=');

    // Create consumer processes
    char *consumer_args[] = { CONSUMER_EXE_PATH, FIFO_PATH, CONSUMER_FILES[1], CONSUMER_READ_COUNT, NULL };
    pid_t pids[no_consumers];
    for (int i = 0; i < no_consumers; i++) {
        if ((pids[i] = exec_in_child(consumer_args)) == -1) return -1;
    }

    // Create a producer process
    char* producer_args[] = { PRODUCER_EXE_PATH, FIFO_PATH, "1", PRODUCER_FILES[0], PRODUCER_WRITE_COUNT, NULL };
    if (exec_in_child(producer_args) == -1) return -1;

    // Wait for customer processes to finish
    return wait_for_processes(pids, no_consumers);
}

int many_to_many(int no_producers, int no_consumers) {
    print_centered("Many producers to many consumers", 30, '=');

    // Create consumer processes
    char *consumer_args[] = { CONSUMER_EXE_PATH, FIFO_PATH, CONSUMER_FILES[1], CONSUMER_READ_COUNT, NULL };
    pid_t consumer_pids[no_consumers];
    for (int i = 0; i < no_consumers; i++) {
        if ((consumer_pids[i] = exec_in_child(consumer_args)) == -1) return -1;
    }

    // Create producer processes
    pid_t producer_pids[no_consumers];
    for (int i = 0; i < no_producers; i++) {
        char buff[6];
        sprintf(buff, "%d", i + 1);
        char* producer_args[] = { PRODUCER_EXE_PATH, FIFO_PATH, buff, PRODUCER_FILES[i], PRODUCER_WRITE_COUNT, NULL };
        if ((producer_pids[i] = exec_in_child(producer_args)) == -1) return -1;
    }

    // Wait for all processes to finish
    int consumers_status = wait_for_processes(consumer_pids, no_consumers);
    int producers_status = wait_for_processes(consumer_pids, no_producers);
    return (consumers_status == -1 || producers_status == -1) ? -1 : 0;
}

pid_t exec_in_child(char* args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Unable to create a child process.\n");
        return -1;
    }

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("Issues while executing a command.\n");
            exit(1);
        }
        exit(0);
    }

    return pid;
}

int wait_for_processes(pid_t *pids, unsigned no_processes) {
    int status;
    for (unsigned i = 0; i < no_processes; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            fprintf(stderr, "Error: Cannot wait for a child process.\n");
            return -1;
        }
        // Check if a process finished with an error status code
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            fprintf(stderr, "Error: There was an error in a child process with PID %d\n", pids[i]);
            return -1;
        }
    }
    return 0;
}
