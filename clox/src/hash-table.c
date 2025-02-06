#include <stdio.h>
#include <stdlib.h>

#include "hash-table.h"
#include "memory.h"

#define TABLE_MAX_LOAD 0.75
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

static HashMapEntry * find_entry(HashMapEntry * entries, size_t capacity, const LoxString * key) {
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
    if(map->length + 1 > map->capacity * TABLE_MAX_LOAD ) {
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

const LoxValue * map_get(const HashMap * map, const LoxString * key) {
    HashMapEntry * entry = find_entry(map->entries, map->capacity, key);
    return entry->key == NULL ? NULL : &entry->value;
}

bool map_delete(HashMap * map, const LoxString * key) {
    if(map->length == 0) return false;

    HashMapEntry * entry = find_entry(map->entries, map->capacity, key);
    if(entry->key == NULL) 
        return false;

    // marked as deleted c:
    entry->key   = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void map_destroy(HashMap * map) {
    free(map->entries);
    map->capacity = 0;
    map->length   = 0;
    map->entries  = NULL;
}
