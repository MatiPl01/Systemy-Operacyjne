#ifndef LIBFILESSEARCH_H
#define LIBFILESSEARCH_H

#include <stdbool.h>

bool search_files(char* start_path, char* searched_str, int max_depth);
int search_str_in_file(char* file_path, char* searched_str);

#endif // LIBFILESSEARCH_H
