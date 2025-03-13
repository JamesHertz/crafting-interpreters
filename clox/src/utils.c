#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "utils.h"

#define LOC_FMT "%s:%d %s()"
#define LOC_PARAMS(loc) loc.file, loc.line, loc.function

void todo(struct __location__ loc, char * message) {
    fprintf(stderr, "TODO: '%s' at `" LOC_FMT "`\n", message, LOC_PARAMS(loc));
    exit(EXIT_FAILURE);
}

void unreachable(struct __location__ loc) {
    fprintf(stderr, LOC_FMT ": reached unreachable statement\n",  LOC_PARAMS(loc));
    exit(EXIT_FAILURE);
}

void assert(struct __location__ loc, int condition, char * expr, char * message, ...) {
    if(condition) return;

    fprintf(stderr, LOC_FMT ": assertion `%s` failed\n", LOC_PARAMS(loc), expr);
    if(message != NULL) {
        fprintf(stderr, "with message: ");
        va_list list;
        va_start(list, message);
        vfprintf(stderr, message, list);
        va_end(list);
        fputc('\n', stderr);
    }

    exit(EXIT_FAILURE);
}
