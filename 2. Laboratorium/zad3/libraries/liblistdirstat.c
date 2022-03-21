#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include "liblistdirstat.h"

/*
 * Private functions
 */
static bool list_dir_recur(char* dir_path);
static bool list_saved_dirs(Node *head);
static int calc_trimmed_dir_path_length(char* dir_path, char* entity_name);
static void update_stats(struct dirent* entity);

// Global statistics
Stats *global_stats;


bool list_dir(char* dir_path) {
    // Allocate memory for the final statistics of the listed directory
    global_stats = (Stats*) calloc(1, sizeof(Stats));
    if (global_stats == NULL) {
        perror("Error: Cannot allocate memory\n");
        return false;
    }
    // Call a recursive function which will traverse the current
    // directory subtree and list all entities of its subdirectories
    bool is_successful = list_dir_recur(dir_path);
    // Add the starting directory to the total number of directories
    global_stats->no_dirs++;
    // Check if a listing operation was successful
    if (is_successful) print_summary();
    free(global_stats);

    return is_successful;
}

bool list_dir_recur(char* dir_path) {
    // Try to open the specified directory
    DIR *d_ptr = NULL;
    d_ptr = opendir(dir_path);
    if (d_ptr == NULL) {
        fprintf(stderr, "Error: Cannot open a directory %s\n", dir_path);
        return false;
    }
    // Get absolute directory path
    char* cwd_abs_path = get_abs_path(dir_path);
    if (cwd_abs_path == NULL) {
        perror("Cannot create an absolute path of the current directory.\n");
        return false;
    }

    // Print the current directory path
    printf("\nCURRENT DIRECTORY: \n\t%s\n\n", cwd_abs_path);
    // Print information table headers
    print_info_headers();
    free(cwd_abs_path);

    // Create a linked list to store subdirectories that will be listed later
    Node *head = create_ll_node("");
    if (head == NULL) return false;
    Node *tail = head;

    // List details of all directory entities
    struct dirent* entity;
    while (true) {
        entity = readdir(d_ptr);
        if (entity == NULL) break;
        if (strcmp(entity->d_name, ".") == 0 || strcmp(entity->d_name, "..") == 0) continue;

        // Get information about hte current entity
        EntityInfo *ei = get_entity_info(dir_path, entity);
        if (ei == NULL) {
            perror("Error: Cannot get entity info.\n");
            return false;
        }

        // If the current entity is a directory, store its path to list its
        // entities later
        if (entity->d_type == DT_DIR) tail = append_to_ll(tail, ei->abs_path);

        update_stats(entity);
        print_entity_info(ei);
        free_entity_info(ei);
    }

    // List subdirectories of the current directory
    bool is_successful = list_saved_dirs(head);
    free_ll(head);
    if (!is_successful) return false;

    if (closedir(d_ptr) == -1) {
        fprintf(stderr, "Error: Cannot close a directory %s\n", dir_path);
        return false;
    }

    return true;
}

static bool list_saved_dirs(Node *head) {
    Node* curr = head->next;

    while (curr != NULL) {
        // Return false if there was an error while listing a directory
        if (!list_dir_recur(curr->text)) return false;
        curr = curr->next;
    }
    return true;
}

bool is_rel_path(char* path) {
    return path != NULL && strlen(path) > 0 && path[0] == '.';
}

char* get_abs_path(char* path) {
    char buff[PATH_MAX];
    realpath(path, buff);
    char* abs_path = (char*) calloc(strlen(buff) + 1, sizeof(char));
    if (abs_path == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }
    strcpy(abs_path, buff);
    return abs_path;
}


char* get_entity_path(char* dir_path, struct dirent* entity) {
    char* entity_name = entity->d_name;
    // Calculate directory path length without repeated slashes or dots (if not necessary)
    int dir_path_len = calc_trimmed_dir_path_length(dir_path, entity_name);
    if (dir_path_len <= 0) return NULL;
    int path_length = dir_path_len + strlen(entity_name) + 1;

    // Merge the directory path with the entity path
    char* path = (char*) calloc(path_length + 1, sizeof(char));
    if (path == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }
    strncpy(path, dir_path, dir_path_len);
    strcat(path, "/");
    strcat(path, entity_name);

    return path;
}

int calc_trimmed_dir_path_length(char* dir_path, char* entity_name) {
    if (dir_path != NULL) {
        int length = strlen(dir_path);
        if (dir_path[length - 1] != '/') return length;
        // Remove repeated trailing slashes
        for (int i = length - 2; i >= 0; i--) {
            if (dir_path[i] != '/') return i + 1;
        }
        return 0;
    }
    perror("Error: Cannot trim the directory path.\n");
    return -1;
}

