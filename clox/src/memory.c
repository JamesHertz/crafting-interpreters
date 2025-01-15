#include "memory.h"

#include <string.h>
#include <errno.h>
#include <assert.h>

void * mem_realloc(void * old, size_t new_size) {
    /*assert(new_size > 0 && "mem_realloc(): allocating with size of 0");*/
    void * ptr = realloc(old, new_size);
    if(new_size != 0 && ptr == NULL){
        fprintf(stderr, "Failed to %sallocate %zu bytes: %s\n", old == NULL ? "" : "re", new_size, strerror(errno));
        exit(1);
    }
    return ptr;
}
