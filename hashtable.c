#include "hashtable.h"
#include <stdio.h>
#include <limits.h>


Node *create_node(const char *key, const char *value) {
    Node *new_node = malloc(sizeof(Node));
    new_node->key = (char*)malloc(strlen(key) + 1);
    new_node->value = (char*)malloc(strlen(value) + 1);
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->next = NULL;
    return new_node;
}


HashTable *create_table() {
    HashTable *hash_table = malloc(sizeof(HashTable));
    hash_table->table = malloc(sizeof(Node *) * TABLE_SIZE);

    for (int i = 0; i < TABLE_SIZE; i++) {
        hash_table->table[i] = NULL;
    }

    return hash_table;
}


unsigned int hash(const char *key) {
    unsigned long int hashval = 0;
    int i = 0;

    
    while (hashval < ULONG_MAX && i < (int)strlen(key)) {
        hashval = hashval << 8;
        hashval += key[i];
        i++;
    }

    return hashval % TABLE_SIZE;
}


void insert(HashTable *hash_table, const char *key, const char *value) {
    unsigned int index = hash(key);
    Node *new_node = create_node(key, value);
    Node *current = hash_table->table[index];

    if (current == NULL) {
        hash_table->table[index] = new_node;
    } else {
        
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}


char *lookup(HashTable *hash_table, const char *key) {
    unsigned int index = hash(key);
    Node *current = hash_table->table[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }

    
    return NULL;
}


void free_table(HashTable *hash_table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = hash_table->table[i];
        while (current != NULL) {
            Node *temp = current;
            current = current->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(hash_table->table);
    free(hash_table);
}
