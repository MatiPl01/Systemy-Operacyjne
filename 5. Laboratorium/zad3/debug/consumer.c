#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

#define ROW_NUM_SEP ' '
#define EMPTY_CHAR ' '
#define ARG_COUNT 3
#define MAX_ROW_NUM 4096

typedef struct Node {
    struct Node *next;
    size_t length;
    char* string;
} Node;

int consume(FILE *fifo_ptr, char* file_path, int read_count);
int save_to_results_file(char* path, int row_num, char* content);
int append_to_file_row(int row_num, char* content, Node *head);
//int write_data_to_file(FILE *write_ptr, Node *head);
int write_data_to_file(int fd, Node *head);
int read_from_fifo(FILE *fifo_ptr, int read_count, char* content_buff, int *row_num);
//Node* read_file_lines(FILE *file_ptr);
Node* read_file_lines(int fd);
Node* create_ll_node(char* string);
Node* append_to_ll(Node *tail, char* string);
int create_empty_file(char* path);
void free_ll(Node *head);
int calc_max_row_num_digits(void);
char next_char(int fd);
int get_file_size(int fd);
int get_line_length(int fd);
int get_cursor_pos(int fd);
char* read_line(int fd);


int main(int argc, char* argv[]) {
    printf("Consumer %d was initialized.\n", getpid());
    if (argc != ARG_COUNT + 1) {
        fprintf(stderr, "Consumer: Wrong number of arguments. Expected %d, got %d.\n", ARG_COUNT, argc - 1);
        return EXIT_FAILURE;
    }

    char* fifo_path = argv[1];
    char* file_path = argv[2];
    int N = (int) strtol(argv[3], NULL, 10);

    if (create_empty_file(file_path) == -1) return EXIT_FAILURE;

    FILE *fifo_ptr;
    if (!(fifo_ptr = fopen(fifo_path, "r"))) {
        perror("Consumer: Unable to open a named pipe.\n");
        return EXIT_FAILURE;
    }

    setbuf(fifo_ptr, NULL);
    setbuf(stdout, NULL);

    int status = consume(fifo_ptr, file_path, N);
    fclose(fifo_ptr);

    printf("Consumer %d finished.\n", getpid());
    return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


int consume(FILE *fifo_ptr, char *file_path, int read_count) {
    char buff[read_count + 1];
    int row_num;

    while (read_from_fifo(fifo_ptr, read_count, buff, &row_num) > 0) {
        printf("Consumer %d:\n\tReceived:\n\t\tRow:     %d\n\t\tContent: %s\n", getpid(), row_num, buff);

        if (row_num < 1) {
            fprintf(stderr, "Consumer: Invalid row number [%d].\n", row_num);
            return -1;
        }

        if (save_to_results_file(file_path, row_num, buff) == -1) return -1;
    }

    return 0;
}

int read_from_fifo(FILE *fifo_ptr, int read_count, char* content_buff, int *row_num) {
    int max_row_num_digits = calc_max_row_num_digits();
    int read_with_row_count = read_count + max_row_num_digits + 2; // +2 because of the space and aa newline
    char *buff = (char*) calloc(read_with_row_count + 1, sizeof(char));
    if (!buff) {
        perror("Unable to allocate memory.\n");
        return -1;
    }

    char sep[] = { ROW_NUM_SEP };

    if (fread(buff, sizeof(char), read_with_row_count, fifo_ptr) > 1) { // >1 because of the \n cter
        char* content;
        char* num_str = strtok_r(buff, sep, &content);
        *row_num = (int) strtol(num_str, NULL, 10);

        // Remove trailing whitespaces
        for (int i = read_count - 1; i >= 0; i--) {
            if (content[i] != EMPTY_CHAR || content[i] != '\n') {
                content[++i] = '\0';
                break;
            }
            read_count--;
        }

        strcpy(content_buff, content);
        free(buff);
        return read_count;
    }

    return 0;
}

//int save_to_results_file(char* file_path, int row_num, char* content) {
//    FILE *read_ptr = fopen(file_path, "r");
//
//    if (!read_ptr) {
//        perror("Consumer: Unable to open a file for reading.\n");
//        return -1;
//    }
//
//    int read_fd = fileno(read_ptr);
//    flock(read_fd, LOCK_EX);
//    printf(">>> READING LOCKED IN %d\n", getpid());
//
//    Node *head = read_file_lines(read_ptr);
//    fclose(read_ptr);
//
//    if (!head) {
//        fprintf(stderr, "Consumer: Unable to create a file lines linked list.\n");
//        return -1;
//    }
//
//    puts("\n=== READ FROM A FILE ===");
//    Node *curr = head->next;
//    while (curr) {
//        printf("%d %s", (int) curr->length, curr->string);
//        curr = curr->next;
//    }
//    puts("=== END OF READ FROM A FILE ===\n");
//
//    if (append_to_file_row(row_num, content, head) == -1) {
//        fprintf(stderr, "Consumer: Unable to append content to the specified file row.\n");
//        free_ll(head);
//        return -1;
//    }
//
//    FILE *write_ptr = fopen(file_path, "w");
//    int write_fd = fileno(write_ptr);
//    flock(write_fd, LOCK_EX);
//    printf(">>> WRITING LOCKED IN %d\n", getpid());
//
//    if (!write_ptr) {
//        perror("Consumer: Unable to open a file for writing.\n");
//        free_ll(head);
//        return -1;
//    }
//
//    if (write_data_to_file(write_ptr, head) == -1) {
//        fclose(write_ptr);
//        free_ll(head);
//        return -1;
//    }
//
//    free_ll(head);
//    fclose(write_ptr);
//    flock(read_fd, LOCK_UN);
//    flock(write_fd, LOCK_UN);
//    printf(">>> READING UNLOCKED IN %d\n", getpid());
//    printf(">>> WRITING UNLOCKED IN %d\n", getpid());
//
//    return 0;
//}

int save_to_results_file(char* file_path, int row_num, char* content) {
    int read_fd = open(file_path, O_RDONLY|O_EXCL);
    if (read_fd < 0) {
        perror("Consumer: Unable to open a file for reading.\n");
        return -1;
    }
    flock(read_fd, LOCK_EX);
    printf(">>> READING LOCKED IN %d\n", getpid());

    Node *head = read_file_lines(read_fd);
    close(read_fd);

    if (!head) {
        fprintf(stderr, "Consumer: Unable to create a file lines linked list.\n");
        return -1;
    }

    puts("\n=== READ FROM A FILE ===");
    Node *curr = head->next;
    while (curr) {
        printf("%d %s", (int) curr->length, curr->string);
        curr = curr->next;
    }
    puts("=== END OF READ FROM A FILE ===\n");

    if (append_to_file_row(row_num, content, head) == -1) {
        fprintf(stderr, "Consumer: Unable to append content to the specified file row.\n");
        free_ll(head);
        return -1;
    }

    int write_fd = open(file_path, O_WRONLY|O_EXCL);
    flock(write_fd, LOCK_EX);
    printf(">>> WRITING LOCKED IN %d\n", getpid());

    if (write_fd < 0) {
        perror("Consumer: Unable to open a file for writing.\n");
        free_ll(head);
        return -1;
    }

    if (write_data_to_file(write_fd, head) == -1) {
        close(write_fd);
        free_ll(head);
        return -1;
    }

    free_ll(head);
    close(write_fd);
    flock(read_fd, LOCK_UN);
    flock(write_fd, LOCK_UN);
    printf(">>> READING UNLOCKED IN %d\n", getpid());
    printf(">>> WRITING UNLOCKED IN %d\n", getpid());

    return 0;
}

int create_empty_file(char* path) {
    FILE *f_ptr;
    if (!(f_ptr = fopen(path, "w"))) {
        perror("Consumer: Unable to create a file.\n");
        return -1;
    }
    fclose(f_ptr);
    return 0;
}

int append_to_file_row(int row_num, char* content, Node *head) {
    Node *prev = head;
    // Go to the node representing a row preceding the desired row
    int curr_row_num = head->next ? 1 : 0;
    while (curr_row_num < row_num) {
        if (prev->next) prev = prev->next;
        if (!prev->next && !append_to_ll(prev, "\n")) {
            fprintf(stderr, "Consumer: Unable to append new row node to he linked list.\n");
            return -1;
        }
        curr_row_num++;
    }

    // Modify string in the prev->next node
    Node *curr = prev->next;
    size_t new_length = curr->length + strlen(content);
    char* old_string = curr->string;
    old_string[curr->length - 1] = '\0';
    char* new_string = (char*) calloc(new_length + 1, sizeof(char));
    if (!new_string) {
        perror("Consumer: Unable to allocate memory.\n");
        return -1;
    }
    strcpy(new_string, old_string);
    strcat(new_string, content);
    new_string[new_length - 1] = '\n';
    free(old_string);
    curr->string = new_string;
    curr->length = new_length;

    return 0;
}

//Node* read_file_lines(FILE *file_ptr) {
//    Node *head = create_ll_node(NULL);
//    Node *tail = head;
//
//    char* line = "";
//    size_t line_length = 0;
//    rewind(file_ptr);
//    while (getline(&line, &line_length, file_ptr) != -1) {
//        if (!(tail = append_to_ll(tail, line))) {
//            free_ll(head);
//            return NULL;
//        }
//    }
//    free(line);
//
//    if (ferror(file_ptr)) {
//        fprintf(stderr, "Consumer: Error while reading a file line.\n");
//        free_ll(head);
//        return NULL;
//    }
//
//    return head;
//}
//
//int write_data_to_file(FILE *write_ptr, Node *head) {
//    Node *curr = head->next;
//
//    while (curr) {
//        if (fwrite(curr->string, sizeof(char), curr->length, write_ptr) < curr->length) {
//            fprintf(stderr, "Consumer: Error writing to a file.\n");
//            return -1;
//        }
//        curr = curr->next;
//    }
//
//    return 0;
//}

char next_char(int fd) {
    char* c = (char*) calloc(1, sizeof(char));
    // Return '\0' character if the next char cannot be read
    if (read(fd, c, 1) < 1) {
        free(c);
        return '\0';
    }
    char res = c[0];
    free(c);
    return res;
}

int get_file_size(int fd) {
    // Get the current cursor position
    int curr_pos = get_cursor_pos(fd);
    // Move the cursor to the end of a file in order to determine
    // the file size
    int size = (int) lseek(fd, 0, SEEK_END);
    // Move the cursor to the previous position
    lseek(fd, curr_pos, SEEK_SET);
    return size;
}

int get_cursor_pos(int fd) {
    return (int) lseek(fd, 0, SEEK_CUR);
}

int get_line_length(int fd) {
    int offset = 0;
    char c;

    do {
        c = next_char(fd);
        offset++;

        // If the next char cannot be read
        if (c == '\0') {
            // Check if the end of a file has been reached
            if (get_cursor_pos(fd) >= get_file_size(fd)) {
                lseek(fd, -(--offset), SEEK_CUR);
                return offset;
            }
                // Otherwise, there is an error
            else {
                printf("Error: Cannot read the next character.");
                return -1;
            }
        }
    } while (c != '\n');

    // Move the cursor back to its previous position
    lseek(fd, -offset, SEEK_CUR);

    return offset;
}

char* read_line(int fd) {
    int length = get_line_length(fd);
    // Check if there is an error
    if (length < 0) return NULL;
    char* line = (char*) calloc(length + 1, sizeof(char));
    // Check if allocation was successful
    if (line == NULL) {
        perror("Error: Cannot allocate memory for a file line.\n");
        return NULL;
    }
    // Read a line from the file
    int read_length = (int) read(fd, line, length);
    if (read_length < length) {
        printf("Error: Cannot read a line from a file.\n");
        free(line);
        return NULL;
    }
    return line;
}

Node* read_file_lines(int fd) {
    Node *head = create_ll_node(NULL);
    Node *tail = head;

    char* line;
    lseek(fd, 0, SEEK_SET);

    while (get_line_length(fd) > 0) {
        line = read_line(fd);
        if (!line || !(tail = append_to_ll(tail, line))) {
            free_ll(head);
            return NULL;
        }
        free(line);
    }

    return head;
}

int write_data_to_file(int fd, Node *head) {
    Node *curr = head->next;

    while (curr) {
        if (write(fd, curr->string, curr->length) == -1) {
            perror("Consumer: Error writing to a file.\n");
            return -1;
        }
        curr = curr->next;
    }

    return 0;
}

Node* create_ll_node(char* string) {
    // Allocate memory for the node struct
    Node *node = (Node*) calloc(1, sizeof(Node));
    if (!node) {
        perror("Consumer: Unable to allocate memory.\n");
        return NULL;
    }

    if (string) {
        size_t length = strlen(string);
        if (!(node->string = (char*) calloc(length + 1, sizeof(char)))) {
            perror("Consumer: Unable to allocate memory.\n");
            return NULL;
        }
        strcpy(node->string, string);
        node->length = length;
    }

    return node;
}

Node* append_to_ll(Node *tail, char* string) {
    Node *node = create_ll_node(string);
    if (!node) return NULL;
    tail->next = node;
    return node;
}

void free_ll(Node *head) {
    Node* curr;

    while (head != NULL) {
        curr = head;
        head = head->next;
        if (curr->string) free(curr->string);
        free(curr);
    }
}

int calc_max_row_num_digits(void) {
    int max_num = MAX_ROW_NUM;
    int digits_count = 0;

    while (max_num > 0) {
        max_num /= 10;
        digits_count++;
    }

    return digits_count;
}
