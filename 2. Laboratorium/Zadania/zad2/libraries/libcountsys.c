#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "libcountlib.h"


char next_char(int fd);


char next_char(int fd) {
    char* c = (char*) calloc(1, sizeof(char));
    // Return '\0' character if the next char cannot be read
    if (read(fd, c, 1) < 1) {
        free(c);
        return '\0';
    }
    char res = c[0];
    free(c);
    return res;
}

/*
 * Better implementation
 */
//CountingResult* count_char_in_file(char c, char* path) {
//    int fd = open(path, O_RDONLY);
//
//    if (fd < 0) {
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
//        curr_c = next_char(fd);
//        if (curr_c == c) no_in_line++;
//        else if (curr_c == '\n' || curr_c == '\0') {
//            cr->no_all += no_in_line;
//            if (no_in_line > 0) cr->no_rows++;
//            no_in_line = 0;
//        }
//    } while (curr_c != '\0');
//
//    close(fd);
//
//    return cr;
//}

/*
 * Worse implementation
 */
static int get_line_length(int fd);
static char* read_line(int fd);
static int get_file_size(int fd);
static int get_cursor_pos(int fd);
static bool handle_char_counting(char c, int fd, CountingResult *cr);


CountingResult* count_char_in_file(char c, char* path) {
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        perror("Error: Cannot open the file.\n");
        return NULL;
    }

    // Perform char counting
    CountingResult *cr = (CountingResult*) malloc(sizeof(CountingResult));
    bool is_successful = handle_char_counting(c, fd, cr);

    // CHeck if counting operation was successful
    if (!is_successful) {
        printf("Error: Cannot copy a file.\n");
        return NULL;
    }

    close(fd);

    return cr;
}

int count_char_in_line(char c, char* line) {
    int count = 0;
    for (int i = 0; i < (int) strlen(line); i++) {
        if (c == line[i]) count++;
    }
    return count;
}

static int get_file_size(int fd) {
    // Get the current cursor position
    int curr_pos = get_cursor_pos(fd);
    // Move the cursor to the end of a file in order to determine
    // the file size
    int size = (int) lseek(fd, 0, SEEK_END);
    // Move the cursor to the previous position
    lseek(fd, curr_pos, SEEK_SET);
    return size;
}

static int get_cursor_pos(int fd) {
    return lseek(fd, 0, SEEK_CUR);
}

static int get_line_length(int fd) {
    int offset = 0;
    char c;

    do {
        c = next_char(fd);
        offset++;

        // If the next char cannot be read
        if (c == '\0') {
            // Check if the end of a file has been reached
            if (get_cursor_pos(fd) >= get_file_size(fd)) {
                lseek(fd, -(--offset), SEEK_CUR);
                return offset;
            }
            // Otherwise, there is an error
            else {
                printf("Error: Cannot read the next character.");
                return -1;
            }
        }
    } while (c != '\n');

    // Move the cursor back to its previous position
    lseek(fd, -offset, SEEK_CUR);

    return offset - 1;
}

static char* read_line(int fd) {
    int length = get_line_length(fd);
    // Check if there is an error
    if (length < 0) return NULL;
    char* line = (char*) calloc(length + 1, sizeof(char));
    // Check if allocation was successful
    if (line == NULL) {
        perror("Error: Cannot allocate memory for a file line.\n");
        return NULL;
    }
    // Read a line from the file
    int read_length = (int) read(fd, line, length);
    if (read_length < length) {
        printf("Error: Cannot read a line from a file.\n");
        free(line);
        return NULL;
    }
    // Skip the \n character
    lseek(fd, 1, SEEK_CUR);
    return line;
}

static bool handle_char_counting(char c, int fd, CountingResult *cr) {
    char* line;
    int line_length, no_curr, no_all, no_rows;
    no_all = no_rows = 0;

    // Loop till the end of a file is reached
    while ((line_length = get_line_length(fd)) > 0) {
        line = read_line(fd);
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
