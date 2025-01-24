#ifndef CLOX_COMMON_H
#define CLOX_COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define UNREACHABLE() do {                         \
  fprintf(                                         \
    stderr,                                        \
    "%s:%d:%s(): reached unreachable statement\n", \
    __FILE__, __LINE__, __func__                   \
  );                                               \
  exit(1);                                         \
} while(0)                                         \

#define TODO(msg) do {                   \
  fprintf(                               \
    stderr,                              \
    "TODO '" msg "' at `%s:%d:%s()`\n",  \
    __FILE__, __LINE__, __func__         \
  );                                     \
  exit(1);                               \
} while(0)                               \

#endif
