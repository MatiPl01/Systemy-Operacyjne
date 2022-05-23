#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>


typedef unsigned short us;
typedef unsigned long ul;

int no_threads;
us** source_image;
us** result_image;
int W, H, M;
int new_M = 0;

const char* TIME_FILE_NAME = "times.txt";
const char* NUMBERS_METHOD = "numbers";
const char* BLOCK_METHOD = "block";
const us MAX_POSSIBLE_COLOR = 255;

char* get_input_string(int *i, int argc, char* argv[], char* msg);
int get_input_num(int *i, int argc, char* argv[], char* msg);
int load_source_image(char* image_path);
int read_image_params(FILE* f_ptr);
us** allocate_image_memory(int width, int height);
ul* run_in_threads(char* method);
void* numbers_method(void* thread_idx);
void* block_method(void* thread_idx);
int create_negative_image(char* method, char* source_path, char* result_path);
int save_negative_image(char* result_path);
void free_image(us **image);
int write_times(ul total_time, ul *thread_times, char* method, char* source_path, char* result_path);
int min(int a, int b);


int main(int argc, char* argv[]) {
    int i = 1;
    char* method;
    char* source_path;
    char* result_path;

    // Get input values
    if ((no_threads = get_input_num(&i, argc, argv, "Please provide the number of threads")) <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        return EXIT_FAILURE;
    }
    if (!(method = get_input_string(&i, argc, argv, "Please provide the method of resources distribution between threads"))) {
        return EXIT_FAILURE;
    }
    if (strcmp(method, NUMBERS_METHOD) != 0 && strcmp(method, BLOCK_METHOD) != 0) {
        fprintf(stderr, "'%s' is not a valid distribution method name. Please use '%s' or '%s'\n",
                method, NUMBERS_METHOD, BLOCK_METHOD);
        free(method);
        return EXIT_FAILURE;
    }
    int exit_status = EXIT_SUCCESS;
    if (!(source_path = get_input_string(&i, argc, argv, "Please provide the sources file name")) ||
       (!(result_path = get_input_string(&i, argc, argv, "Please provide the result file name")))) {
        exit_status = EXIT_FAILURE;
    }

    // Load the sources image
    if (exit_status == EXIT_SUCCESS) {
        if (load_source_image(source_path) == -1
            || create_negative_image(method, source_path, result_path) == -1) {
            exit_status = EXIT_FAILURE;
        }
    }

    // Free memory
    if (source_path) free(source_path);
    if (result_path) free(result_path);

    return exit_status;
}


char* get_input_string(int *i, int argc, char* argv[], char* msg) {
    // If an argument was provided, return this argument
    if (*i < argc) {
        char* arg = malloc(strlen(argv[*i]) + 1);
        if (!arg) {
            perror("Unable to allocate memory for the input string\n");
            return NULL;
        }
        strcpy(arg, argv[(*i)++]);
        return arg;
    }

    // Otherwise, get an input from a user
    printf("%s\n>>> ", msg);
    char* line = "";
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

int get_input_num(int *i, int argc, char* argv[], char* msg) {
    char* str = get_input_string(i, argc, argv, msg);
    int num = (int) strtol(str, NULL, 10);
    free(str);
    return num;
}

int load_source_image(char* image_path) {
    printf("Loading: %s...\n", image_path);

    FILE* f_ptr;
    if (!(f_ptr = fopen(image_path, "r"))) {
        perror("Cannot open an image file\n");
        return -1;
    }

    // Read the parameters of the image (into the global variables)
    if (read_image_params(f_ptr) == -1) return -1;

    // Allocate memory for the image pixel values
    source_image = allocate_image_memory(W, H);

    // Read image pixel values
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            if (fscanf(f_ptr, "%hu", &source_image[i][j]) == EOF) {
                perror("Cannot read image pixel value. Not enough values are provided\n");
                return -1;
            }
        }
    }

    if (fclose(f_ptr) != 0) {
        perror("Cannot close a file\n");
        return -1;
    }
    return 0;
}

int read_image_params(FILE* f_ptr) {
    char* line = NULL;
    size_t size = 256; // Initial buffer size
    // Skip the line with the P2 value
    if (getline(&line, &size, f_ptr) == -1) {
        perror("Error while reading an image file\n");
        return -1;
    }

    // Skip empty lines and comments
    while (getline(&line, &size, f_ptr) > 0) {
        if (line[0] != '\n' && line[0] != '#') break;
    }

    // Read file dimensions
    if (sscanf(line, "%d %d", &W, &H) == EOF) {
        perror("Cannot read image dimensions\n");
        return -1;
    }

    // Free memory allocated with getline
    free(line);

    // Get max pixel value
    if (fscanf(f_ptr, "%d", &M) == EOF) {
        perror("Cannot read max pixel value\n");
        return -1;
    }

    return 0;
}

