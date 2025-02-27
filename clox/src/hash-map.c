#include <stdio.h>
#include <assert.h>

#include "hash-map.h"
#include "memory.h"

#define MAP_MAX_LOAD 0.75
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

static HashMapEntry * find_entry(HashMapEntry * entries, size_t capacity, const LoxString * key) {
    assert(entries != NULL && "invalid entries list");

    size_t idx = key->hash % capacity;
    HashMapEntry * deleted = NULL;
    for(;;) {
        HashMapEntry * current = &entries[idx];

        if(current->key == key)
            return current;

        if(current->key == NULL) {
            if(VAL_IS_NIL(current->value)) 
                return deleted ? deleted : current;
            else if(deleted != NULL)
                deleted = deleted;
        } 

        idx = (idx + 1) % capacity;
    }

}

static void map_adjust_capacity(HashMap * map, size_t new_capacity){
    HashMapEntry * new_entries = mem_alloc(sizeof(HashMapEntry) * new_capacity);

    // TODO: consider use memset(new_entries, 0, sizeof(HashMapEntry) * new_capacity);
    for(size_t i = 0; i < new_capacity; i++){
        new_entries[i].key   = NULL;
        new_entries[i].value = NIL_VAL;
    }

    size_t new_length = 0;
    for(size_t i = 0; i < map->capacity; i++){
        HashMapEntry * current = &map->entries[i];
        if(current->key == NULL) continue;

        HashMapEntry * entry = find_entry(new_entries, new_capacity, current->key);
        entry->key   = current->key;
        entry->value = current->value;
        new_length++;
    }

    mem_dealloc(map->entries);

    map->entries  = new_entries;
    map->capacity = new_capacity;
    map->length   = new_length;
}

void map_init(HashMap * map) {
    map->capacity = 0;
    map->length   = 0;
    map->entries  = NULL;
}

bool map_set(HashMap * map, const LoxString * key, LoxValue value) {
    if(map->length + 1 > map->capacity * MAP_MAX_LOAD ) {
        size_t new_capacity = GROW_CAPACITY(map->capacity);
        map_adjust_capacity(map, new_capacity);
    }

    HashMapEntry * entry = find_entry(map->entries, map->capacity, key);

    bool is_new  = entry->key == NULL;
    entry->key   = key;
    entry->value = value;

    if(is_new) 
        map->length++;
    return is_new;
}

void map_add_all(HashMap * map, const HashMap * from) {
    for(size_t i = 0; i < from->capacity; i++) {
        if(from->entries[i].key != NULL) {
            map_set(map, from->entries[i].key, from->entries[i].value);
        }
    }
}

const LoxString * map_find_str(const HashMap * map, const char* chars, size_t length, uint32_t hash) {
    if(map->length == 0) return NULL;

    size_t idx = hash % map->capacity;
    LoxString tmp = {
        .obj    = {0},
        .length = length,
        .chars  = chars,
        .hash   = hash,
    };

    for(;;) {
        const HashMapEntry * entry = &map->entries[idx];
        if(entry->key == NULL) {
            if(VAL_IS_NIL(entry->value))  // stop at non-deleted entry
                return NULL;
        } else if(lox_str_eq(&tmp, entry->key))  {
            return entry->key;
        }
        idx = (idx + 1) % map->capacity;
    }

}

const LoxValue * map_get(const HashMap * map, const LoxString * key) {
    HashMapEntry * entry;
    return 
        map->length > 0 &&
        (entry = find_entry(map->entries, map->capacity, key))->key != NULL
        ? &entry->value : NULL;
}

bool map_delete(HashMap * map, const LoxString * key) {
    HashMapEntry * entry;
    if(map->length == 0 || (entry = find_entry(map->entries, map->capacity, key))->key == NULL) 
        return false;

    // marked as deleted c:
    entry->key   = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void map_destroy(HashMap * map) {
    mem_dealloc(map->entries);
    map->capacity = 0;
    map->length   = 0;
    map->entries  = NULL;
}

uint32_t str_hash(const char* str, size_t length) {
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < length; i++) {
    hash ^= (uint8_t)str[i];
    hash *= 16777619;
  }
  return hash;
}
