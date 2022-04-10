#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>


#define CMD_SEPARATOR "|"
#define DECLARATION_SIGN "="
#define READ_FD 0
#define WRITE_FD 1


typedef struct Node {
    struct Node *next;
    unsigned num;
    char* string;
} Node;

typedef struct Command {
    char*** parts_arr;
    unsigned parts_count;
} Command;

int exec_commands(FILE *f_ptr);
Command** get_commands(FILE *f_ptr, unsigned *max_id);
Node* get_commands_ll(FILE *f_ptr, unsigned *max_num);
Command* parse_command(char* string);
int get_command_id(const char* part);
char*** decode_command(char* line, Command** commands, unsigned *parts_count);
char** split_pipes(char* line, unsigned *parts_count);
char* trim_whitespace(char* string);
bool is_space(const char *string);
char** split_args(char* command, unsigned *args_count);
int exec_command_line(char*** command_line, unsigned parts_count);
bool check_processes_status(pid_t *pids, unsigned no_processes);
Node* create_ll_node(unsigned num, const char* string);
Node* append_to_ll(Node *tail, unsigned num, const char* string);
unsigned count_args(const char* command);
void free_ll(Node *head);
void free_commands_arr(Command** commands, unsigned length);
void free_strings_arr(char** arr, unsigned length);
void free_2D_strings_arr(char*** arr, unsigned length);
void print_command_info(char*** command_line, unsigned parts_count);


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
    unsigned max_id;  // Max index of a command
    unsigned parts_count;
    char* line = "";
    size_t line_length = 0;
    Command** commands = get_commands(f_ptr, &max_id);
    if (!commands) return -1;
    char*** command_line;

    while (getline(&line, &line_length, f_ptr) != -1) {
        if (is_space(line)) continue;
        if (!(command_line = decode_command(line, commands, &parts_count))) {
            free_commands_arr(commands, max_id);
            return -1;
        }
        print_command_info(command_line, parts_count);
        exec_command_line(command_line, parts_count);
        free(command_line);
        puts("");
    }

    free(line);
    free_commands_arr(commands, max_id + 1);

    return 0;
}

Command** get_commands(FILE *f_ptr, unsigned *max_id) {
    Node* head = get_commands_ll(f_ptr, max_id);
    if (!head) return NULL;

    Command** commands = (Command**) calloc(*max_id + 1, sizeof(Command*));
    if (!commands) {
        perror("Unable to allocate memory.\n");
        free_ll(head);
        return NULL;
    }

    Command* command;
    Node* curr = head->next;
    while (curr) {
        if (!(command = parse_command(curr->string))) {
            for (unsigned i = 0; i < *max_id; i++) {
                if (commands[i]) free(commands[i]);
            }
            free(commands);
            free_ll(head);
        }
        commands[curr->num] = command;
        curr = curr->next;
    }

    free_ll(head);
    return commands;
}

Command* parse_command(char* string) {
    Command* command = (Command*) calloc(1, sizeof(Command));
    if (!command) {
        perror("Unable to allocate memory.\n");
        return NULL;
    }

    unsigned splits_count;
    char** splits_array = split_pipes(string, &splits_count);
    if (!splits_array) {
        free(command);
        return NULL;
    }

    char*** parts_array = (char***) calloc(splits_count, sizeof(char**));
    if (!parts_array) {
        perror("Unable to allocate memory.\n");
        free_strings_arr(splits_array, splits_count);
        free(command);
        return NULL;
    }

    unsigned _;
    for (unsigned i = 0; i < splits_count; i++) {
        if (!(parts_array[i] = split_args(splits_array[i], &_))) {
            for (unsigned j = 0; j < i; j++) free_strings_arr(parts_array[j], 0);
            free_strings_arr(splits_array, splits_count);
            free(command);
        }
    }

    free_strings_arr(splits_array, splits_count);
    command->parts_count = splits_count;
    command->parts_arr = parts_array;

    return command;
}

