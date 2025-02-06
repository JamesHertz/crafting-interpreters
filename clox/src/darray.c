#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define CLOX_DARRAY_NO_MACROS

#include "darray.h"
#include "memory.h"

#define DEFAULT_SIZE 8
#define GROWTH_FACTOR 2

static void * da_get_ptr(const DaArrayAny * array, size_t idx) {
    if(idx >= array->length){
        fprintf(stderr, "da_get_ptr(): index %zu out of range 0..%zu\n", idx, array->length);
        exit(1);
    }
    return array->values + idx * array->elem_size;
}

void da_init(DaArrayAny * array, size_t elem_size) {
    array->values = mem_alloc(elem_size * DEFAULT_SIZE);
    array->size   = DEFAULT_SIZE;
    array->elem_size = elem_size;
    array->length = 0;
}

void * da_get(const DaArrayAny * array, size_t idx) {
    return da_get_ptr(array, idx);
}

void da_set(DaArrayAny * array, size_t idx, const void * value) {
    void * ptr = da_get_ptr(array, idx);
    memcpy(ptr, value, array->elem_size);
}

void da_push(DaArrayAny * array, const void * elem) {
    if(array->length == array->size) { // array is full
        array->size *= GROWTH_FACTOR;
        array->values = mem_realloc(array->values, array->size * array->elem_size);
    }

    void * ptr = da_get_ptr(array, array->length++);
    memcpy(ptr, elem, array->elem_size);
}

void * da_pop(DaArrayAny * array) {
    assert(array->length > 0 && "called `da_pop()` from an empty array");
    void * ptr = da_get_ptr(array, array->length - 1);
    array->length--;
    return ptr;
}

void da_destroy(DaArrayAny * array){
    mem_dealloc(array->values);
    array->size   = 0;
    array->length = 0;
}
