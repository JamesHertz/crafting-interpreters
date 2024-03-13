#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    void * values;
    size_t length;
    size_t size;
} da_array;

// the layout of the struct should be the same
// as the one above c:
void _da_init(da_array * arr, size_t elem_size);
void _da_resize(da_array * arr, size_t elem_size);

// set of helpul macros c:
#define da_init(arr, type) _da_init((da_array *) (arr), sizeof(type))
#define da_resize(arr, elem_size) _da_resize((da_array *) (arr), elem_size)

#define da_add(arr, value, type) do{                \
          if( (arr)->size == (arr)->length )        \
              da_resize(arr, sizeof(type));         \
          (arr)->values[ (arr)->length++ ] = value; \
        }while(0)


// #define da_add(arr, value) do{                      \
//           if( (arr).size == (arr).length )          \
//               da_resize(arr, sizeof(typeof(value)); \
//           (arr).values[ (arr).length++ ] = value;   \
//         }while(0)
//

#endif 





