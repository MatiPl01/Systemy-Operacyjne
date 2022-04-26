#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "libfilessearch.h"


typedef long long ll;

typedef struct Node {
    unsigned value;
    struct Node* next;
} Node;

static char* merge_path(char* dir_path, char* entity_path);
static bool search_files_recur(char* dir_path, char* rel_path, char* searched_str, int remaining_depth);
static bool check_children_status(Node *pids_ll);

Node* create_ll_node(unsigned value);
Node* append_to_ll(Node *tail, unsigned value);
void free_ll(Node *head);
void print_headers();

bool search_files(char* start_path, char* searched_str, int max_depth) {
    print_headers();
    return search_files_recur(start_path, ".", searched_str, max_depth);
}

static bool search_files_recur(char* dir_path, char* rel_path, char* searched_str, int remaining_depth) {
//    if (remaining_depth == 1) return false; // <- testing if error checking works
    if (remaining_depth <= 0) return true;

    // Try to open the specified directory
    DIR *d_ptr = NULL;
    d_ptr = opendir(dir_path);
    if (d_ptr == NULL) {
        fprintf(stderr, "Error: Cannot open a directory %s.\n", dir_path);
        return false;
    }

    char* entity_path;
    char* rel_entity_path;
    struct dirent* entity;

    // Create a sentinel node for the children pids linked list
    Node *pids_head = create_ll_node(0);
    if (!pids_head) return false;
    Node *pids_tail = pids_head;

    // Loop over all entities in a directory
    while (true) {
        entity = readdir(d_ptr);
        if (entity == NULL) break;
        if (strcmp(entity->d_name, ".") == 0 || strcmp(entity->d_name, "..") == 0) continue;

        // If an entity is a directory or a regular file
        if (entity->d_type == DT_DIR || entity->d_type == DT_REG) {
            // Create a path of an entity
            if ((entity_path = merge_path(dir_path, entity->d_name)) == NULL) {
                fprintf(stderr, "Error: Cannot get entity path.\n");
                free_ll(pids_head);
                closedir(d_ptr);
                return false;
            }
            // Create a path of an entity relative to the start directory
            if ((rel_entity_path = merge_path(rel_path, entity->d_name)) == NULL) {
                fprintf(stderr, "Error: Cannot get relative directory path.\n");
                closedir(d_ptr);
                free_ll(pids_head);
                free(entity_path);
                return false;
            }
            // Search files recursively if the current entity is a directory
            if (entity->d_type == DT_DIR) {
                unsigned pid = fork();
                // Child process
                if (pid == 0) {
                    // Search a directory and check if there were no errors
                    if (!search_files_recur(entity_path, rel_entity_path, searched_str, remaining_depth - 1)) {
                        exit(1);
                    }
                    exit(0);
                // Parent process
                } else {
                    // Add the child process pid to the linked list
                    if (!(pids_tail = append_to_ll(pids_tail, pid))) {
                        free(entity_path);
                        free(rel_entity_path);
                        free_ll(pids_head);
                        closedir(d_ptr);
                        return false;
                    }
                }
            // Otherwise, check if the current entity is a text file and includes
            // the searched string
            } else {
                int res = search_str_in_file(entity_path, searched_str);
                if (res == -1) {
                    free(entity_path);
                    free(rel_entity_path);
                    free_ll(pids_head);
                    closedir(d_ptr);
                    return false;
                }
                printf("%10d | %9s | %s \n", getpid(), res == 1 ? "yes" : "no", rel_entity_path);
            }
            // Release memory
            free(entity_path);
            free(rel_entity_path);
        }
    }

    // Close the current directory
    if (closedir(d_ptr) == -1) {
        free_ll(pids_head);
        fprintf(stderr, "Error: Cannot close a directory %s.\n", dir_path);
        return false;
    }

    // Check if all children processes finished with no errors
    bool status = check_children_status(pids_head);
    free_ll(pids_head);
    return status;
    return true;
}

void print_headers() {
    printf("%10s | %9s | %s\n", "PID", "INCLUDES?", "RELATIVE PATH");
    printf("--------------------------------------\n");
}

static char* merge_path(char* dir_path, char* entity_path) {
    char buff[PATH_MAX];

    // Merge the directory path with the entity path
    sprintf(buff, "%s/%s", dir_path, entity_path);
    char* path = (char*) calloc(strlen(buff) + 1, sizeof(char));

    if (path == NULL) {
        fprintf(stderr, "Error: Cannot allocate memory.\n");
        return NULL;
    }

    strcpy(path, buff);
    return path;
}

int search_str_in_file(char* file_path, char* searched_str) {
    // Open a file
    FILE *f_ptr = fopen(file_path, "r");
    if (f_ptr == NULL) {
        fprintf(stderr, "Error: Cannot open a file %s.\n", file_path);
        return -1;
    }

    // Get a file size
    fseek(f_ptr, 0, SEEK_END);
    ll f_size = ftell(f_ptr);
    rewind(f_ptr);

    // Allocate memory for a file's content
    char* f_content = (char*) calloc(f_size + 1, sizeof(char));
    if (f_content == NULL) {
        fprintf(stderr, "Error: Cannot allocate memory.\n");
        return -1;
    }

    // Read a content of a file
    ll read_length = (ll) fread(f_content, sizeof(char), f_size, f_ptr);
    if (read_length < f_size) {
        fprintf(stderr, "Error: Something went wrong while reading a file %s.\n", file_path);
        return -1;
    }

    // Check if a file content contains the searched string
    char* res = strstr(f_content, searched_str);
    free(f_content);
    fclose(f_ptr);

    return res != NULL;
}

static bool check_children_status(Node *pids_ll) {
    int status;
    Node *curr = pids_ll->next;

    while (curr) {
        // Wait for thr current child process to finish and get its exit status code
        if (waitpid(curr->value, &status, 0) == -1) {
            fprintf(stderr, "Error: Cannot wait for a child process.\n");
            return false;
        }
        // Check if a process exited with an error status code
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            fprintf(stderr, "Error: There was an error in a child process with PID %d\n", curr->value);
            return false;
        }
        curr = curr->next;
    }

    return true;
}

Node* create_ll_node(unsigned value) {
    // Allocate memory for the node struct
    Node *node = (Node*) calloc(1, sizeof(Node));
    if (!node) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }
    node->value = value;
    return node;
}

Node* append_to_ll(Node *tail, unsigned value) {
    Node *node = create_ll_node(value);
    if (!node) return NULL;
    tail->next = node;
    return node;
}

void free_ll(Node *head) {
    Node* curr;

    while (head) {
        curr = head;
        head = head->next;
        free(curr);
    }
}
