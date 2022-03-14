#ifndef LIBSYSOPS_H
#define LIBSYSOPS_H

#define TEMP_FILE_TEMPLATE "/tmp/sysops-tempXXXXXX"

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
char* get_files_stats(char** paths, int no_paths);

bool does_file_exist(char* path);

#endif // LIBSYSOPS_H
