#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include <stdio.h>

void * mem_realloc(void * old, size_t new_size);

static inline void * mem_alloc(size_t size) {
    return mem_realloc(NULL, size);
}

static inline void * mem_dealloc(void * ptr) {
    return mem_realloc(ptr, 0);
}
#endif 
