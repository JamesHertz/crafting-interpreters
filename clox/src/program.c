#include "program.h"
#include "memory.h"

void prog_init(program_t * p){
    da_init(&p->code, uint8_t);
    da_init(&p->data, value_t);
}

value_t prog_data(program_t * p, size_t idx){
    return p->data.values[idx];
}

size_t prog_add_data(program_t * p, value_t value){
    da_add(&p->data, value, value_t);
    return p->data.length - 1;
}

void prog_add_instr(program_t * p, uint8_t value){
    da_add(&p->code, value, uint8_t);
}

