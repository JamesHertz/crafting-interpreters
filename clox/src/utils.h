#ifndef CLOX_COMMON_H
#define CLOX_COMMON_H

#include <stdio.h>
#include <stdlib.h>

// TODO: create a set of functions that these macros call c:

#define UNREACHABLE() unreachable(CURRENT_LOCATION())
#define TODO(msg)     todo(CURRENT_LOCATION(), msg)
#define ASSERTF(expr, msg, ...) assert(CURRENT_LOCATION(), expr, #expr, msg __VA_OPT__(,) __VA_ARGS__)
#define ASSERT(expr) assert(CURRENT_LOCATION(), expr, #expr, 0)

#define CURRENT_LOCATION() ((struct __location__) { .file = __FILE__, .function = __func__, .line = __LINE__ } )

struct __location__ {
    const char * file;
    const char * function;
    int line;
};


void assert(struct __location__ loc, int condition, char * expr, char * message, ...) __attribute__((format (printf, 4, 5)));
void unreachable(struct __location__ loc) __attribute__((noreturn));
void todo(struct __location__ loc, char * message) __attribute__((noreturn));

#endif
