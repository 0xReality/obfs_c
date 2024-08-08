#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void usage_exit(void) {
    fprintf(stderr, "usage: ./%s [files] <args>\n", APP_NAME);
    exit(1);
}

long nlines(FILE* file) {
    long line_count = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            ++line_count;
        }
    }

    fseek(file, 0, SEEK_SET); 

    return line_count;
}

 bool isValidFile(char* path) {
    int size = strlen(path);
    if (size > 3 && ((path[--size] == 'c' || path[size] == 'h') && path[--size] == '.')) {
        return true;
    }
    return false;
}

FILE* open_file(char* path) {
    if (path == NULL) return NULL;

    FILE* f = fopen(path, "r+");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    return f;
}

char* read_line(FILE* stream) {
    char* line = NULL;
    size_t len = 0;

    if (getline(&line, &len, stream) != -1) {
        return line;
    }

    free(line);
    return NULL;
}

long file_to_array(FILE* stream, char*** res) {
    long size = nlines(stream);
    *res = (char**) malloc(sizeof(char*) * size);

    for (long i = 0; i < size; i++) {
        (*res)[i] = read_line(stream);
    }

    return size;
}


char* file_to_string(FILE* stream) {
    fseek(stream, 0, SEEK_END);
    size_t size = ftell(stream); 
    fseek(stream, 0, SEEK_SET);  

    
    char* result = (char*)malloc(size + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    size_t bytesRead = fread(result, 1, size, stream);
    if (bytesRead != size) {
        fprintf(stderr, "Error reading file\n");
        free(result);
        exit(1);
    }

    result[size] = '\0';

    return result;
}


void read_array(char** data, long size) {
    for (long i = 0; i < size; i++) {
        printf("%s", data[i]);
    }
}



void free_array(char** data, long size){
    for (long i = 0; i < size; i++) {
        free(data[i]);
    }
    free(data);
}


char** split_line_into_words(const char* line, int* num_words) {
    int capacity = INITIAL_CAPACITY;
    char** words = (char**)malloc(capacity * sizeof(char*));
    if (!words) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *num_words = 0;
    const char* start = line;
    const char* end;

    while (*start) {
        
        while (isspace((unsigned char)*start)) {
            start++;
        }

        if (*start == '\0') break;

        
        if (strchr("=+-*/(),;[]{}", *start) && *start != '(') {
            char* token = (char*)malloc(2);
            if (!token) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            token[0] = *start;
            token[1] = '\0';

            if (*num_words >= capacity) {
                capacity *= 2;
                words = (char**)realloc(words, capacity * sizeof(char*));
                if (!words) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }

            words[*num_words] = token;
            (*num_words)++;
            start++;
            continue;
        }

        
        end = start;
        while (*end && (isalnum((unsigned char)*end) || *end == '_')) {
            end++;
        }
        
        if (*end == '(') {
            const char* func_end = end;
            int paren_count = 1;
            while (*func_end && paren_count > 0) {
                func_end++;
                if (*func_end == '(') paren_count++;
                if (*func_end == ')') paren_count--;
            }
            if (paren_count == 0) {
                func_end++;
                
                int func_length = func_end - start;
                char* function_call = (char*)malloc(func_length + 1);
                if (!function_call) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                
                strncpy(function_call, start, func_length);
                function_call[func_length] = '\0';

                if (*num_words >= capacity) {
                    capacity *= 2;
                    words = (char**)realloc(words, capacity * sizeof(char*));
                    if (!words) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                }

                words[*num_words] = function_call;
                (*num_words)++;
                start = func_end;

                
                while (*start && strchr("=+-*/(),;[]{}", *start)) {
                    char* punct = (char*)malloc(2);
                    if (!punct) {
                        perror("malloc");
                        exit(EXIT_FAILURE);
                    }
                    punct[0] = *start;
                    punct[1] = '\0';

                    if (*num_words >= capacity) {
                        capacity *= 2;
                        words = (char**)realloc(words, capacity * sizeof(char*));
                        if (!words) {
                            perror("realloc");
                            exit(EXIT_FAILURE);
                        }
                    }

                    words[*num_words] = punct;
                    (*num_words)++;
                    start++;
                }

                continue;
            }
        }

        end = start;
        while (*end && !isspace((unsigned char)*end) && !strchr("=+-*/(),;[]{}", *end)) {
            end++;
        }

        if (*end && strchr("=+-*/(),;[]{}", *end)) {
            int word_length = end - start;
            char* word = (char*)malloc(word_length + 1);
            if (!word) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            strncpy(word, start, word_length);
            word[word_length] = '\0';

            if (*num_words >= capacity) {
                capacity *= 2;
                words = (char**)realloc(words, capacity * sizeof(char*));
                if (!words) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }

            words[*num_words] = word;
            (*num_words)++;

            char* punct = (char*)malloc(2);
            if (!punct) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            punct[0] = *end;
            punct[1] = '\0';

            if (*num_words >= capacity) {
                capacity *= 2;
                words = (char**)realloc(words, capacity * sizeof(char*));
                if (!words) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }

            words[*num_words] = punct;
            (*num_words)++;
            start = end + 1;
        } else {
            int word_length = end - start;
            char* word = (char*)malloc(word_length + 1);
            if (!word) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            strncpy(word, start, word_length);
            word[word_length] = '\0';

            if (*num_words >= capacity) {
                capacity *= 2;
                words = (char**)realloc(words, capacity * sizeof(char*));
                if (!words) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }

            words[*num_words] = word;
            (*num_words)++;
            start = end;
        }
    }

    return words;
}


void free_words(char** words, int num_words) {
    for (int i = 0; i < num_words; i++) {
        free(words[i]);
    }
    free(words);
}