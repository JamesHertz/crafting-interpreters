#ifndef CLOX_PROGRAM_H
#define CLOX_PROGRAM_H

#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>

#include "darray.h"
#include "value.h"

typedef enum {
    OP_CONST,
    OP_RETURN,
    OP_POP,

    // arithmetic horsemans
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,

    // comparison
    OP_EQ,
    OP_LESS,
    OP_GREATER,
    
    // boolean
    OP_NOT,

    // LoxValue values
    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    // print
    OP_PRINT,

    // variables
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,

    OP_SET_LOCAL,
    OP_GET_LOCAL,

    // control flow
    OP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,

    OP_CALL,
} OpCode;

// WARN: please don't alter the order <values>, <length>, <size> of the inner structs, this is crucial!
void chunk_init(LoxChunk * c);
size_t chunk_add_constant(LoxChunk * c, LoxValue value);
LoxValue chunk_get_constant(const LoxChunk * c, size_t idx);
void chunk_add_instr(LoxChunk * c, uint8_t value, uint32_t line);
void chunk_destroy(LoxChunk * c);

void chunk_debug(const LoxChunk * c, const char * title);
size_t chunk_instr_debug(const LoxChunk * c, size_t offset);

#endif