us** allocate_image_memory(int width, int height) {
    us** image;
    if (!(image = calloc(height, sizeof(us*)))) {
        perror("Cannot allocate memory for the sources image\n");
        return NULL;
    }
    for (int i = 0; i < H; i++) {
        if (!(image[i] = calloc(width, sizeof(us)))) {
            perror("Cannot allocate memory for the sources image\n");
            free(image);
            return NULL;
        }
    }

    return image;
}

int create_negative_image(char* method, char* source_path, char* result_path) {
    // Allocate memory for the resulting image
    if (!(result_image = allocate_image_memory(W, H))) return -1;

    // Start the total time measurements
    struct timeval start_t, end_t;
    gettimeofday(&start_t, NULL);

    // Create the negative image
    ul *thread_times;
    if (!(thread_times = run_in_threads(method))) return -1;

    // Finish the total time measurements
    gettimeofday(&end_t, NULL);
    ul total_t = (end_t.tv_sec - start_t.tv_sec) * 1000000 + end_t.tv_usec - start_t.tv_usec;

    // Write the result to the image file and to a console
    if (write_times(total_t, thread_times, method, source_path, result_path) == -1) return -1;

    // Save the negative image to the file
    if (save_negative_image(result_path) == -1) return -1;
    // Release memory
    free(thread_times);
    free_image(source_image);
    free_image(result_image);

    return 0;
}

ul* run_in_threads(char* method) {
    pthread_t *threads;
    int *thread_indexes;

    // Allocate memory for threads identifiers
    if (!(threads = calloc(no_threads, sizeof(pthread_t))) ||
        !(thread_indexes = calloc(no_threads, sizeof(int)))) {
        if (threads) free(threads);
        perror("Cannot allocate memory for threads identifiers\n");
        return NULL;
    }

    // Choose function used with the specified method
    void* (*start_routine)(void*) = strcmp(method, NUMBERS_METHOD) == 0 ? numbers_method : block_method;

    // Run the specified function in threads
    for (int i = 0; i < no_threads; i++) {
        thread_indexes[i] = i;
        if (pthread_create(&threads[i], NULL, start_routine, &thread_indexes[i]) != 0) {
            fprintf(stderr, "Error while creating a thread\n");
            return NULL;
        }
    }

    // Allocate memory for thread total execution times
    ul *execution_times;
    if (!(execution_times = calloc(no_threads, sizeof(ul)))) {
        perror("Cannot allocate memory for thread execution times\n");
        return NULL;
    }

    // Wait for threads to finish and store time of each thread execution
    for (int i = 0; i < no_threads; i++) {
        ul *execution_time;
        if (pthread_join(threads[i], (void**) &execution_time) != 0) {
            perror("Cannot wait for threads to finish\n");
            return NULL;
        }
        execution_times[i] = *execution_time;
        free(execution_time);
    }

    // Release memory
    free(threads);
    free(thread_indexes);

    return execution_times;
}

void* numbers_method(void* thread_idx) {
    // Set up the time measurements
    int idx = *((int *) thread_idx);
    struct timeval start_t, end_t;

    // Calculate minimum and maximum color values processed by teh current thread
    // (min_color will be included and Max_color will be excluded)
    int min_color = (M + 1) / no_threads * idx;
    int max_color = min((M + 1) / no_threads * (idx + 1), M + 1);

    // Start the time measurement
    gettimeofday(&start_t, NULL);

    // Calculate the negative image
    us curr_color;
    for (int i = 0; i < H; i++){
        for (int j = 0; j < W; j++){
            curr_color = source_image[i][j];
            if (min_color <= curr_color && curr_color < max_color) {
                result_image[i][j] = MAX_POSSIBLE_COLOR - curr_color;
                // Update the new max pixel color value
                if (result_image[i][j] > new_M) new_M = result_image[i][j];
            }
        }
    }

    // Finish the time measurement
    gettimeofday(&end_t, NULL);

    // Allocate memory for the value returned from a thread
    ul *total_t;
    if (!(total_t = malloc(sizeof(total_t)))) {
        perror("Cannot allocate memory for the value returned from a thread");
        exit(EXIT_FAILURE);
    }

    // Calculate the total thread execution time
    *total_t = (end_t.tv_sec - start_t.tv_sec) * 1000000 + end_t.tv_usec - start_t.tv_usec;

    // Pass the total execution time as the thread exit value and exit the current thread
    pthread_exit(total_t);
}

