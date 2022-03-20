#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "libcountlib.h"

/*
 * Better implementation
 */
//CountingResult* count_char_in_file(char c, char* path) {
//    FILE *f_ptr = fopen(path, "r");
//
//    if (f_ptr == NULL) {
//        perror("Error: Cannot open the file.\n");
//        return NULL;
//    }
//
//    CountingResult *cr = (CountingResult*) malloc(sizeof(CountingResult));
//    cr->no_rows = 0;
//    cr->no_all = 0;
//
//    char curr_c;
//    int no_in_line = 0;
//    do {
//        curr_c = fgetc(f_ptr);
//        if (curr_c == c) no_in_line++;
//        else if (curr_c == '\n' || curr_c == EOF) {
//            cr->no_all += no_in_line;
//            if (no_in_line > 0) cr->no_rows++;
//            no_in_line = 0;
//        }
//    } while (curr_c != EOF);
//
//    fclose(f_ptr);
//
//    return cr;
//}

/*
 * Worse implementation
 */
static int get_line_length(FILE* f_ptr);
static char* read_line(FILE* f_ptr);
static bool handle_char_counting(char c, FILE* f_ptr, CountingResult *cr);


CountingResult* count_char_in_file(char c, char* path) {
    FILE *f_ptr = fopen(path, "r");

    if (f_ptr == NULL) {
        perror("Error: Cannot open the file.\n");
        return NULL;
    }

    // Perform char counting
    CountingResult *cr = (CountingResult*) malloc(sizeof(CountingResult));
    bool is_successful = handle_char_counting(c, f_ptr, cr);

    // CHeck if counting operation was successful
    if (!is_successful) {
        perror("Error: Cannot copy a file.\n");
        return NULL;
    }

    fclose(f_ptr);

    return cr;
}

int count_char_in_line(char c, char* line) {
    int count = 0;
    for (int i = 0; i < (int) strlen(line); i++) {
        if (c == line[i]) count++;
    }
    return count;
}

static int get_line_length(FILE* f_ptr) {
    int offset = 0;
    char c;

    do {
        c = getc(f_ptr);
        offset++;

        // If reached the end of a file
        if (c == EOF) {
            fseek(f_ptr, -(--offset), SEEK_CUR);
            return offset;
        }
    } while (c != '\n');

    // Move the cursor back to its previous position
    fseek(f_ptr, -offset, SEEK_CUR);

    return offset - 1;
}

static char* read_line(FILE* f_ptr) {
    int length = get_line_length(f_ptr);
    // Check if there is an error
    if (length < 0) return NULL;
    char* line = (char*) calloc(length + 1, sizeof(char));
    // Check if allocation was successful
    if (line == NULL) {
        perror("Error: Cannot allocate memory for a file line.\n");
        return NULL;
    }
    // Read a line from the file
    int read_length = (int) fread(line, sizeof(char), length, f_ptr);
    if (read_length < length) {
        perror("Error: Cannot read a line from a file.\n");
        free(line);
        return NULL;
    }
    // Skip the \n character
    fseek(f_ptr, 1, SEEK_CUR);
    return line;
}

static bool handle_char_counting(char c, FILE* f_ptr, CountingResult *cr) {
    char* line;
    int line_length, no_curr, no_all, no_rows;
    no_all = no_rows = 0;

    // Loop till the end of a file is reached
    while ((line_length = get_line_length(f_ptr)) > 0) {
        line = read_line(f_ptr);
        if (line == NULL) return false;
        no_curr = count_char_in_line(c, line);
        if (no_curr > 0) {
            no_all += no_curr;
            no_rows++;
        }
        free(line);
    }

    // Save the results
    cr->no_all = no_all;
    cr->no_rows = no_rows;

    // When line_length < 0 there is an error
    return line_length >= 0;
}
