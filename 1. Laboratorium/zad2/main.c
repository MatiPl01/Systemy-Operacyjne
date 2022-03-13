#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef DYNAMIC_MODE
    #include <dlfcn.h>
#else
    #include "../zad1/mylib.h"
#endif

#ifdef MEASURE_TIME
    #include <unistd.h>
    #include <stdint.h>
    #include <sys/times.h>

    // TODO - declare time testing function
#endif


#define CREATE_TABLE_CMD "create_table"
#define WC_FILES_CMD "wc_files"
#define REMOVE_BLOCK_CMD "remove_block"


void handle_create_table(int *i, int argc, char** argv);
int get_table_size_arg(int *i, int argc, char** argv);

void handle_wc_files(int *i, int argc, char** argv);
char** get_files_paths_args(int *i, int argc, char** argv, int no_args);

void handle_remove_block(int *i, int argc, char** argv);
int get_removed_block_idx(int *i, int argc, char** argv);

char* get_next_arg(int *i, int argc, char** argv);
int calc_cmd_args(int cmd_idx, int argc, char** argv);
bool is_cmd_arg(char* arg);

void free_array(char** arr, int length);


int main(int argc, char** argv) {
    #ifdef DYNAMIC_MODE
        // TODO - load a library dynamically
    #endif

    if (argc <= 1) {
        fprintf(stderr, "Error: No arguments were specified.");
        return 1;
    }

    char* cmd;
    for (int i = 1; i < argc; i++) {
        cmd = argv[i];

        if (strcmp(cmd, CREATE_TABLE_CMD) == 0) handle_create_table(&i, argc, argv);
        else if (strcmp(cmd, WC_FILES_CMD) == 0) handle_wc_files(&i, argc, argv);
        else if (strcmp(cmd, REMOVE_BLOCK_CMD) == 0) handle_remove_block(&i, argc, argv);
    }

    free_pointers_array();

    return 0;
}


int get_table_size_arg(int *i, int argc, char** argv) {
    // Try to get the size argument
    char* arg = get_next_arg(i, argc, argv);
    if (arg == NULL) {
        fprintf(stderr, "Error: %s expected a size argument.", CREATE_TABLE_CMD);
        exit(1);
    }
    return atoi(arg);
}

void handle_create_table(int *i, int argc, char** argv) {
    // Try to create a pointers array
    int size = get_table_size_arg(i, argc, argv);
    // Check if a pointers array was successfully created
    bool was_created = create_pointers_array(size);
    // Stop a program if a pointers array already exist
    if (!was_created) {
        fprintf(stderr, "Error: Cannot complete %s. Pointers array already exists.", CREATE_TABLE_CMD);
        exit(1);
    }
}

char** get_files_paths_args(int *i, int argc, char** argv, int no_args) {
    char** args = (char**) calloc(no_args, sizeof(char*));

    for (int j = 0; j < no_args; j++) {
        char* arg = get_next_arg(i, argc, argv);
        char* arg_cp = (char*) calloc(strlen(arg), sizeof(char));
        strcpy(arg_cp, arg);
        args[j] = arg_cp;
    }

    return args;
}

void handle_wc_files(int *i, int argc, char** argv) {
    int no_args = calc_cmd_args(*i, argc, argv);
    if (no_args == 0) {
        fprintf(stderr, "Error: %s expected at least 1 file path.", WC_FILES_CMD);
        exit(1);
    }

    char** paths = get_files_paths_args(i, argc, argv, no_args);
    char* stats = get_files_stats(paths, no_args);

    // TODO - save stats to the report file
    int save_idx = save_string_block(stats);
    printf("Stats: \n%s\n", stats);

    free_array(paths, no_args);
    free(stats);

    if (save_idx < 0) {
        fprintf(stderr, "Error: Cannot complete %s. Statistics block cannot be saved.", WC_FILES_CMD);
        free_pointers_array();
        exit(1);
    }
}

int get_removed_block_idx(int *i, int argc, char** argv) {
    // Try to get the index argument
    char* arg = get_next_arg(i, argc, argv);
    if (arg == NULL) {
        fprintf(stderr, "Error: %s expected an index argument.", REMOVE_BLOCK_CMD);
        exit(1);
    }
    return atoi(arg);
}

void handle_remove_block(int *i, int argc, char** argv) {
    int idx = get_removed_block_idx(i, argc, argv);
    bool was_removed = remove_block_at_index(idx);

    if (!was_removed) {
        fprintf(stderr, "Error: Cannot complete %s. Block at index %d cannot be removed.", REMOVE_BLOCK_CMD, idx);
        exit(1);
    }
}

bool is_cmd_arg(char* arg) {
    return strcmp(arg, CREATE_TABLE_CMD) > 0 &&
           strcmp(arg, WC_FILES_CMD) > 0 &&
           strcmp(arg, REMOVE_BLOCK_CMD) > 0;
}

char* get_next_arg(int *i, int argc, char** argv) {
    if (++(*i) >= argc + 1 || !is_cmd_arg(argv[*i])) return NULL;
    return argv[*i];
}

int calc_cmd_args(int cmd_idx, int argc, char** argv) {
    int c = 0;
    for (int j = cmd_idx + 1; j < argc; j++, c++) {
        if (is_cmd_arg(argv[j])) break;
    }
    return c;
}

void free_array(char** arr, int length) {
    for (int i = 0; i < length; i++) free(arr[i]);
    free(arr);
}
