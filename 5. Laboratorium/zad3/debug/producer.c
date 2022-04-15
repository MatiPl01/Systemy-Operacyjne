#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>


#define ROW_NUM_SEP ' '
#define EMPTY_CHAR ' '
#define ARG_COUNT 4
#define MAX_ROW_NUM 4096

int produce(FILE *fifo_ptr, FILE *file_ptr, int row_num, int read_count);
int format_row_num(int row_num, char* buff);


int main(int argc, char* argv[]) {
    printf("Producer %d was initialized.\n", getpid());
    if (argc != ARG_COUNT + 1) {
        fprintf(stderr, "Producer: Wrong umber of arguments. Expected %d, got %d.\n", ARG_COUNT, argc - 1);
        return EXIT_FAILURE;
    }

    char* fifo_path = argv[1];
    int row_num = (int) strtol(argv[2], NULL, 10);
    char* file_path = argv[3];
    int N = (int) strtol(argv[4], NULL, 10);

    FILE *fifo_ptr, *file_ptr;
    if (!(fifo_ptr = fopen(fifo_path, "w"))) {
        perror("Producer: Unable to open a named pipe.\n");
        return EXIT_FAILURE;
    }
    if (!(file_ptr = fopen(file_path, "r"))) {
        fprintf(stderr, "Producer: Unable to open a file %s.\n", file_path);
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    setbuf(fifo_ptr, NULL);
    setbuf(file_ptr, NULL);
    setbuf(stdout, NULL);

    int status = produce(fifo_ptr, file_ptr, row_num, N);
    fclose(fifo_ptr);
    fclose(file_ptr);

    printf("Producer %d finished.\n", getpid());
    return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


int produce(FILE *fifo_ptr, FILE *file_ptr, int row_num, int read_count) {
    char buff[read_count + 1];
    size_t curr_read_count;

    while ((curr_read_count = fread(buff, sizeof(char), read_count, file_ptr)) > 0) {
        sleep(rand() % 2 + 1);
        for (size_t i = curr_read_count; i < read_count; i++) buff[i] = EMPTY_CHAR;
        buff[read_count] = '\0';

        flock(fileno(fifo_ptr), LOCK_EX);

        char row_num_buff[32];
        if (format_row_num(row_num, row_num_buff) == -1) return -1;
        printf("Producer %d:\n\tWriting: '%s%c%s'\n", getpid(), row_num_buff, ROW_NUM_SEP, buff);
        fprintf(fifo_ptr, "%s%c%s\n", row_num_buff, ROW_NUM_SEP, buff);

        flock(fileno(fifo_ptr), LOCK_UN);
    }

    return 0;
}

int format_row_num(int row_num, char* buff) {
    int max_num = MAX_ROW_NUM;
    if (row_num > max_num) {
        fprintf(stderr, "Row number is too large.\n");
        return -1;
    }

    int max_digits_count = 0;
    while (max_num > 0) {
        max_digits_count++;
        max_num /= 10;
    }

    int i;
    buff[max_digits_count] = '\0';
    for (i = max_digits_count - 1; i >= 0; i--) {
        buff[i] = (char)(row_num % 10 + '0');
        row_num /= 10;
    }

    return 0;
}
