#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../library/libfilessearch.h"


char* get_input_string(int *i, int argc, char* argv[], char* msg);
int get_input_num(int *i, int argc, char* argv[], char* msg);


int main(int argc, char* argv[]) {
    if (argc > 4) {
        fprintf(stderr, "Error: Too many arguments.\n");
        return 1;
    }

    // Get input arguments
    int i = 1;
    char* start_path   = get_input_string(&i, argc, argv, "Please provide a path of the starting directory");
    char* searched_str = get_input_string(&i, argc, argv, "Please provide a string that will be searched");
    int   max_depth    = get_input_num(&i, argc, argv, "Please provide a max search depth");

    // Search for the specified string and check if searching
    // was successfully finished
    if (!search_files(start_path, searched_str, max_depth)) {
        fprintf(stderr, "Error: Something went wrong while searching files.\n");
        return 1;
    }

    return 0;
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
    return atoi(str);
}
