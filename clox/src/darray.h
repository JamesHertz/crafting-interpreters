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
void * da_get_elem(const DaArrayAny * array, size_t idx);
void da_set(DaArrayAny * array, size_t idx, const void * value);
void da_destroy(DaArrayAny * array);

#ifndef CLOX_DARRAY_NO_MACROS

#define da_basic_type(array)    typeof((array)->values[0])
#define deref_elem(array, ptr) (*((da_basic_type(array) *) (ptr)))

#define da_init(array)             da_init((DaArrayAny *) (array), sizeof((array)->values[0]))
#define da_pop(array)              deref_elem(array, da_pop((DaArrayAny *) array))
#define da_get_ptr(array, idx)     ((da_basic_type(array) *) da_get_elem((const DaArrayAny *) array, idx))
#define da_get(array, idx)         deref_elem(array, da_get_elem((const DaArrayAny *) array, idx))


#define da_destroy(array)          da_destroy((DaArrayAny *) array);

#define da_push(array, elem)       do {                                           \
                                        da_basic_type(array) __e__ = elem;        \
                                        da_push((DaArrayAny *) array, &__e__);    \
                                    } while(0)
#define da_set(array, idx, value)  do {                                           \
                                        da_basic_type(array) __e__ = value;       \
                                        da_set((DaArrayAny *) array, idx, &__e__); \
                                   } while(0)

#define DA_FOR_EACH_ELEM(elem, array, code)         \
  for(size_t i = 0; i < (array)->length; i++) {     \
    da_basic_type(array) elem = (array)->values[i]; \
    code                                            \
 }

#endif

#endif
