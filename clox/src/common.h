#ifndef CLOX_COMMON_H
#define CLOX_COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define UNREACHABLE() do {                         \
  fprintf(                                         \
    stderr,                                        \
    "%s:%s():%d: reached unreachable statement\n", \
    __FILE__, __func__, __LINE__                   \
  );                                               \
  exit(1);                                         \
} while(0)                                         \

#define TODO(msg) do {                   \
  fprintf(                               \
    stderr,                              \
    "TODO '" msg "' at `%s:%s():%d`\n",  \
    __FILE__, __func__, __LINE__         \
  );                                     \
  exit(1);                               \
} while(0)                               \

#define ASSERTF(expr, msg, ...) do {                 \
  if(!(expr)) {                                      \
      fprintf(                                       \
        stderr,                                      \
        "%s:%s():%d: assertion `" #expr "` failed\n" \
        "with message: " msg "\n",                   \
        __FILE__, __func__, __LINE__                 \
        __VA_OPT__(,) __VA_ARGS__                    \
      );                                             \
      exit(1);                                       \
   }                                                 \
} while(0)

#define ASSERT(expr) ASSERTF(expr, "-")

#endif
