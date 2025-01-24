#ifndef CLOX_DARRAY_H
#define CLOX_DARRAY_H

#include <sys/types.h>

#define DaArray(type) struct { \
        type * values;         \
        size_t length;         \
        size_t size;           \
    }

typedef DaArray(void) DaArrayAny;

void da_push(DaArrayAny * array, void * elem, size_t elem_size);
void * da_pop(DaArrayAny * array, size_t elem_size);
void * da_get_ptr(const DaArrayAny * array, size_t idx, size_t elem_size);
void da_init(DaArrayAny * array, size_t elem_size);
void da_destroy(void * array);

#ifndef CLOX_ARRAY_NO_MACROS

#define da_init(type, array)       da_init((DaArrayAny *) (array), sizeof(type))
#define da_push(type, array, elem) da_push((DaArrayAny *)  (array), (void *) (elem), sizeof(type))
#define da_pop(type, array)        (*(type *) da_pop((DaArrayAny *) (array), sizeof(type)))
#define da_get(type, array, idx)   (*(type *) da_get_ptr((const DaArrayAny *) (array), (idx), sizeof(type)))
#define da_set(type, array, idx, value)  do { \
        *(type **) da_get_ptr((DaArrayAny *) (array), (idx), sizeof(type))) = value; \
 } while(0)

#endif

#endif