Node* get_commands_ll(FILE *f_ptr, unsigned *max_num) {
    char* line = "";
    char* command;
    unsigned command_num;
    size_t line_length = 0;
    Node *head = create_ll_node(-1, NULL);
    if (!head) return NULL;
    Node *tail = head;

    *max_num = 0;
    while (getline(&line, &line_length, f_ptr) != -1) {
        // Break if a line without = sign was encountered (I assume that all declarations
        // are preceding lines defining the order of commands execution)
        if (!strstr(line, DECLARATION_SIGN)) break;
        // Use strtok_r to get also the remaining part (after = sign)
        if ((command_num = get_command_id(strtok_r(line, DECLARATION_SIGN, &command))) == -1) {
            free(line);
            free_ll(head);
            return NULL;
        }
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

    free(line);
    return head;
}

int get_command_id(const char* part) {
    int mul = 1;
    int res = 0;
    bool found_digit = false;
    size_t length = strlen(part);
    size_t i;

    for (i = length - 1; i > 0; i--) {
        if (isspace(part[i])) continue;
        int dgt = part[i] - '0';
        if (dgt < 0 || dgt > 9) break;
        found_digit = true;
        res += mul * dgt;
        mul *= 10;
    }

    if (!found_digit) {
        fprintf(stderr, "No command number was found.\n");
        return -1;
    }

    return res + 1;
}

char*** decode_command(char* line, Command** commands, unsigned *parts_count) {
    unsigned splits_count;
    char** splits = split_pipes(line, &splits_count);
    if (!splits) return NULL;

    unsigned ids[splits_count];
    *parts_count = 0;
    for (unsigned i = 0; i < splits_count; i++) {
        if ((ids[i] = get_command_id(splits[i])) == -1) {
            free_strings_arr(splits, splits_count);
            return NULL;
        }
        *parts_count += commands[ids[i]]->parts_count;
    }

    char*** result = (char***) calloc(*parts_count + 1, sizeof(char**));
    if (!result) {
        free_strings_arr(splits, splits_count);
        perror("Unable to allocate memory.\n");
        return NULL;
    }

    unsigned res_idx = 0;
    for (unsigned i = 0; i < splits_count; i++) {
        for (unsigned j = 0; j < commands[ids[i]]->parts_count; j++) {
            result[res_idx++] = commands[ids[i]]->parts_arr[j];
        }
    }

    result[*parts_count] = NULL;

    free_strings_arr(splits, splits_count);
    return result;
}

char** split_pipes(char* line, unsigned *parts_count) {
    *parts_count = 1;
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == *CMD_SEPARATOR) (*parts_count)++;
    }

    char** result = (char**) calloc(*parts_count + 1, sizeof(char*));
    if (!result) {
        perror("Unable to allocate memory.\n");
        return NULL;
    }

    unsigned i = 0;
    char* part = strtok(line, CMD_SEPARATOR);
    while (part) {
        part = trim_whitespace(part);
        if (!(result[i] = (char*) calloc(strlen(part) + 1, sizeof(char)))) {
            free_strings_arr(result, i);
            perror("Unable to allocate memory.\n");
            free_strings_arr(result, i);
            return NULL;
        }
        strcpy(result[i], part);
        part = strtok(NULL, CMD_SEPARATOR);
        i++;
    }

    result[*parts_count] = NULL;
    return result;
}

