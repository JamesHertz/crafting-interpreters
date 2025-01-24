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
    OP_FALSE
} op_code_t;

// WARN: please don't alter the order <values>, <length>, <size> of the inner structs, this is crucial!
typedef struct {
    uint32_t line;
    uint8_t op_code;
} instr_t;

typedef struct {
    DaArray(instr_t) code;
    DaArray(LoxValue) constants;
} LoxProgram;

void prog_init(LoxProgram * p);
size_t prog_add_constant(LoxProgram * p, LoxValue value);
LoxValue prog_get_constant(const LoxProgram * p, size_t idx);
void prog_add_instr(LoxProgram * p, uint8_t value, uint32_t line);
void prog_destroy(LoxProgram * p);

void prog_debug(const LoxProgram * p, const char * title);
size_t prog_instr_debug(const LoxProgram * p, size_t offset);

#endif
