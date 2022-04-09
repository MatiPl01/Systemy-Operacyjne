#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct Node {
    struct Node *next;
    char* string;
    int num;
} Node;

int exec_commands(FILE *f_ptr);
int get_command_num(char* part);
char** get_commands(FILE *f_ptr, int *max_idx);
Node* get_commands_ll(FILE *f_ptr, int *max_num);

Node* create_ll_node(int num, char* string);
Node* append_to_ll(Node *tail, int num, char* string);
void free_ll(Node *head);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Expected 2 arguments, got %d\n", argc);
        return 1;
    }

    char* file_path = argv[1];
    FILE *f_ptr = fopen(file_path, "r");

    if (!f_ptr) {
        perror("Unable to open the commands file.\n");
        return 1;
    }

    if (exec_commands(f_ptr) == -1) {
        fclose(f_ptr);
        return 1;
    }

    fclose(f_ptr);

    return 0;
}


int exec_commands(FILE *f_ptr) {
    int max_idx;  // Max index of command
    char** commands = get_commands(f_ptr, &max_idx);

    for (int i = 0; i <= max_idx; i++) {
        if (commands[i]) puts(commands[i]);
    }

    for (int i = 0; i <= max_idx; i++) free(commands[i]);
    free(commands);

    return 0;
}

int get_command_num(char* part) {
    int length = (int) strlen(part);
    char* num_substr = part + length - 2;
    for (int i = length - 1; i >= 0; i--, num_substr--) {
        int num = part[i] - '0';
        if (num < 0 || num > 9) break;
    }

    return (int) strtol(num_substr, NULL, 10);
}

Node* get_commands_ll(FILE *f_ptr, int *max_num) {
    char* line = "";
    char* command;
    int command_num;
    size_t line_length = 0;
    Node *head = create_ll_node(-1, NULL);
    if (!head) return NULL;
    Node *tail = head;

    *max_num = 0;
    while (getline(&line, &line_length, f_ptr) != -1) {
        if (strstr(line, "=")) {
            // Use strtok_r to get also the remaining part (after = sign)
            command_num = get_command_num(strtok_r(line, "=", &command));
            // Remove the \n character
            command[strlen(command) - 1] = '\0';

            if (command_num > *max_num) *max_num = command_num;
            if (!(tail = append_to_ll(tail, command_num, command++))) {  // command++ is to skip the = sign
                fprintf(stderr, "Unable to append a node to the linked list.\n");
                free(line);
                free_ll(head);
                return NULL;
            }
        }
    }

    free(line);
    return head;
}

char** get_commands(FILE *f_ptr, int *max_idx) {
    Node* head = get_commands_ll(f_ptr, max_idx);
    if (!head) return NULL;

    printf("%d\n", *max_idx);
    // Create an array of declared commands
    char** commands = (char**) calloc(*max_idx + 1, sizeof(char*));
    if (!commands) {
        perror("Unable to allocate memory.\n");
        free_ll(head);
        return NULL;
    }

    Node* curr = head->next;
    while (curr) {
        commands[curr->num] = curr->string;
        curr = curr->next;
    }

    free_ll(head);

    return commands;
}

Node* create_ll_node(int num, char* string) {
    // Allocate memory for a node struct and a string
    Node *node = (Node*) calloc(1, sizeof(Node));
    if (node && string) node->string = (char*) calloc(strlen(string) + 1, sizeof(char));

    if (!node || (string && !node->string)) {
        perror("Unable to allocate memory.\n");
        return NULL;
    }

    if (string) strcpy(node->string, string);
    node->num = num;
    return node;
}

Node* append_to_ll(Node *tail, int num, char* string) {
    Node *node = create_ll_node(num, string);
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
