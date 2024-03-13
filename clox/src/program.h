#ifndef CLOX_PROGRAM_H
#define CLOX_PROGRAM_H

#include <stdint.h>
#include <stdio.h>


// by now c:
typedef double value_t;

typedef enum {
    OP_CONST,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    OP_RETURN,
    OP_NEG
} op_code_t;


typedef struct {
    struct {
        uint8_t * values;
        size_t length;
        size_t size;
    } code;

    struct {
        value_t * values;
        size_t length;
        size_t size;
    } data;

} program_t;


void init_prog(program_t * p);
size_t prog_add_value(program_t * p, value_t value);
void prog_add_instr(program_t * p, uint8_t value);

#endif
