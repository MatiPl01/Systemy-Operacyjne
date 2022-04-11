#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>


#define ROW_NUM_SEP "#"
#define ARG_COUNT 3

int consume(FILE *fifo_ptr, FILE *file_ptr, unsigned read_count);
int save_to_results_file(FILE *file_ptr, unsigned row_num, char* content);


int main(int argc, char* argv[]) {
    if (argc != ARG_COUNT + 1) {
        fprintf(stderr, "Wrong umber of arguments. Expected %d, got %d.\n", ARG_COUNT, argc - 1);
        return 1;
    }

    char* fifo_path = argv[1];
    char* file_path = argv[2];
    unsigned N = (unsigned) strtol(argv[3], NULL, 10);

    FILE *fifo_ptr, *file_ptr;
    if (!(fifo_ptr = fopen(fifo_path, "r"))) {
        perror("Unable to open a named pipe.\n");
        return 2;
    }
    if (!(file_ptr = fopen(file_path, "w"))) {
        perror("Unable to open a file.\n");
        return 2;
    }

    int status = consume(fifo_ptr, file_ptr, N);
    fclose(fifo_ptr);
    fclose(file_ptr);

    return status == 0 ? 0 : 3;
}


int consume(FILE *fifo_ptr, FILE *file_ptr, unsigned read_count) {
    char buff[read_count + 1];

    while (fread(buff, sizeof(char), read_count, fifo_ptr)) { // TODO - it should not work (no space for row number in a buffer -> won't read enough from a fifo)
        printf("Consumer read: %s\n", buff);

        char* row_num_str = strtok(buff, ROW_NUM_SEP);
        unsigned row_num = (unsigned) strtol(row_num_str, NULL, 10);

        char* content = strtok(NULL, "\n\0");
        save_to_results_file(file_ptr, row_num, content);
    }

    return 0;
}

int save_to_results_file(FILE *file_ptr, unsigned row_num, char* content) {
    rewind(file_ptr);
    int fd = fileno(file_ptr);

    flock(fd, LOCK_EX);

    char c = 0;
    unsigned curr_row_num = 1; // Number file lines starting from 1
    while (c != EOF) {
        c = (char) fgetc(file_ptr);
        if (c == '\n') curr_row_num++;
        if (curr_row_num == row_num) {
            if (fprintf(file_ptr, content, strlen(content) + 1) < 0) {
                perror("Error writing to a file.\n");
                return -1;
            }
            fflush(file_ptr);
            break;
        }
    }

    flock(fd, LOCK_UN);
    return 0;
}
