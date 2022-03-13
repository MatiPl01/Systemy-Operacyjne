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

bool create_pointers_array(int length);

bool free_pointers_array();

int find_empty_index();

bool create_block_at_index(char* block, int idx);

bool remove_block_at_index(int idx);

int save_string_block(char* block);

/*
 * Files
 */
char* read_file(char* path);

int get_file_length(FILE *fs);

char* get_files_stats(char** paths, int no_paths);

#endif // MYLIB_H
