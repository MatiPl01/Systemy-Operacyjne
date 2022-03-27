//
// Created by mateu on 20.03.2022.
//

#ifndef LIBLISTDIRNFTW_H
#define LIBLISTDIRNFTW_H

/*
 * Structs
 */
typedef struct Stats {
    int no_files;
    int no_dirs;
    int no_char_devs;
    int no_block_devs;
    int no_fifos;
    int no_slinks;
    int no_socks;
} Stats;

struct FTW;
struct stat;

/*
 * Paths
 */
bool is_rel_path(char* path);
char* get_abs_path(char* path);

/*
 * Entities information
 */
char* get_entity_type(const struct stat *s);
bool print_entity_info(const char* path, const struct stat *sb);
void print_info_headers();
void print_summary();

/*
 * Main function
 */
bool list_dir(char* path);

/*
 * Helpers
 */
char* get_formatted_time(time_t time);

#endif //LIBLISTDIRNFTW_H
