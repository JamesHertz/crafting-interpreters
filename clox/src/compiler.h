#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include <stdbool.h>
#include "program.h"

bool compile(const char * source, program_t * p);

#endif 
