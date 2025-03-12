#include "memory.h"

#include <string.h>
#include <errno.h>

void * mem_realloc(void * old, size_t new_size) {
    void * ptr = realloc(old, new_size);
    if(new_size != 0 && ptr == NULL){
        fprintf(stderr, "Failed to %sallocate %zu bytes: %s\n", old == NULL ? "" : "re", new_size, strerror(errno));
        exit(1);
    }
    return ptr;
}
