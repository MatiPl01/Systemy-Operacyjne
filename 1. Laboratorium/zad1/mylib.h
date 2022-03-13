#ifndef MYLIB_H
#define MYLIB_H

#define TEMP_FILE_TEMPLATE "/tmp/my-lib-tempXXXXXX"

/*
 * PointersArray
 */
typedef struct {
    int length;
    char **array;
} PointersArray;

PointersArray* create_pointers_array(int length);

void free_pointers_array(PointersArray *pa);

int find_empty_index(PointersArray *pa);

bool create_block_at_index(PointersArray *pa, char* block, int idx);

bool remove_block_at_index(PointersArray *pa, int idx);

int save_string_block(PointersArray *pa, char* block);

/*
 * Files
 */
char* read_file(char* path);

int get_file_length(FILE *fs);

char* get_files_statistics(char** paths, int no_paths);

#endif // MYLIB_H
