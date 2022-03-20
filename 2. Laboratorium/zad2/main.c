#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LIB_COUNT_LIB "./libraries/libcountlib.h"
#define LIB_COUNT_SYS "./libraries/libcountsys.h"

#ifdef LIB_SYS
    #include LIB_COUNT_SYS
#else
    #include LIB_COUNT_LIB
#endif


char get_input_char(int argc, char* argv[]);
char* get_input_path(int argc, char* argv[]);
char input_char(char* mess);
char* input_line(char* mess);


int main(int argc, char* argv[]) {
    if (argc > 3) {
        perror("Error: Too many arguments.\n");
        return 1;
    }
    char c = get_input_char(argc, argv);
    char* path = get_input_path(argc, argv);

    CountingResult *cr = count_char_in_file(c, path);

    if (cr == NULL) {
        perror("Error: Cannot perform character counting.\n");
        return 1;
    }

    printf("Number of characters: %d\n", cr->no_all);
    printf("Number of rows:       %d\n", cr->no_rows);

    free(cr);

    return 0;
}


char get_input_char(int argc, char* argv[]) {
    if (argc < 2) return input_char("Please provide a single character to count.");
    if (strlen(argv[1]) > 1) {
        fprintf(stderr, "Error: Expected a single character, got '%s'\n", argv[1]);
        return '\0';
    }
    return argv[1][0];
}

char* get_input_path(int argc, char* argv[]) {
    if (argc < 3) return input_line("Please provide a path to a file.");
    return argv[2];
}

char input_char(char* mess) {
    printf("%s\n>>>", mess);
    char c;
    scanf(" %c", &c);
    getchar();
    return c;
}

char* input_line(char* mess) {
    printf("%s\n>>>", mess);
    char* line;
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}
