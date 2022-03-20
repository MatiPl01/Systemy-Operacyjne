#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "libcopysys.h"


int get_file_size(int fd);
int get_cursor_pos(int fd);
bool copy_file_helper(int source_fd, int target_fd);
int get_line_length(int fd);
char next_char(int fd);
char* read_line(int fd);
bool is_whitespace(char c);
bool is_line_empty(char* line);
bool write_file(int fd, char* text);


bool copy_file_sys(char* source_path, char* target_path) {
    // Open files
    int source_fd, target_fd;
    source_fd = open(source_path, O_RDONLY);
    target_fd = open(target_path, O_WRONLY);

    // Check if files were opened correctly
    if (source_fd < 0) {
        perror("Error: Cannot open the source file.\n");
        return false;
    }
    if (target_fd < 0) {
        perror("Error: Cannot open the target file.\n");
        return false;
    }

    // Copy non-empty lines to the target file
    bool is_successful = copy_file_helper(source_fd, target_fd);
    close(source_fd);
    close(target_fd);

    // CHeck if copy operation was successful
    if (!is_successful) {
        perror("Error: Cannot copy a file.\n");
        return false;
    }

    return true;
}

int get_file_size(int fd) {
    // Get the current cursor position
    int curr_pos = get_cursor_pos(fd);
    // Move the cursor to the end of a file in order to determine
    // the file size
    int size = (int) lseek(fd, 0, SEEK_END);
    // Move the cursor to the previous position
    lseek(fd, curr_pos, SEEK_SET);
    return size;
}

int get_cursor_pos(int fd) {
    return lseek(fd, 0, SEEK_CUR);
}

bool copy_file_helper(int source_fd, int target_fd) {
    char* line;
    bool is_first_line_written = false;
    int line_length;

    // Loop till the end of a file is reached
    while ((line_length = get_line_length(source_fd)) > 0) {
        line = read_line(source_fd);
        if (line == NULL) return false;
        if (is_line_empty(line)) {
            free(line);
            continue;
        }
        // Write '\n' if the first line had been written before and then
        // write the current line
        if ((is_first_line_written && !write_file(target_fd, "\n")) || !write_file(target_fd, line)) {
            free(line);
            return false;
        } else is_first_line_written = true;
        free(line);
    }

    // When line_length < 0 there is an error
    return line_length >= 0;
}

int get_line_length(int fd) {
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
                perror("Error: Cannot read the next character.");
                return -1;
            }
        }
    } while (c != '\n');

    // Move the cursor back to its previous position
    lseek(fd, -offset, SEEK_CUR);

    return offset - 1;
}

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

char* read_line(int fd) {
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
        perror("Error: Cannot read a line from a file.\n");
        free(line);
        return NULL;
    }
    // Skip the \n character
    lseek(fd, 1, SEEK_CUR);
    return line;
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_line_empty(char* line) {
    for (int i = 0; i < (int) strlen(line); i++) {
        if (!is_whitespace(line[i])) return false;
    }
    return true;
}

bool write_file(int fd, char* text) {
    int length = strlen(text);
    int written_length = write(fd, text, length);

    if (written_length < length) {
        perror("Error: Cannot write a line to the file.\n");
        return false;
    }
    return true;
}
