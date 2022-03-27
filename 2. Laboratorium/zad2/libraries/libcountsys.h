#ifndef COUNTSYS_H
#define COUNTSYS_H

typedef struct {
    int no_all;
    int no_rows;
} CountingResult;

CountingResult* count_char_in_file(char c, char* path);

int count_char_in_line(char c, char* line);

#endif //COUNTSYS_H
