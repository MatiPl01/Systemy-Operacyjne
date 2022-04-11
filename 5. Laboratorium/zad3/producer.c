#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>


#define ROW_NUM_SEP '#'
#define ARG_COUNT 4

int produce(FILE *fifo_ptr, FILE *file_ptr, unsigned row_num, unsigned read_count);


int main(int argc, char* argv[]) {
    if (argc != ARG_COUNT + 1) {
        fprintf(stderr, "Wrong umber of arguments. Expected %d, got %d.\n", ARG_COUNT, argc - 1);
        return 1;
    }

    char* fifo_path = argv[1];
    unsigned row_num = (unsigned) strtol(argv[2], NULL, 10);
    char* file_path = argv[3];
    unsigned N = (unsigned) strtol(argv[4], NULL, 10);

    FILE *fifo_ptr, *file_ptr;
    if (!(fifo_ptr = fopen(fifo_path, "w"))) {
        perror("Unable to open a named pipe.\n");
        return 2;
    }
    if (!(file_ptr = fopen(file_path, "r"))) {
        perror("Unable to open a file.\n");
        return 2;
    }

    int status = produce(fifo_ptr, file_ptr, row_num, N);
    fclose(fifo_ptr);
    fclose(file_ptr);

    return status == 0 ? 0 : 3;
}


int produce(FILE *fifo_ptr, FILE *file_ptr, unsigned row_num, unsigned read_count) {
    char buff[read_count + 1];

    while (fread(buff, sizeof(char), read_count, file_ptr)) {
        sleep(rand() % 2 + 1);
        printf("Producer %d is writing to the fifo...\n\tMessage:\n\t%s\n", getpid(), buff);

        if (fprintf(fifo_ptr, "%d%c%s\n", row_num, ROW_NUM_SEP, buff) == -1) {
            perror("Unable to write to a fifo.\n");
            return -1;
        }

        fflush(fifo_ptr);
    }

    bool is_ok = ferror(fifo_ptr) == 0;
    if (is_ok) fprintf(stderr, "Something went wrong while reading a file.\n");
    return is_ok ? 0 : -1;
}
