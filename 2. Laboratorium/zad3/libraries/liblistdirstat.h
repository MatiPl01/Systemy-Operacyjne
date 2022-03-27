//
// Created by mateu on 20.03.2022.
//

#ifndef LIBLISTDIRSTAT_H
#define LIBLISTDIRSTAT_H

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

typedef struct EntityInfo {
    char* abs_path;
    char* type;
    nlink_t no_links;
    off_t total_size;
    time_t last_access_time;
    time_t last_modification_time;
} EntityInfo;

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

struct dirent;

/*
 * Data structures
 */
Node* create_ll_node(char* str);
Node* append_to_ll(Node *tail, char* str);
void free_ll(Node *head);

/*
 * Paths
 */
bool is_rel_path(char* path);
char* get_abs_path(char* path);
char* get_entity_path(char* dir_path, struct dirent* entity);

/*
 * Entities information
 */
EntityInfo* get_entity_info(char* dir_path, struct dirent* entity);
char* get_entity_type(struct dirent* entity);
bool print_entity_info(EntityInfo *ei);
void free_entity_info(EntityInfo *ei);
void print_info_headers();
void print_summary();

/*
 * Helpers
 */
char* get_formatted_time(time_t time);

/*
 * Main function
 */
bool list_dir(char* path);

#endif //LIBLISTDIRSTAT_H
