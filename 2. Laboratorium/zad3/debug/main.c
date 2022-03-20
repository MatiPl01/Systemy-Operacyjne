#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define LIB_LIST_DIR_STAT "../libraries/liblistdirstat.h"
#define LIB_LIST_DIR_NFTW "../libraries/liblistdirnftw.h"

#ifdef LIB_NFTW
    #include LIB_LIST_DIR_NFTW
#else
    #include LIB_LIST_DIR_STAT
#endif


char* get_dir_path(int argc, char* argv[]);
char* get_input_line(char* mess);


int main(int argc, char* argv[]) {
    char* path = get_dir_path(argc, argv);
    if (path == NULL) return 1;

    list_dir(path);

    free(path);

    return 0;
}

char* get_dir_path(int argc, char* argv[]) {
    char* path;
    if (argc < 2) {
        path = get_input_line("Please specify a path of a directory");
    } else if (argc > 2) {
        fprintf(stderr, "Error: Too many arguments. Expected 1 path, got %d arguments.\n", argc - 1);
        return NULL;
    } else {
        int length = strlen(argv[1]);
        path = (char*) calloc(length + 1, sizeof(char));

        if (path == NULL) {
            perror("Error: Cannot allocate memory.\n");
            return NULL;
        }

        strcpy(path, argv[1]);
    }
    return path;
}

char* get_input_line(char* mess) {
    printf("%s\n>>>", mess);
    char* line = "";
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}
