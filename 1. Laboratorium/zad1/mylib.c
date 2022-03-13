#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "mylib.h"

/*
 * PointersArray
 */
PointersArray* create_pointers_array(int length) {
    if (length <= 0) {
        fprintf(stderr, "Error: Cannot create a pointers array of length %d.", length);
        return NULL;
    }

    // Create an array of pointers to the results blocks
    PointersArray *pa = (PointersArray*) malloc(sizeof(PointersArray));
    char **array = (char**) calloc(length, sizeof(char*));
    pa->length = length;
    pa->array = array;

    return pa;
}

void free_pointers_array(PointersArray *pa) {
    if (pa == NULL) {
        fprintf(stderr, "Error: Cannot free a pointers array. Pointers array is NULL.");
        return;
    }

    // Free all array elements
    char *curr = pa->array[0];
    while (curr != NULL) free(curr++);

    // Free the array pointer
    free(pa->array);

    // Free PointersArray struct
    free(pa);
}

int find_empty_index(PointersArray *pa) {
    if (pa == NULL) {
        fprintf(stderr, "Error: Cannot find an empty index in a pointers array. Pointers array is NULL.");
        return -1;
    }

    // Look for an empty index
    for (int i = 0; i < pa->length; i++) {
        if (pa->array[i] == NULL) return i;
    }
    return -1;
}

bool create_block_at_index(PointersArray *pa, char* block, int idx) {
    // Return false if function input parameters are incorrect
    if (pa == NULL || block == NULL || idx < 0 || idx >= pa->length) {
        fprintf(stderr, "Error: Cannot create a memory block. Wrong input parameters.");
        return false;
    }

    // Save the block at the specified index
    pa->array[idx] = block;
    return true;
}

bool remove_block_at_index(PointersArray *pa, int idx) {
    // Return false if function input parameters are incorrect
    if (pa == NULL || idx < 0 || idx >= pa->length || pa->array[idx] == NULL) {
        fprintf(stderr, "Error: Cannot remove a memory block. Wrong input parameters.");
        return false;
    }

    // Remove a block (chars array) from the specified index
    free(pa->array[idx]);
    pa->array[idx] = NULL;

    return true;
}

int save_string_block(PointersArray *pa, char* block) {
    if (pa == NULL || block == NULL) {
        fprintf(stderr, "Error: Cannot load a file to the pointers array. Wrong input parameters.");
        return -1;
    }

    // Find an index where a new block can be stored
    int idx = find_empty_index(pa);
    // Return if there is no empty space remaining in the PointersArray
    if (idx < 0) {
        fprintf(stderr, "Error: Cannot load a file to the pointers array. No enough empty space.");
        return -1;
    }

    // Save the file content block
    create_block_at_index(pa, block, idx);

    return idx;
}

/*
 * Files
 */
char* read_file(char* path) {
    if (path == NULL) {
        fprintf(stderr, "Error: Cannot read a file. File path is NULL.");
        return NULL;
    }

    // Try to open a file located at the specified path
    FILE *fs = fopen(path, "r");
    // Return null if a file wasn't found at the specified path
    if (fs == NULL) return NULL;

    int length = get_file_length(fs);
    char *buffer = (char*) calloc(length, sizeof(char));
    // Load the content of a file into the buffer string
    fread(buffer, sizeof(char), length, fs);
    fclose(fs);

    return buffer;
}

int get_file_length(FILE *fs) {
    if (fs == NULL) {
        fprintf(stderr, "Error: Cannot get a length of a file. File stream is NULL.");
        return -1;
    }

    // Store the initial cursor position
    int pos = ftell(fs);
    // Move the cursor to the end of a file
    fseek(fs, 0, SEEK_END);
    // Get the current cursor position (it will be the same as the length of a file)
    int length = ftell(fs);
    // Move the cursor back to its previous position
    fseek(fs, pos, SEEK_SET);

    return length;
}

char* get_files_statistics(char** paths, int no_paths) {
    // Create a temporary file
    char path_buffer[32] = TEMP_FILE_TEMPLATE;
    int path_length = strlen(TEMP_FILE_TEMPLATE);
    int file_id = mkstemp(path_buffer);
    char* temp_path = (char*) calloc(path_length, sizeof(char));
    strcpy(temp_path, path_buffer);

    // Loop over an array of files paths and save each file statistics
    // to the temporary file
    for (int i = 0; i < no_paths; i++) {
        char command[] = "wc ";
        strcat(command, paths[i]);
        strcat(command, " >> ");
        strcat(command, temp_path);

        system(command);
    }
    unlink(path_buffer);
    free(temp_path);

    // Move the cursor to the end of a file to get a length of
    // a temporary file
    int length = lseek(file_id, 0, SEEK_END);
    // Move the cursor to the beginning of a file
    lseek(file_id, 0, SEEK_SET);

    // Create the result string
    char* result = (char*) calloc(length, sizeof(char));
    int read_file_length = read(file_id, result, length);
    close(file_id);

    // Something went wrong if a length of the temporary file content,
    // that was read, is lower than the expected length of the file
    if (read_file_length < length) {
        free(result);
        return NULL;
    }

    return result;
}
