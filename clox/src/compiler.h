#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include <stdbool.h>
#include "chunk.h"
#include "function.h"
#include "hash-map.h"

LoxFunction * compile(const char * source, HashMap * strings);

#endif 