char* trim_whitespace(char* string) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*string)) string++;

    if (*string == 0) return string;

    // Trim trailing space
    end = string + strlen(string) - 1;
    while (end > string && isspace((unsigned char) *end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return string;
}

bool is_space(const char *string) {
    const char *c = string;
    while (*c != '\0') {
        if (!isspace(*c++)) return false;
    }
    return true;
}

char** split_args(char* command, unsigned *args_count) {
    *args_count = count_args(command);

    char** args = (char**) calloc(*args_count + 1, sizeof(char*));
    if (!args) {
        perror("Unable to allocate memory");
        return NULL;
    }

    unsigned i = 0;
    char* arg = strtok(command, " \t\n");
    while (arg) {
        if (!(args[i] = (char*) calloc(strlen(arg) + 1, sizeof(char)))) {
            free_strings_arr(args, i);
            perror("Unable to allocate memory");
            return NULL;
        }
        strcpy(args[i], arg);
        arg = strtok(NULL, " \t\n");
        i++;
    }
    args[*args_count] = NULL;

    return args;
}

unsigned count_args(const char* command) {
    unsigned count = 1;  // There will be 1 more argument than a number of whitespaces separating them
    bool was_prev_space = false;
    for (size_t i = 0; command[i] != '\0'; i++) {
        if (isspace(command[i]) && !was_prev_space) {
            was_prev_space = true;
            count++;
        } else {
            was_prev_space = false;
        }
    }
    return count;
}

int exec_command_line(char*** command_line, unsigned parts_count) {
    int curr_fd[2], prev_fd[2];
    unsigned i;
    pid_t process_pids[parts_count];

    for (i = 0; i < parts_count; i++) {
        if (pipe(curr_fd) == -1) {
            perror("Unable to open a pipe.\n");
            return -1;
        }

        pid_t pid = fork();
        process_pids[i] = pid;

        if (pid < 0) {
            perror("Unable to create a child process.\n");
            close(curr_fd[READ_FD]);
            close(curr_fd[WRITE_FD]);
            if (i > 0) {
                close(prev_fd[READ_FD]);
                close(prev_fd[WRITE_FD]);
            }
            return -1;
        }

        if (pid == 0) {
            if (i > 0) {
                dup2(prev_fd[READ_FD], STDIN_FILENO);
                close(prev_fd[WRITE_FD]);
                close(prev_fd[READ_FD]);
            }
            // Duplicate the file descriptor for all processes except the last one
            // (the last process should print the results to the console)
            if (i < parts_count - 1) {
                dup2(curr_fd[WRITE_FD], STDOUT_FILENO);
                close(curr_fd[READ_FD]);
                close(curr_fd[WRITE_FD]);
            }

            execvp(command_line[i][0], command_line[i]);
            // Exec function only returns there was an exception while executing
            // the specified command
            perror("Unable to execute a command.\n");
            exit(1);
        }

        close(prev_fd[READ_FD]);
        close(prev_fd[WRITE_FD]);

        if (i < parts_count) {
            prev_fd[READ_FD] = curr_fd[READ_FD];
            prev_fd[WRITE_FD] = curr_fd[WRITE_FD];
        }
    }

    if (!check_processes_status(process_pids, parts_count)) return -1;

    return 0;
}

bool check_processes_status(pid_t *pids, unsigned no_processes) {
    int status;
    for (unsigned i = 0; i < no_processes; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            fprintf(stderr, "Cannot wait for a child process.\n");
            return false;
        }
        // Check if a process finished with an error status code
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            fprintf(stderr, "There was an error in a child process with PID %d\n", pids[i]);
            return false;
        }
    }
    return true;
}

Node* create_ll_node(unsigned num, const char* string) {
    // Allocate memory for a node struct and a string
    Node *node = (Node*) calloc(1, sizeof(Node));
    if (node && string) node->string = (char*) calloc(strlen(string) + 1, sizeof(char));

    if (!node || (string && !node->string)) {
        fprintf(stderr, "Unable to allocate memory.\n");
        return NULL;
    }

    if (string) strcpy(node->string, string);
    node->num = num;
    return node;
}

Node* append_to_ll(Node *tail, unsigned num, const char* string) {
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
        free(curr->string);
        free(curr);
    }
}

void free_strings_arr(char** arr, unsigned length) {
    if (length == 0) {
        for (unsigned i = 0; arr[i] != NULL; i++) free(arr[i]);
    } else {
        for (unsigned i = 0; i < length; i++) free(arr[i]);
    }
    free(arr);
}

void free_2D_strings_arr(char*** arr, unsigned length) {
    for (unsigned i = 0; i < length; i++) free_strings_arr(arr[i], 0);
}

void free_commands_arr(Command** commands, unsigned length) {
    for (unsigned i = 0; i < length; i++) {
        if (commands[i]) free_2D_strings_arr(commands[i]->parts_arr, commands[i]->parts_count);
    }
    free(commands);
}

void print_command_info(char*** command_line, unsigned parts_count) {
    printf("COMMAND: ");
    for (unsigned i = 0; i < parts_count; i++) {
        for (unsigned j = 0; command_line[i][j]; j++) {
            printf(" %s", command_line[i][j]);
        }
        if (i < parts_count - 1) printf(" |");
    }
    printf("\nRESULT:\n");
}
