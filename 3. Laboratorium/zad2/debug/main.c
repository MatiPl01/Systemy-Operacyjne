#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../library/libintegrate.h"


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


typedef long double ld;

ld get_rect_width(int argc, char* argv[]);
int get_no_processes(int argc, char* argv[]);
bool write_file(FILE* f_ptr, char* text);

ld f(ld x) {
    return 4 / (x * x + 1);
}


int main(int argc, char* argv[]) {
    ld step = get_rect_width(argc, argv);
    if (step < 0) return 1;
    int no_processes = get_no_processes(argc, argv);
    if (no_processes < 0) return 1;
    ld a = 0, b = 1;

    // Open time measurements file
#ifdef MEASURE_TIME
    FILE *f_ptr = fopen(MEASURE_TIME, "a");

        if (f_ptr == NULL) {
            perror("Error: Failed to open the time measurements file.\n");
            return 1;
        }

        start_timer();
#endif

    ld result = integrate_async(f, a, b, step, no_processes);

    // Save time measurements
#ifdef MEASURE_TIME
    stop_timer();

        char* header = get_times_header();

        if (header == NULL) {
            fprintf(stderr, "Error: Failed to create times header.\n");
            return 1;
        }

        bool writing_success = write_file(f_ptr, header);
        free(header);

        if (!writing_success) {
            printf("Error: Cannot write to the time measurements file.\n");
            fclose(f_ptr);
            return 1;
        }

        char* times = get_times_str();
        writing_success = write_file(f_ptr, times);
        free(times);
        fclose(f_ptr);
        if (!writing_success) {
            printf("Error: Cannot write to the time measurements file.\n");
            return 1;
        }
#endif

    if (result < 0) {
        fprintf(stderr, "Error: Cannot calculate the integral.\n");
        return 1;
    }

    printf("Integration result: %Lf\n", result);

    return 0;
}

ld get_rect_width(int argc, char* argv[]) {
    ld width;

    if (argc < 2) {
        printf("Please provide a width of the precision rectangle\n>>> ");
        scanf("%Lf", &width);

        if (width <= 0) {
            fprintf(stderr, "Error: A width of a rectangle should be a positive number.\n");
            return -1;
        }
    } else {
        width = atof(argv[1]);
    }

    return width;
}

int get_no_processes(int argc, char* argv[]) {
    int no_processes;

    if (argc < 3) {
        printf("Please provide a number of processes\n>>> ");
        scanf("%d", &no_processes);

        if (no_processes <= 0) {
            fprintf(stderr, "Error: A number of processes should be a positive number.\n");
            return -1;
        }
    } else {
        no_processes = atoi(argv[2]);
    }

    return no_processes;
}

bool write_file(FILE* f_ptr, char* text) {
    printf("%s", text);
    int length = (int) strlen(text);
    int written_length = (int) fwrite(text, sizeof(char), length, f_ptr);

    if (written_length < length) {
        printf("Error: Cannot write a line to the file.\n");
        return false;
    }
    return true;
}
