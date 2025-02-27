#ifndef CLOX_HASH_MAP_H
#define CLOX_HASH_MAP_H

// TODO: add a "value.h" file
#include "value.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const LoxString * key;
    LoxValue value;
} HashMapEntry;

typedef struct {
    size_t length;
    size_t capacity;
    HashMapEntry * entries;
} HashMap;

void map_init(HashMap * map);

bool map_set(HashMap * map, const LoxString * key, LoxValue value);
void map_add_all(HashMap * map, const HashMap * from);

const LoxValue * map_get(const HashMap * map, const LoxString * key);
bool map_delete(HashMap * map, const LoxString * key);

const LoxString * map_find_str(const HashMap * map, const char* chars, size_t length, uint32_t hash);
void map_destroy(HashMap * map);

uint32_t str_hash(const char* str, size_t length);

#endif 
