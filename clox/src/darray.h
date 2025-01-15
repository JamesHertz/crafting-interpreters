#ifndef CLOX_DARRAY_H
#define CLOX_DARRAY_H

#include <sys/types.h>

typedef struct {
    void * values;
    size_t length;
    size_t size;
} da_array_t;

void da_push(da_array_t * array, void * elem, size_t elem_size);
void * da_pop(da_array_t * array, size_t elem_size);
void * da_get_ptr(const da_array_t * array, size_t idx, size_t elem_size);
void da_init(da_array_t * array, size_t elem_size);
void da_destroy(void * array);




#ifndef CLOX_ARRAY_NO_MACROS

#define DA_DECLARE_ARRAY(type) struct { \
        type * values;                  \
        size_t length;                  \
        size_t size;                    \
    }

#define da_init(type, array)       da_init((da_array_t *) (array), sizeof(type))
#define da_push(type, array, elem) da_push((da_array_t *)  (array), (void *) (elem), sizeof(type))
#define da_pop(type, array)        (*(type *) da_pop((da_array_t *) (array), sizeof(type)))
#define da_get(type, array, idx)   (*(type *) da_get_ptr((const da_array_t *) (array), (idx), sizeof(type)))
#define da_set(type, array, idx, value)  do { \
        *((type **) da_get_ptr((da_array_t *) (array), (idx), sizeof(type))) = value; \
 } while(0)

#endif

#endif
