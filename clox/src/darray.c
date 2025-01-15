#define CLOX_ARRAY_NO_MACROS

#include "darray.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_SIZE 4
#define GROWTH_FACTOR 2

void da_init(da_array_t * array, size_t elem_size) {
    array->values = mem_alloc(elem_size * DEFAULT_SIZE);
    array->size   = DEFAULT_SIZE;
    array->length = 0;
}

void * da_get_ptr(const da_array_t * array, size_t idx, size_t elem_size) {
    if(idx >= array->length){
        fprintf(stderr, "da_get_ptr(): index %zu out of range 0..%zu\n", idx, array->length);
        exit(1);
    }
    return array->values + idx * elem_size;
}

void da_push(da_array_t * array, void * elem, size_t elem_size) {
    if(array->length == array->size) { // array is full
        array->size *= GROWTH_FACTOR;
        array->values = mem_realloc(array->values, array->size * elem_size);
    }

    void * ptr = da_get_ptr(array, array->length++, elem_size);
    memcpy(ptr, elem, elem_size);
}

void * da_pop(da_array_t * array, size_t elem_size) {
    assert(array->length > 0 && "called `da_pop()` from an empty array");
    void * ptr = da_get_ptr(array, array->length - 1, elem_size);
    array->length--;
    return ptr;
}

void da_destroy(void * array){
    da_array_t * arr = array;
    mem_dealloc(arr->values);
    arr->size   = 0;
    arr->length = 0;
}
