#ifndef __LOX_NATIVE_FN__
#define __LOX_NATIVE_FN__

#include "value.h"
#include "vm.h"

void lox_clock(LoxVM * vm);

static inline void load_native_funcs(LoxVM * vm) {
    struct {
        const char * name;
        Fn executor;
        uint8_t arity;
    } natives[] = {
        { .name = "clock", .executor = lox_clock, .arity = 0 }
    };

    size_t defined_fns = sizeof(natives) / sizeof(natives[0]);
    for(size_t i = 0; i < defined_fns; i++) {
        vm_define_native_fn(vm, natives[i].name, natives[i].executor, natives[i].arity);
    }
}

#endif
