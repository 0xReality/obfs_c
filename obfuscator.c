#include "obfuscator.h"
#include "hashtable.h"
#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>


HashTable *hash_table;
bool isInit = false;

void init(){
    hash_table = create_table();
    isInit = true;
}

void free_obfs(){
    free_table(hash_table);
}


bool is_preprocessor_directive(const char* line) {
    return line[0] == '#';
}

uint32_t xor128()
{
	static uint32_t x = 123456789, y = 362436069, z = 521288629,
	                w = 88675123;
	uint32_t t = (x^(x<<11)); x = y; y = z; z = w; return w = (w^(w>>19))^(t^(t>>8));
}

char* random_string(size_t length) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *randomString = malloc(length + 1);
    if (randomString) {

        srand((unsigned int)(time(NULL) ^ getpid() ^ xor128()));
        
        for (size_t n = 0; n < length; n++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            randomString[n] = charset[key];
        }
        randomString[length] = '\0';
    }
    return randomString;
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void strip_pointer_symbols(char* str) {
    char* src = str;
    char* dest = str;

    while (*src) {
        if (*src != '*') {
            *dest++ = *src;
        }
        src++;
    }
    *dest = '\0';
}

bool is_c_keyword(const char* word) {
    const char* keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "if", "inline", "int", "long", "register", "restrict", "return",
        "short", "signed", "sizeof", "static", "struct", "switch",
        "typedef", "union", "unsigned", "void", "volatile", "while",
        "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
        "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local", "(", ")", ";", "main",
        "NULL"
    };
    
    size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    
    char word_copy[256];
    strncpy(word_copy, word, sizeof(word_copy) - 1);
    word_copy[sizeof(word_copy) - 1] = '\0';

    strip_pointer_symbols(word_copy);

    for (size_t i = 0; i < num_keywords; i++) {
        if (strcmp(word_copy, keywords[i]) == 0) {
            return true;
        }
    }

    return false;
}

