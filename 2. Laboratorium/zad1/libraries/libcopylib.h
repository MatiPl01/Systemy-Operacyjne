#ifndef LABORATORIA_COPYLIB_H
#define LABORATORIA_COPYLIB_H

bool copy_file_lib(char* source_path, char* target_path);

int get_line_length(FILE* f_ptr);

char* read_line(FILE* f_ptr);

bool is_whitespace(char c);

bool is_line_empty(char* line);

bool copy_file_helper(FILE *source_ptr, FILE *target_ptr);

bool write_file(FILE* f_ptr, char* line);

char* find_next_line(FILE* f_ptr);

bool reached_EOF(FILE* f_ptr);

#endif //LABORATORIA_COPYLIB_H
