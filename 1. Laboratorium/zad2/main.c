#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


#define LIB_HEADER_PATH "../zad1/libsysops.h"
#define LIB_SHARED_PATH "../zad1/libsysops.so"

#define CREATE_TABLE_CMD "create_table"
#define WC_FILES_CMD "wc_files"
#define REMOVE_BLOCK_CMD "remove_block"


#ifdef DYNAMIC_MODE
    #include <dlfcn.h>

    bool  (*create_pointers_array)(int);
    bool  (*free_pointers_array)(void);
    bool  (*remove_block_at_index)(int);
    int   (*save_string_block)(char*);
    char* (*get_files_stats)(char**, int);

    void load_my_lib() {
        void *lib_handle = dlopen(LIB_SHARED_PATH, RTLD_LAZY);

        if (lib_handle == NULL) {
            perror("dlopen");
            fprintf(stderr, "Error: Cannot load the dynamic library '%s'\n", LIB_SHARED_PATH);
            exit(1);
        }

        create_pointers_array = dlsym(lib_handle, "create_pointers_array");
        free_pointers_array = dlsym(lib_handle, "free_pointers_array");
        remove_block_at_index = dlsym(lib_handle, "remove_block_at_index");
        save_string_block = dlsym(lib_handle, "save_string_block");
        get_files_stats = dlsym(lib_handle, "get_files_stats");
    }
#else
    #include LIB_HEADER_PATH
#endif

#ifdef MEASURE_TIME
    #include <sys/times.h>
    #include <unistd.h>

    struct tms tms_start_buffer, tms_end_buffer;
    clock_t clock_t_start, clock_t_end;

    void start_timer() {
        clock_t_start = times(&tms_start_buffer);
    }

    void stop_timer() {
        clock_t_end = times(&tms_end_buffer);
    }

    void print_times_headers() {
        printf("\n               %-10s %-10s %-10s\n", "Real", "System", "User");
    }

    void print_time(double time) {
        char s[20];
        sprintf(s, "%.6f", time);
        printf("%-10s ", s);
    }

    double calc_time(clock_t end, clock_t start) {
        return (end - start) / sysconf(_SC_CLK_TCK);
    }

    void print_times(char* cmd) {
        printf("%-15s", cmd);
        print_time(calc_time(clock_t_start, clock_t_end));
        print_time(calc_time(tms_start_buffer.tms_stime, tms_end_buffer.tms_stime));
        print_time(calc_time(tms_start_buffer.tms_cutime, tms_end_buffer.tms_cutime));
        printf("\n");
    }
#endif

void handle_create_table(int *i, int argc, char** argv);
int get_table_size_arg(int *i, int argc, char** argv);

void handle_wc_files(int *i, int argc, char** argv);
char** get_files_paths_args(int *i, int argc, char** argv, int no_args);

void handle_remove_block(int *i, int argc, char** argv);
int get_removed_block_idx(int *i, int argc, char** argv);

void exec_cmd(int *i, int argc, char** argv);
char* get_next_arg(int *i, int argc, char** argv);
int calc_cmd_args(int cmd_idx, int argc, char** argv);
int parse_int(char* str);
bool is_cmd_arg(char* arg);
bool is_number(const char* arg);

void free_array(char** arr, int length);


int main(int argc, char** argv) {
    if (argc <= 1) {
        fprintf(stderr, "Error: No arguments were specified.\n");
        return 1;
    }

    #ifdef DYNAMIC_MODE
        load_my_lib();
    #endif

    #ifdef MEASURE_TIME
        print_times_headers();
    #endif

    for (int i = 1; i < argc; i++) exec_cmd(&i, argc, argv);

    free_pointers_array();

    return 0;
}


int get_table_size_arg(int *i, int argc, char** argv) {
    // Try to get the size argument
    char* arg = get_next_arg(i, argc, argv);
    if (arg == NULL || !is_number(arg)) {
        fprintf(stderr, "Error: %s expected a size argument.\n", CREATE_TABLE_CMD);
        exit(1);
    }
    return parse_int(arg);
}

