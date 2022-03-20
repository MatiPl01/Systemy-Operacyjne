#include <stdio.h>
#include <stdbool.h>
#include <string.h>
//
//#define COPY_MODE 1
//
//#define LIB_COPY_LIB "../libraries/libcopylib.h"
//#define LIB_COPY_SYS "../libraries/libcopysys.h"
//
//#ifdef COPY_MODE
//    #if COPY_MODE == 1
//        #include LIB_COPY_SYS
//    #else
//        #include LIB_COPY_LIB
//    #endif
//#else
//    #include LIB_COPY_LIB
//#endif
#include "../libraries/libcopysys.h"

char* get_input_line(char* mess);
char* get_file_path(int *i, int argc, char* argv[], char* mess);


int main(int argc, char* argv[]) {
    // Get files paths
    int i = 1;
    char* source_path = get_file_path(&i, argc, argv, "Please provide a source file path");
    char* target_path = get_file_path(&i, argc, argv, "Please provide a target file path");

    // Print copy operation details
    printf("Started copying a file\n");
    printf("  Source: %s\n", source_path);
    printf("  Target: %s\n", target_path);

    // Copy a file
    bool is_successful = copy_file_sys(source_path, target_path);

    // Check if the copy operation was successful
    if (is_successful) {
        printf("Success: Finished copying.\n");
    } else {
        perror("Error: Failed to copy a file.\n");
        return 1;
    }

    return 0;
}


char* get_file_path(int *i, int argc, char* argv[], char* mess) {
    if (*i < argc) {
        char* path = argv[*i];
        ++(*i);
        return path;
    }
    return get_input_line(mess);
}

char* get_input_line(char* mess) {
    printf("%s\n>>>", mess);
    char* line;
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}
