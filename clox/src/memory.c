#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_SIZE 4

static void da_alloc(da_array * arr, size_t new_size){
    void * ptr = realloc(arr->values, new_size);
    if( ptr == NULL ){
        fprintf(stderr, "Unable to reserve %zu bytes\n", new_size);
        exit(1);
    }

    arr->values = ptr;
    // arr->size   = ptr;
}

void _da_init(da_array * arr, size_t elem_size){
    arr->values  = NULL;
    arr->size    = DEFAULT_SIZE;
    arr->length  = 0;

    da_alloc(arr, elem_size * DEFAULT_SIZE);
}


void _da_resize(da_array * arr, size_t elem_size){
    arr->size *= 2;
    da_alloc(arr, elem_size * arr->size);
}