void* block_method(void* thread_idx) {
    // Set up the time measurements
    int idx = *((int *) thread_idx);
    struct timeval start_t, end_t;

    // Calculate column indexes processed by the curren thread
    int min_col_idx = (int) ceil((double) W / no_threads) * idx;
    int max_col_idx = min((int) ceil((double) W / no_threads) * (idx + 1), W);

    // Start the time measurement
    gettimeofday(&start_t, NULL);

    // Calculate the negative image
    for (int i = 0; i < H; i++){
        for (int j = min_col_idx; j < max_col_idx; j++){
            result_image[i][j] = MAX_POSSIBLE_COLOR - source_image[i][j];
            // Update the new max pixel color value
            if (result_image[i][j] > new_M) new_M = result_image[i][j];
        }
    }

    // Finish the time measurement
    gettimeofday(&end_t, NULL);

    // Allocate memory for the value returned from a thread
    ul *total_t;
    if (!(total_t = malloc(sizeof(total_t)))) {
        perror("Cannot allocate memory for the value returned from a thread");
        exit(EXIT_FAILURE);
    }

    // Calculate the total thread execution time
    *total_t = (end_t.tv_sec - start_t.tv_sec) * 1000000 + end_t.tv_usec - start_t.tv_usec;

    // Pass the total execution time as the thread exit value and exit the current thread
    pthread_exit(total_t);
}

int write_times(ul total_time, ul *thread_times, char* method, char* source_path, char* result_path) {
    // Open the times file
    FILE* f_ptr;
    if (!(f_ptr = fopen(TIME_FILE_NAME, "a"))) {
        perror("Cannot open a time measurements file\n");
        return -1;
    }

    // Write the header
    printf("\n***********************************************\n");
    printf("Number of threads:            %d\n", no_threads);
    printf("Threads distribution method:  %s\n", method);
    printf("Source file name:  %s\n", source_path);
    printf("Result file name:  %s\n", result_path);
    printf("***********************************************\n\n");

    fprintf(f_ptr, "***********************************************\n");
    fprintf(f_ptr, "Number of threads:            %d\n", no_threads);
    fprintf(f_ptr, "Threads distribution method:  %s\n", method);
    fprintf(f_ptr, "Source file name:  %s\n", source_path);
    fprintf(f_ptr, "Result file name:  %s\n", result_path);
    fprintf(f_ptr, "***********************************************\n\n");

    // Write execution time of each thread
    for (int i = 0; i < no_threads; i++) {
        printf("Thread %5d:\t%6lu [μs]\n", i, thread_times[i]);
        fprintf(f_ptr,"Thread %5d:\t%6lu [μs]\n", i, thread_times[i]);
    }

    // Write the total execution time
    printf("\nTotal time: %6lu [μs]\n", total_time);
    fprintf(f_ptr,"\nTotal time: %6lu [μs]\n", total_time);

    // Close the time measurements file
    if (fclose(f_ptr) != 0) {
        perror("Cannot close a file\n");
        return -1;
    }

    return 0;
}

int save_negative_image(char* result_path) {
    FILE *f_ptr;
    if (!(f_ptr = fopen(result_path, "w"))) {
        perror("Cannot save the negative image\n");
        return -1;
    }

    // Save image headers (skip author and comments)
    fprintf(f_ptr, "P2\n%d %d\n%d", W, H, new_M);

    // Writhe the image pixel color values
    for (int i = 0; i < H; i++) {
        fprintf(f_ptr, "\n");
        for (int j = 0; j < W; j++) {
            fprintf(f_ptr, "%d ", result_image[i][j]);
        }
    }

    // Close the result image file
    if (fclose(f_ptr) != 0) {
        perror("Cannot close a file\n");
        return -1;
    }

    return 0;
}

void free_image(us **image) {
    for (int i = 0; i < H; i++) free(image[i]);
    free(image);
}

int min(int a, int b) {
    return a > b ? b : a;
}

/*
5
block
./sources/dragon.ascii.pgm
./results/dragon.ascii.pgm
 */
