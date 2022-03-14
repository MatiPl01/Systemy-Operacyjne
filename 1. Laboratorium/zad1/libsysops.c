#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "libsysops.h"


PointersArray *pa;

// Library private functions
int calc_cmd_length(char* temp_path, char** paths, int no_paths);
char* create_cmd(char* temp_path, char** paths, int no_paths);

/*
 * PointersArray
 */
bool create_pointers_array(int length) {
    if (pa != NULL) {
        fprintf(stderr, "Error: Pointers array already exist. Remove the existing pointers array first.\n");
        return false;
    }
    if (length <= 0) {
        fprintf(stderr, "Error: Cannot create a pointers array of length %d.\n", length);
        return false;
    }
    // Create an array of pointers to the results blocks
    pa = (PointersArray*) malloc(sizeof(PointersArray));
    char** array = (char**) calloc(length, sizeof(char*));
    pa->length = length;
    pa->array = array;
    return true;
}

bool free_pointers_array() {
    if (pa == NULL) {
        fprintf(stderr, "Error: Cannot free a pointers array. Pointers array does not exist.\n");
        return false;
    }
    // Free the remaining elements
    for (int i = 0; i < pa->length; i++) {
        if (pa->array[i] != NULL) free(pa->array[i]);
    }
    // Free the array pointer
    free(pa->array);
    // Free PointersArray struct
    free(pa);

    return true;
}

int find_empty_index() {
    if (pa == NULL) {
        fprintf(stderr, "Error: Cannot free a pointers array. Pointers array does not exist.\n");
        return -1;
    }
    // Look for an empty index
    for (int i = 0; i < pa->length; i++) {
        if (pa->array[i] == NULL) return i;
    }
    return -1;
}

bool create_block_at_index(char* block, int idx) {
    if (pa == NULL) {
        fprintf(stderr, "Error: Cannot free a pointers array. Pointers array does not exist.\n");
        return false;
    }
    // Return false if function input parameters are incorrect
    if (block == NULL || idx < 0 || idx >= pa->length) {
        fprintf(stderr, "Error: Cannot create a memory block. Wrong input parameters.\n");
        return false;
    }
    // Save the block at the specified index
    pa->array[idx] = (char*) calloc(strlen(block) + 1, sizeof(char));
    strcpy(pa->array[idx], block);

    return true;
}

bool remove_block_at_index(int idx) {
    if (pa == NULL) {
        fprintf(stderr, "Error: Cannot free a pointers array. Pointers array does not exist.\n");
        return false;
    }
    // Return false if function input parameters are incorrect
    if (idx < 0 || idx >= pa->length || pa->array[idx] == NULL) {
        fprintf(stderr, "Error: Cannot remove a memory block. Wrong input parameter.\n");
        return false;
    }
    // Remove a block (chars array) from the specified index
    free(pa->array[idx]);
    pa->array[idx] = NULL;

    return true;
}

int save_string_block(char* block) {
    if (block == NULL) {
        fprintf(stderr, "Error: Cannot load a file to the pointers array. Wrong input parameter.\n");
        return -1;
    }
    // Find an index where a new block can be stored
    int idx = find_empty_index();
    // Return if there is no empty space remaining in the PointersArray
    if (idx < 0) {
        fprintf(stderr, "Error: Cannot load a file to the pointers array. No enough empty space.\n");
        return -1;
    }
    // Save the file content block
    create_block_at_index(block, idx);

    return idx;
}

/*
 * Files
 */
bool does_file_exist(char* path) {
    if (access(path, F_OK) != -1) return true;
    return false;
}

char* get_files_stats(char** paths, int no_paths) {
    // Check if input parameters are correct
    if (paths == NULL || no_paths <= 0) return NULL;
    for (int i = 0; i < no_paths; i++) {
        if (!does_file_exist(paths[i])) {
            fprintf(stderr, "Error: File '%s' does not exist.\n", paths[i]);
            return NULL;
        }
    }
    // Create a temporary file
    char path_buffer[32] = TEMP_FILE_TEMPLATE;
    int path_length = strlen(TEMP_FILE_TEMPLATE);
    int file_id = mkstemp(path_buffer);
    char* temp_path = (char*) calloc(path_length + 1, sizeof(char));
    strcpy(temp_path, path_buffer);
    // Execute a command calculating files statistics
    char* cmd = create_cmd(temp_path, paths, no_paths);
    system(cmd);

    unlink(path_buffer);
    free(temp_path);
    free(cmd);
    // Move the cursor to the end of a file to get a length of
    // a temporary file
    int length = (int) lseek(file_id, 0, SEEK_END);
    // Move the cursor to the beginning of a file
    lseek(file_id, 0, SEEK_SET);
    // Create the result string
    char* result = (char*) calloc(length + 1, sizeof(char));
    int read_file_length = (int) read(file_id, result, length);
    close(file_id);
    // Something went wrong if a length of the temporary file content,
    // that was read, is lower than the expected length of the file
    if (read_file_length < length) {
        free(result);
        return NULL;
    }

    return result;
}

int calc_cmd_length(char* temp_path, char** paths, int no_paths) {
    /* wc <paths> > <temp_path>\0
     * \_/       \_/            |
     *  3         3             1
     */
    int length = strlen(temp_path);
    for (int i = 0; i < no_paths; i++) length += strlen(paths[i]) + 1;
    return length + 6;
}

char* create_cmd(char* temp_path, char** paths, int no_paths) {
    int cmd_length = calc_cmd_length(temp_path, paths, no_paths);
    char* cmd = (char*) calloc(cmd_length, sizeof(char));

    strcat(cmd, "wc ");
    for (int i = 0; i < no_paths; i++) {
        strcat(cmd, paths[i]);
        strcat(cmd, " ");
    }
    strcat(cmd, "> ");
    strcat(cmd, temp_path);
    return cmd;
}
