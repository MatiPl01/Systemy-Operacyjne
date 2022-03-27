#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "libcopylib.h"


static int get_line_length(FILE* f_ptr);
static char* find_next_line(FILE* f_ptr);
static char* read_line(FILE* f_ptr);
static bool write_file(FILE* f_ptr, char* line);
static bool reached_EOF(FILE* f_ptr);
static bool copy_file_helper(FILE *source_ptr, FILE *target_ptr);
static bool is_whitespace(char c);
static bool is_line_empty(char* line);


bool copy_file(char* source_path, char* target_path) {
    // Open files streams
    FILE *source_ptr = fopen(source_path, "r");
    FILE *target_ptr = fopen(target_path, "w");

    // Check if streams were opened
    if (source_ptr == NULL) {
        perror("Error: Cannot open the source file.\n");
        return false;
    }
    if (target_ptr == NULL) {
        perror("Error: Cannot open the target file.\n");
        return false;
    }

    // Copy non-empty lines to the target file
    bool is_successful = copy_file_helper(source_ptr, target_ptr);
    fclose(source_ptr);
    fclose(target_ptr);

    // CHeck if copy operation was successfull
    if (!is_successful) {
        printf("Error: Cannot copy a file.\n");
        return false;
    }

    return true;
}

static bool copy_file_helper(FILE *source_ptr, FILE *target_ptr) {
    char* line = find_next_line(source_ptr);
    // Return true if reached the end of file or false when an error occurred
    if (line == NULL) return reached_EOF(source_ptr);

    bool is_success = true;
    while (true) {
        is_success = write_file(target_ptr, line);
        free(line);
        // Return false if error occurred while writing to a file
        if (!is_success) return false;
        line = find_next_line(source_ptr);
        // Return true if reached the end of file or false when an error occurred
        if (line == NULL) return reached_EOF(source_ptr);
        is_success = write_file(target_ptr, "\n");
        // Return false if error occurred while writing to a file
        if (!is_success) {
            free(line);
            return false;
        }
    }

    return true;
}

static bool write_file(FILE* f_ptr, char* text) {
    int length = strlen(text);
    int written_length = fwrite(text, sizeof(char), length, f_ptr);

    if (written_length < length) {
        printf("Error: Cannot write a line to the file.\n");
        return false;
    }
    return true;
}

static int get_line_length(FILE* f_ptr) {
    // Return -1 if reached the end of a file
    if (reached_EOF(f_ptr)) return -1;

    int c = 0, length = 0;

    // Calculate the total length of a line (\n character inclusive)
    while (c != EOF && c != '\n') {
        c = fgetc(f_ptr);
        length++;
    }

    // Decrease a length of a line if reached the end of a file
    if (c == EOF) length--;
    // Move back the cursor to its previous position
    fseek(f_ptr, -length, SEEK_CUR);

    return length;
}

static bool reached_EOF(FILE* f_ptr) {
    int curr_pos = ftell(f_ptr);
    fseek(f_ptr, 0, SEEK_END);
    int end_pos = ftell(f_ptr);
    fseek(f_ptr, curr_pos, SEEK_SET);
    return curr_pos == end_pos;
}

static char* read_line(FILE* f_ptr) {
    int length = get_line_length(f_ptr);
    // Check if reached the end of a file
    if (length < 0) return NULL;
    // Allocate memory for the file text line
    char* line = (char*) calloc(length + 1, sizeof(char));
    // Check if allocation was successful
    if (line == NULL) {
        perror("Error: Cannot allocate memory for a file line.\n");
        return NULL;
    }
    // Read a line from the file
    int read_length = fread(line, sizeof(char), length, f_ptr);
    if (read_length < length) {
        printf("Error: Cannot read a line from a file.\n");
        free(line);
        return NULL;
    }
    // Remove the \n character
    if (line[length - 1] == '\n') line[length - 2] = '\0';

    return line;
}

static char* find_next_line(FILE* f_ptr) {
    while (true) {
        char* line = read_line(f_ptr);
        // Return true if the there are no more lines in a file (line is NULL)
        // or the current line is not empty (has not only whitespace characters)
        if (line == NULL || !is_line_empty(line)) return line;
        free(line);
    }
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool is_line_empty(char* line) {
    for (int i = 0; i < (int) strlen(line); i++) {
        if (!is_whitespace(line[i])) return false;
    }
    return true;
}
