#ifndef CLOX_HASH_TABLE_H
#define CLOX_HASH_TABLE_H

// TODO: add a "value.h" file
#include "program.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const obj_string_t * key;
    value_t value;
} Entry;

typedef struct {
    size_t length;
    size_t capacity;
    Entry * entries;
} HashTable;

void ht_init(HashTable * table);

bool ht_set(HashTable * table, const obj_string_t * key, value_t value);
void ht_add_all(HashTable * table, const HashTable * from);

const value_t * ht_get(const HashTable * table, const obj_string_t * key);
bool ht_delete(HashTable * table, const obj_string_t * key);

obj_string_t * ht_find_str(const HashTable * table, const char* chars, size_t length, uint32_t hash);
void ht_destroy(HashTable * table);

#endif 
