#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>


#define TABLE_SIZE 1024


typedef struct Node {
    char *key;
    char *value;
    struct Node *next;
} Node;


typedef struct HashTable {
    Node **table;
} HashTable;




Node *create_node(const char *key, const char *value);


HashTable *create_table();


unsigned int hash(const char *key);


void insert(HashTable *hash_table, const char *key, const char *value);


char *lookup(HashTable *hash_table, const char *key);


void free_table(HashTable *hash_table);

#endif 
