#include "program.h"
#include "memory.h"

void init_prog(program_t * p){
    da_init(&p->code, uint8_t);
    da_init(&p->data, value_t);
}

size_t prog_add_value(program_t * p, value_t value){
    da_add(&p->data, value, value_t);
    return p->data.length - 1;
}

void prog_add_instr(program_t * p, uint8_t value){
    da_add(&p->code, value, uint8_t);
}