bool is_operator(const char* token) {
    if (token == NULL) {
        return false;
    }
    const char* operators[] = {"+", "-", "*", "/", "%", "=", "==", "!=", "<", "<=", ">", ">="};
    size_t num_operators = sizeof(operators) / sizeof(operators[0]);

    for (size_t i = 0; i < num_operators; ++i) {
        if (strcmp(token, operators[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

bool is_in_quotes(const char* str) {
    if (str == NULL || strlen(str) < 2) {
        return false;
    }
    return ((str[0] == '"' || str[strlen(str) - 1] == '"') ||
            (str[0] == '\'' || str[strlen(str) - 1] == '\''));
}

bool is_number(const char* str) {
    char* endPtr;
    strtol(str, &endPtr, 10 ); 
    if (endPtr == str)return false;
    else return true;
}

char* remove_comments(const char* input) {
    int len = strlen(input);
    char* result = (char*)malloc(len + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    int in_single_line_comment = 0;
    int in_multi_line_comment = 0;
    int j = 0;

    for (int i = 0; i < len; i++) {
        if (in_single_line_comment) {
            if (input[i] == '\n') {
                in_single_line_comment = 0;
                result[j++] = input[i];
            }
        } else if (in_multi_line_comment) {
            if (input[i] == '*' && input[i + 1] == '/') {
                in_multi_line_comment = 0;
                i++;
            }
        } else {
            if (input[i] == '/' && input[i + 1] == '/') {
                in_single_line_comment = 1;
                i++;
            } else if (input[i] == '/' && input[i + 1] == '*') {
                in_multi_line_comment = 1;
                i++;
            } else {
                result[j++] = input[i];
            }
        }
    }

    result[j] = '\0';
    return result;
}

char* whitespace_remover(const char* input) {
    size_t len = strlen(input);
    char* result = (char*)malloc(len * 2 + 1); 
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    int in_string = 0;
    int in_char = 0;
    int in_single_line_comment = 0;
    int in_multi_line_comment = 0;
    int in_preprocessor_directive = 0;
    size_t j = 0;

    for (size_t i = 0; i < len; i++) {
        if (in_string) {
            result[j++] = input[i];
            if (input[i] == '"' && input[i - 1] != '\\') {
                in_string = 0;
            }
        } else if (in_char) {
            result[j++] = input[i];
            if (input[i] == '\'' && input[i - 1] != '\\') {
                in_char = 0;
            }
        } else if (in_single_line_comment) {
            result[j++] = input[i];
            if (input[i] == '\n') {
                in_single_line_comment = 0;
            }
        } else if (in_multi_line_comment) {
            result[j++] = input[i];
            if (input[i] == '*' && input[i + 1] == '/') {
                result[j++] = input[++i];
                in_multi_line_comment = 0;
            }
        } else if (in_preprocessor_directive) {
            result[j++] = input[i];
            if (input[i] == '\n') {
                in_preprocessor_directive = 0;
            }
        } else {
            if (input[i] == '#') {
                in_preprocessor_directive = 1;
                result[j++] = input[i];
            } else if (input[i] == '"') {
                in_string = 1;
                result[j++] = input[i];
            } else if (input[i] == '\'') {
                in_char = 1;
                result[j++] = input[i];
            } else if (input[i] == '/' && input[i + 1] == '/') {
                result[j++] = input[i++];
                result[j++] = input[i];
                in_single_line_comment = 1;
            } else if (input[i] == '/' && input[i + 1] == '*') {
                result[j++] = input[i++];
                result[j++] = input[i];
                in_multi_line_comment = 1;
            } else if (is_whitespace(input[i])) {
                while (i < len && is_whitespace(input[i])) {
                    i++;
                }
                if (i < len && (isalpha(input[i]) || input[i] == '#')) {
                    result[j++] = ' ';
                }
                i--;
            } else {
                
                if (strncmp(&input[i], "return", 6) == 0 && !isalnum(input[i + 6]) && input[i + 6] != ';') {
                    strcpy(&result[j], "return");
                    j += 6;
                    
                    if (i + 6 < len && input[i + 6] != ';') {
                        result[j++] = ' ';
                    }
                    i += 5; 
                } else {
                    result[j++] = input[i];
                }
            }
        }
    }

    result[j] = '\0';
    return result;
}

char* words_to_string(char** words, int size) {
    if (size == 0) {
        char* empty_string = malloc(1);
        if (empty_string == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        empty_string[0] = '\0';
        return empty_string;
    }

    int total_length = 0;
    for (int i = 0; i < size; ++i) {
        total_length += strlen(words[i]);
    }
    total_length += (size - 1);

    char* result = malloc(total_length + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    result[0] = '\0';
    for (int i = 0; i < size; ++i) {
        strcat(result, words[i]);
        if (i < size - 1) {
            strcat(result, " ");
        }
    }

    return result;
}

bool is_function(char* token ) {
    if (token == NULL) return false;
    int size = strlen(token);
    if (size < 1) return false;

    bool has_open_paren = false;    
    int paren_count = 0;
    for (int i = 0; i < size; i++) {
        if (token[i] == '(') {
            has_open_paren = true;
            paren_count++;
        } else if (token[i] == ')') {
            paren_count--;
            if (paren_count == 0) {
                return has_open_paren;
            }
        }
    }

    return has_open_paren && paren_count == 0;
}

bool is_control_statement(const char* token) {
    int size = strlen(token);
    for (int i = 0; i < size; i++) {
        if (i + 2 <= size && token[i] == 'i' && token[i + 1] == 'f') {
            return true;
        } else if (i + 2 <= size && token[i] == 'f' && token[i + 1] == 'o' && token[i + 2] == 'r') return true;
        else if (i + 4 <= size && token[i] == 'w' && token[i + 1] == 'h' && token[i + 2] == 'i' && token[i + 3] == 'l' && token[i + 4] == 'e') return true;
        else if (i + 5 <= size && token[i] == 's' && token[i + 1] == 'w' && token[i + 2] == 'i' && token[i + 3] == 't' && token[i + 4] == 'c' && token[i + 5] == 'h') return true;
    }
    return false;
}

void pointer_check(char* pre_var, char* post_var){
    int s_counter = 0;
    while (*post_var) {
        if (*post_var == '*') {
            s_counter++;
        }
        post_var++;
    }
    for(int i=0;i<s_counter;i++){
        strcat(pre_var, "*");
    }
    
}


int is_increment_decrement(char* token) {
    int size = strlen(token);
    
    if (size >= 2 && ((token[size - 2] == '+' && token[size - 1] == '+'))) {
        token[size - 2] = '\0';  
        return 2;
    }

    if ((size >= 2 && token[size - 2] == '-' && token[size - 1] == '-')) {
        token[size - 2] = '\0';  
        return -2;
    }


    if (size >= 2 && ((token[0] == '+' && token[1] == '+'))) {
        memmove(token, token + 2, size - 1);
        token[size - 2] = '\0'; 
        return 1;
    }

        if (size >= 2 && (token[0] == '-' && token[1] == '-')) {
        memmove(token, token + 2, size - 1);
        token[size - 2] = '\0'; 
        return -1;
    }

    return 0;
}


void insert_brackets(char *pre_var, const char *post_var, char *result) {
    
    const char *p = post_var;
    char *r = result;
    
    
    while (*pre_var && *pre_var != '[') {
        *r++ = *pre_var++;
    }

    
    while (*p && *p != '[') {
        *r++ = *p++;
    }
    if (*p == '[') {
        
        while (*p) {
            *r++ = *p++;
        }
    }
    
    *r = '\0'; 
}


void stack_check(char *pre_var, char *post_var) {
    
    char result[256];
    
    
    result[0] = '\0';

    
    insert_brackets(pre_var, post_var, result);
    
    printf("Transformed pre_var: %s\n", result);
    
    
    memset(post_var, 0, strlen(post_var));
    
    
    strcpy(post_var, result);
}

void fix_incr_decr(char* token, int v) {
    int len = strlen(token);
    char* new_token;

    
    if (v == 1 || v == -1) {
        new_token = malloc(len + 3);  
        if (new_token == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        if (v == 1) {
            strcpy(new_token, "++");  
        } else {
            strcpy(new_token, "--");  
        }
        strcpy(new_token + 2, token);  
    } else if (v == 2 || v == -2) {
        new_token = malloc(len + 3);  
        if (new_token == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        strcpy(new_token, token);  
        if (v == 2) {
            strcpy(new_token + len, "++");  
        } else {
            strcpy(new_token + len, "--");  
        }
    } else {
        fprintf(stderr, "Invalid value for v\n");
        return;
    }

    
    strcpy(token, new_token);
    free(new_token);
}


char* obfuscate_variable(char* token) {

    int add_inc_dec = is_increment_decrement(token);


    char* hashCheck = lookup(hash_table, token);
    if(hashCheck != NULL){

        fix_incr_decr(hashCheck, add_inc_dec);
        return hashCheck; 

    }


    size_t len = strlen(token);
    if(strcmp(token, "  ") == 0){
        return token;
    }
    char* obfuscated = malloc(len + 6); 
    if (!obfuscated) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    size_t i = 0, j = 0;

    
    while (i < len && (isalnum(token[i]) || token[i] == '_')) {
        obfuscated[j++] = token[i++];
    }

    
    char* random_str = random_string(5);
    memcpy(obfuscated, random_str, 5);
    j += 5;
    
    while (i < len) {
        obfuscated[j++] = token[i++];
    }

    obfuscated[j] = '\0';

    insert(hash_table, token, obfuscated);
    if(add_inc_dec != 0) fix_incr_decr(obfuscated, add_inc_dec);
    return obfuscated;
}



char* parse_state(char* base) {
    char* start = strchr(base, '(');
    if (!start) {
        return NULL; 
    }
    start++; 

    char* end = strchr(start, ')');
    if (!end) {
        return NULL;
    }

    size_t length = end - start;
    char* res = malloc(length + 1); 
    if (!res) {
        return NULL; 
    }

    strncpy(res, start, length);
    res[length] = '\0'; 

    return res;
}



char* obfuscate_line(char* expr) {
    if(isInit != true){
        fprintf(stderr, "HashTable not init\n");
        exit(1);
    }
    int size;
    char** tokens = split_line_into_words(expr, &size);
    if (tokens == NULL) return NULL;

    bool variable_context = false;
    bool quote_contex = false;
    size_t obfuscated_expr_size = strlen(expr) * 2 + 1;
    char* obfuscated_expr = malloc(obfuscated_expr_size); 
    obfuscated_expr[0] = '\0';  

    for (int i = 0; i < size; i++) {
        printf("Processing token: %s\n", tokens[i]);

        
        if(is_preprocessor_directive(tokens[i])){
            printf("token: %s is a pre processor\n", tokens[i]);
            strcat(obfuscated_expr, expr);
            strcat(obfuscated_expr, "\n");
            break;
        }
        
        if ((is_control_statement(tokens[i])) || is_function(tokens[i])) {
            printf("token: %s is a control statement or a function\n", tokens[i]);

            
            char* open_paren = strchr(tokens[i], '(');
            if (open_paren) {
                *open_paren = '\0';
                open_paren++;
                char* close_paren = strrchr(open_paren, ')');
                if (close_paren) {
                    *close_paren = '\0';

                    
                    char* obfuscated_content = obfuscate_line(open_paren);

                    
                    strcat(obfuscated_expr, tokens[i]);
                    strcat(obfuscated_expr, "(");
                    strcat(obfuscated_expr, obfuscated_content);
                    strcat(obfuscated_expr, ")");
                    
                    
                    if (i + 1 < size && strcmp(tokens[i + 1], ";") == 0) {
                        strcat(obfuscated_expr, ";");
                        i++; 
                    }

                    
                }
            } else {
                strcat(obfuscated_expr, tokens[i]);
            }
        } else if (is_c_keyword(tokens[i])) {
            printf("token: %s is a keyword\n", tokens[i]);
            strcat(obfuscated_expr, tokens[i]);
        } else if (is_operator(tokens[i])) {
            printf("token: %s is an operator\n", tokens[i]);
            strcat(obfuscated_expr, tokens[i]);
            variable_context = false; 
        } else if (is_in_quotes(tokens[i])) {
            printf("token: %s is in quotes\n", tokens[i]);
            strcat(obfuscated_expr, tokens[i]);
            quote_contex = !quote_contex;
            continue;
        } else if (is_number(tokens[i])) {
            printf("token: %s is a number\n", tokens[i]);
            strcat(obfuscated_expr, tokens[i]);
            continue;
        } else if (strcmp(tokens[i], "{") == 0 || strcmp(tokens[i], "}") == 0 || strcmp(tokens[i], ",") == 0 || strcmp(tokens[i], ";") == 0) {
            
            printf("token: %s is punctuation\n", tokens[i]);
            strcat(obfuscated_expr, tokens[i]);
        } else {
            
            if (!variable_context && !quote_contex) {
                char* obfuscated_token = obfuscate_variable(tokens[i]);
                if (obfuscated_token) {
                    printf("Obfuscating variable: %s -> %s\n", tokens[i], obfuscated_token);
                    strcat(obfuscated_expr, obfuscated_token);
                } else {
                    strcat(obfuscated_expr, tokens[i]);
                }
            } else {
                strcat(obfuscated_expr, tokens[i]);
            }
        }

        
        if (i < size - 1) {
            strcat(obfuscated_expr, " ");
        }
    }

    char* final_expr = whitespace_remover(obfuscated_expr);
    free(obfuscated_expr);

    printf("new expr: %s\n", final_expr);
    return final_expr;
}