EntityInfo* get_entity_info(char* dir_path, struct dirent* entity) {
    if (entity == NULL) return NULL;

    // Create the absolute path
    char* abs_path = NULL;
    char* entity_path = get_entity_path(dir_path, entity);
    if (entity_path != NULL) abs_path = get_abs_path(entity_path);
    free(entity_path);
    if (abs_path == NULL) {
        printf("ERROR NOW: '%s', ('%s')\n", entity_path, dir_path);
        perror("Error: Cannot create an absolute path.\n");
        return NULL;
    }

    // Get a type of the entity
    char* type = get_entity_type(entity);
    if (type == NULL) {
        perror("Error: Cannot recognize a type of the entity.\n");
        return NULL;
    }

    // Create entity stats object
    struct stat sb;
    if (lstat(abs_path, &sb) == -1) {
        perror("Error: Cannot read entity stats.\n");
        return NULL;
    }

    // Create the entity info object
    EntityInfo *ei = (EntityInfo*) malloc(sizeof(EntityInfo));
    ei->abs_path = abs_path;
    ei->type = type;
    ei->no_links = sb.st_nlink;
    ei->total_size = sb.st_size;
    ei->last_access_time = sb.st_atime;
    ei->last_modification_time = sb.st_mtime;

    return ei;
}

char* get_entity_type(struct dirent* entity) {
    char* type = NULL;
    switch (entity->d_type) {
        case DT_REG: return "file";
        case DT_DIR: return "dir";
        case DT_CHR: return "char dev";
        case DT_BLK: return "block dev";
        case DT_FIFO: return "fifo";
        case DT_LNK: return "slink";
        case DT_SOCK: return "sock";
    }
    return type;
}

bool print_entity_info(EntityInfo *ei) {
    printf("%8ld |", ei->no_links);
    printf(" %9s |", ei->type);
    printf(" %10ldB |", ei->total_size);

    char* lat = get_formatted_time(ei->last_access_time);
    char* lmt = get_formatted_time(ei->last_modification_time);

    bool status = true;
    if (lat != NULL && lmt != NULL) {
        printf(" %s |", lat);
        printf(" %s |", lmt);
        printf(" %s\n", ei->abs_path);
    } else status = false;

    if (lat != NULL) free(lat);
    if (lmt != NULL) free(lmt);

    return status;
}

void free_entity_info(EntityInfo *ei) {
    free(ei->abs_path);
    free(ei);
}

void print_info_headers() {
    printf("%-8s | %-9s | %-11s | %-19s | %-19s | %s\n",
           "No Links",
           "Type",
           "Size",
           "Last access",
           "Last modification",
           "Absolute path");
    for (int i = 0; i < 95; i++) printf("-");
    printf("\n");
}

void print_summary() {
    printf("\n");
    for (int i = 0; i < 29; i++) printf("-");
    printf("\n| %-25s |\n", "SUMMARY");
    printf("| Plain files: %-12d |\n", global_stats->no_files);
    printf("| Directories: %-12d |\n", global_stats->no_dirs);
    printf("| Char devices: %-11d |\n", global_stats->no_char_devs);
    printf("| Block devices: %-10d |\n", global_stats->no_block_devs);
    printf("| Named pipes: %-12d |\n", global_stats->no_fifos);
    printf("| Symbolic link: %-10d |\n", global_stats->no_slinks);
    printf("| Sockets: %-16d |\n", global_stats->no_socks);
    for (int i = 0; i < 29; i++) printf("-");
}

void update_stats(struct dirent* entity) {
    switch (entity->d_type) {
        case DT_REG:
            global_stats->no_files++;
            break;
        case DT_DIR:
            global_stats->no_dirs++;
            break;
        case DT_CHR:
            global_stats->no_char_devs++;
            break;
        case DT_BLK:
            global_stats->no_block_devs++;
            break;
        case DT_FIFO:
            global_stats->no_fifos++;
            break;
        case DT_LNK:
            global_stats->no_slinks++;
            break;
        case DT_SOCK:
            global_stats->no_socks++;
            break;
    }
}

char* get_formatted_time(time_t time) {
    // Convert time to the formatted string
    char buff[20];
    struct tm* time_info;
    time_info = localtime(&time);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", time_info);
    int length = strlen(buff);

    // Copy formatted string to the persistent memory
    char* f_time = (char*) calloc(length + 1, sizeof(char));
    if (f_time == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }
    strcpy(f_time, buff);
    return f_time;
}

Node* create_ll_node(char* str) {
    // Allocate memory for the node struct
    Node *node = (Node*) malloc(sizeof(Node));
    if (node == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }

    // Assign text to the text attribute of a node
    node->text = (char*) calloc(strlen(str) + 1, sizeof(char));
    if (node->text == NULL) {
        perror("Error: Cannot allocate memory.\n");
        free(node);
        return NULL;
    }
    strcpy(node->text, str);
    node->next = NULL;
    return node;
}

Node* append_to_ll(Node *tail, char* str) {
    Node *node = create_ll_node(str);
    tail->next = node;
    return node;
}

void free_ll(Node *head) {
    Node* curr;

    while (head != NULL) {
        curr = head;
        head = head->next;
        free(curr->text);
        free(curr);
    }
}
