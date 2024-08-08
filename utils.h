#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#define INITIAL_CAPACITY 25


#define APP_NAME "obfs"

void usage_exit(void);
long nlines(FILE* file);
// Check if file ext is .c or .h
bool isValidFile(char* path);
// Opens a new Stream on file path returns NULL if stream did not open
FILE* open_file(char* path);
char* read_line(FILE* stream);
long file_to_array(FILE* stream, char*** res);
void read_array(char** data, long size);
void free_array(char** data, long size);
char** split_line_into_words(const char* line, int* num_words);
void free_words(char** words, int num_words);
char* file_to_string(FILE* stream);


#endif