void handle_create_table(int *i, int argc, char** argv) {
    // Try to create a pointers array
    int size = get_table_size_arg(i, argc, argv);
    // Check if a pointers array was successfully created
    bool was_created = create_pointers_array(size);
    // Stop a program if a pointers array already exist
    if (!was_created) {
        fprintf(stderr, "Error: Cannot complete %s.\n", CREATE_TABLE_CMD);
        exit(1);
    }
}

char** get_files_paths_args(int *i, int argc, char** argv, int no_args) {
    char** args = (char**) calloc(no_args, sizeof(char*));

    for (int j = 0; j < no_args; j++) {
        char* arg = get_next_arg(i, argc, argv);
        char* arg_cp = (char*) calloc(strlen(arg) + 1, sizeof(char));
        strcpy(arg_cp, arg);
        args[j] = arg_cp;
    }

    return args;
}

void handle_wc_files(int *i, int argc, char** argv) {
    int no_args = calc_cmd_args(*i, argc, argv);
    if (no_args == 0) {
        fprintf(stderr, "Error: %s expected at least 1 file path.\n", WC_FILES_CMD);
        exit(1);
    }

    char** paths = get_files_paths_args(i, argc, argv, no_args);
    char* stats = get_files_stats(paths, no_args);
    free_array(paths, no_args);

    if (save_string_block(stats) < 0) {
        fprintf(stderr, "Error: Cannot complete %s. Statistics block cannot be saved.\n", WC_FILES_CMD);
        free_pointers_array();
        free(stats);
        exit(1);
    }

    free(stats);
}

int get_removed_block_idx(int *i, int argc, char** argv) {
    // Try to get the index argument
    char* arg = get_next_arg(i, argc, argv);
    if (arg == NULL) {
        fprintf(stderr, "Error: %s expected an index argument.\n", REMOVE_BLOCK_CMD);
        exit(1);
    }
    return parse_int(arg);
}

void handle_remove_block(int *i, int argc, char** argv) {
    int idx = get_removed_block_idx(i, argc, argv);
    bool was_removed = remove_block_at_index(idx);

    if (!was_removed) {
        fprintf(stderr, "Error: Cannot complete %s. Block at index %d cannot be removed.\n", REMOVE_BLOCK_CMD, idx);
        exit(1);
    }
}

bool is_cmd_arg(char* arg) {
    return strcmp(arg, CREATE_TABLE_CMD) != 0 &&
           strcmp(arg, WC_FILES_CMD) != 0 &&
           strcmp(arg, REMOVE_BLOCK_CMD) != 0;
}

bool is_number(const char* arg) {
    for (int i = 0; arg[i] != '\0'; i++) {
        if (!isdigit(arg[i])) return false;
    }
    return true;
}

char* get_next_arg(int *i, int argc, char** argv) {
    if (++(*i) >= argc || !is_cmd_arg(argv[*i])) return NULL;
    return argv[*i];
}

int calc_cmd_args(int cmd_idx, int argc, char** argv) {
    int c = 0;
    for (int j = cmd_idx + 1; j < argc; j++, c++) {
        if (!is_cmd_arg(argv[j])) break;
    }
    return c;
}

void free_array(char** arr, int length) {
    for (int i = 0; i < length; i++) free(arr[i]);
    free(arr);
}

int parse_int(char* str) {
    char *end_ptr;
    return (int) strtol(str, &end_ptr, 10);
}

void exec_cmd(int *i, int argc, char** argv) {
    #ifdef MEASURE_TIME
        start_timer();
    #endif

    char* cmd = argv[*i];
    if (strcmp(cmd, CREATE_TABLE_CMD) == 0) {
        handle_create_table(i, argc, argv);
    } else if (strcmp(cmd, WC_FILES_CMD) == 0) {
        handle_wc_files(i, argc, argv);
    } else if (strcmp(cmd, REMOVE_BLOCK_CMD) == 0) {
        handle_remove_block(i, argc, argv);
    } else {
        fprintf(stderr, "Error: Command '%s' is not recognized.\n", cmd);
        free_pointers_array();
        exit(1);
    }

    #ifdef MEASURE_TIME
        stop_timer();
        print_times(cmd);
    #endif
}
