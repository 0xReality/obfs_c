#ifndef OBFS_H
#define OBFS_H

#include "utils.h"
#include "hashtable.h"

void init();
void free_obfs();

char* remove_comments(const char* input);

char* whitespace_remover(const char* input);

char* obfs_secure(char* code);

char* obfuscate_line(char* expr);



#endif