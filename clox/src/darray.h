#ifndef CLOX_DARRAY_H
#define CLOX_DARRAY_H

#include <sys/types.h>

#define DaArray(type) struct { \
        type * values;         \
        size_t length;         \
        size_t size;           \
        size_t elem_size;      \
    }

typedef DaArray(void) DaArrayAny;

void da_init(DaArrayAny * array, size_t elem_size);
void da_push(DaArrayAny * array, const void * elem);
void * da_pop(DaArrayAny * array);
void * da_get(const DaArrayAny * array, size_t idx);
void da_set(DaArrayAny * array, size_t idx, const void * value);
void da_destroy(DaArrayAny * array);

#ifndef CLOX_DARRAY_NO_MACROS

#define da_init(array)             da_init((DaArrayAny *) (array), sizeof((array)->values[0]))
#define da_push(array, elem)       da_push((DaArrayAny *) array, elem)
#define da_pop(array)              da_pop((DaArrayAny *) array)
#define da_get(array, idx)         da_get((const DaArrayAny *) array, idx)
#define da_set(array, idx, value)  da_set((DaArrayAny *) array, idx, value)
#define da_destroy(array)          da_destroy((DaArrayAny *) array);

#define deref_as(type, ptr) (*((type *) (ptr)))

#define DA_FOR_EACH_ELEM(type, elem, array, code) \
  for(size_t i = 0; i < (array)->length; i++) {   \
    type elem = (array)->values[i];               \
    code                                          \
 }

#endif

#endif
