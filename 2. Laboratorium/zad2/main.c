#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define LIB_COUNT_LIB "./libraries/libcountlib.h"
#define LIB_COUNT_SYS "./libraries/libcountsys.h"

#ifdef LIB_SYS
    #include LIB_COUNT_SYS
#else
    #include LIB_COUNT_LIB
#endif


#ifdef MEASURE_TIME
    #include <sys/times.h>
    #include <unistd.h>

    struct tms tms_start_buffer, tms_end_buffer;
    clock_t clock_t_start, clock_t_end;

    void start_timer() {
        clock_t_start = times(&tms_start_buffer);
    }

    void stop_timer() {
        clock_t_end = times(&tms_end_buffer);
    }

    char* get_times_header() {
        char* s = (char*) calloc(34, sizeof(char));
        if (s == NULL) {
            perror("Error: Cannot allocate memory.\n");
            return NULL;
        }
        sprintf(s, "%-10s %-10s %-10s\n", "Real", "System", "User");
        return s;
    }

    double calc_time(clock_t end, clock_t start) {
        return (double)(end - start) / (double) sysconf(_SC_CLK_TCK);
    }

    char* get_time_str(double time) {
        char* s = (char*) calloc(12, sizeof(char));
        if (s == NULL) {
            perror("Error: Cannot allocate memory.\n");
            return NULL;
        }
        char temp[11];
        sprintf(temp, "%.2f", time);
        sprintf(s, "%-10s ", temp);
        return s;
    }

    char* get_times_str() {
        char* s = (char*) calloc(35, sizeof(char));
        int n = 3;
        double times[] = {
                calc_time(clock_t_end, clock_t_start),
                calc_time(tms_end_buffer.tms_stime, tms_start_buffer.tms_stime),
                calc_time(tms_end_buffer.tms_cutime, tms_start_buffer.tms_cutime)
        };
        for (int i = 0; i < n; i++) {
            char* t_s = get_time_str(times[i]);
            if (t_s == NULL) {
                perror("Error: Cannot create times string.\n");
                return NULL;
            }
            strcat(s, t_s);
            free(t_s);
        }
        strcat(s, "\n");
        return s;
    }
#endif


char get_input_char(int argc, char* argv[]);
char* get_input_path(int argc, char* argv[]);
char input_char(char* mess);
char* input_line(char* mess);
bool write_file(FILE* f_ptr, char* text);


int main(int argc, char* argv[]) {
    if (argc > 3) {
        perror("Error: Too many arguments.\n");
        return 1;
    }
    // Get input arguments
    char c = get_input_char(argc, argv);
    char* path = get_input_path(argc, argv);

    // Open time measurements file
    #ifdef MEASURE_TIME
        FILE *f_ptr = fopen(MEASURE_TIME, "a");

        if (f_ptr == NULL) {
            perror("Error: Failed to open the time measurements file.\n");
            return 1;
        }

        char* header = get_times_header();

        if (header == NULL) {
            perror("Error: Failed to create times header.\n");
            return 1;
        }

        bool writing_success = write_file(f_ptr, header);
        free(header);

        if (!writing_success) {
            perror("Error: Cannot write to the time measurements file.\n");
            fclose(f_ptr);
            return 1;
        }

        start_timer();
    #endif

    // Count occurrences of the specified character and a number of
    // rows where this character appears
    CountingResult *cr = count_char_in_file(c, path);

    // Save time measurements
    #ifdef MEASURE_TIME
        stop_timer();
        char* times = get_times_str();
        writing_success = write_file(f_ptr, times);
        free(times);
        fclose(f_ptr);
        if (!writing_success) {
            perror("Error: Cannot write to the time measurements file.\n");
            return 1;
        }
    #endif

    // Check if counting operation was successful
    if (cr == NULL) {
        perror("Error: Cannot perform character counting.\n");
        return 1;
    }

    // Print results to the stdout
    printf("Number of characters: %d\n", cr->no_all);
    printf("Number of rows:       %d\n", cr->no_rows);

    free(cr);

    return 0;
}


char get_input_char(int argc, char* argv[]) {
    if (argc < 2) return input_char("Please provide a single character to count.");
    if (strlen(argv[1]) > 1) {
        fprintf(stderr, "Error: Expected a single character, got '%s'\n", argv[1]);
        return '\0';
    }
    return argv[1][0];
}

char* get_input_path(int argc, char* argv[]) {
    if (argc < 3) return input_line("Please provide a path to a file.");
    return argv[2];
}

char input_char(char* mess) {
    printf("%s\n>>>", mess);
    char c;
    scanf(" %c", &c);
    getchar();
    return c;
}

char* input_line(char* mess) {
    printf("%s\n>>>", mess);
    char* line;
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

bool write_file(FILE* f_ptr, char* text) {
    int length = (int) strlen(text);
    int written_length = (int) fwrite(text, sizeof(char), length, f_ptr);

    if (written_length < length) {
        perror("Error: Cannot write a line to the file.\n");
        return false;
    }
    return true;
}
