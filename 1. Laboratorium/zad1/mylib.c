#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mylib.h"

/*
 * PointersArray
 */
PointersArray* create_pointers_array(int length) {
    if (length <= 0) return NULL;

    // Create an array of pointers to the results blocks
    PointersArray *pa = (PointersArray*)malloc(sizeof(PointersArray));
    char **array = (char**)calloc(length, sizeof(char*));
    pa->length = length;
    pa->array = array;

    return pa;
}

void free_pointers_array(PointersArray *pa) {
    if (pa == NULL) return;

    // Free all array elements
    char *curr = pa->array[0];
    while (curr != NULL) free(curr++);

    // Free the array pointer
    free(pa->array);

    // Free PointersArray struct
    free(pa);
}

int find_empty_index(PointersArray *pa) {
    if (pa != NULL) {
        // Look for an empty index
        for (int i = 0; i < pa->length; i++) {
            if (pa->array[i] == NULL) return i;
        }
    }
    return -1;
}

bool create_block_at_index(PointersArray *pa, char* block, int idx) {
    // Return false if function input parameters are incorrect
    if (pa == NULL || block == NULL || idx < 0 || idx >= pa->length) return false;

    // Save the block at the specified index
    pa->array[idx] = block;
    return true;
}

bool remove_block_at_index(PointersArray *pa, int idx) {
    // Return false if function input parameters are incorrect
    if (pa == NULL || idx < 0 || idx >= pa->length || pa->array[idx] == NULL) return false;

    // Remove a block (chars array) from the specified index
    free(pa->array[idx]);
    pa->array[idx] = NULL;

    return true;
}

/*
 * Files
 */
char* read_file(char* path) {
    if (path == NULL) return NULL;

    // Try to open a file located at the specified path
    FILE *fs = fopen(path, "r");
    // Return null if a file wasn't found at the specified path
    if (fs == NULL) return NULL;

    int length = get_file_length(fs);
    char *buffer = (char*)calloc(length, sizeof(char));
    // Load the content of a file into the buffer string
    fread(buffer, sizeof(char), length, fs);
    fclose(fs);

    return buffer;
}

int get_file_length(FILE *fs) {
    if (fs == NULL) return -1;

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

int load_file_to_pointers_array(PointersArray *pa, char* path) {
    if (pa == NULL || path == NULL) return -1;

    // Find an index where the file content can be stored
    int idx = find_empty_index(pa);
    // Return if there is no empty space remaining in the PointersArray
    if (idx < 0) return -2;

    // Read the file content
    char* content = read_file(path);
    // Return if the file cannot be read
    if (content == NULL) return -3;

    // Save the file content block
    create_block_at_index(pa, content, idx);

    return idx;
}
