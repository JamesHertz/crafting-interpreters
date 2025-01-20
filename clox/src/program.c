#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "program.h"
#include "darray.h"
#include "memory.h"

void prog_init(program_t * p){
    da_init(instr_t, &p->code);
    da_init(value_t, &p->constants);
}

static void free_objects(program_t * p) {
    for(size_t i = 0; i < p->constants.length; p++) {
        value_t val = p->constants.values[i];
        if(VAL_IS_STRING(val)) {
            obj_string_t * str = (obj_string_t *) val.as.object;
            mem_dealloc((void *) str->chars);

            str->chars    = NULL;
            str->length   = 0;
            str->obj.type = 0;
            assert(str->obj.next == NULL);

            mem_dealloc(str);
        }
    }
}

void prog_destroy(program_t * p){
    free_objects(p);
    da_destroy(&p->code);
    da_destroy(&p->constants);
}

value_t prog_get_constant(const program_t * p, size_t idx){
    return da_get(value_t, &p->constants, idx);
}

size_t prog_add_constant(program_t * p, value_t value){
    da_push(value_t, &p->constants, &value);
    return p->constants.length - 1;
}

void prog_add_instr(program_t * p, uint8_t op_code, uint32_t line){
    instr_t tmp = {
        .op_code = op_code,
        .line    = line
    };
    da_push(instr_t, &p->code, &tmp);
}

static size_t print_constant_instr(const char * name, const program_t * p, size_t offset){
    instr_t constant = da_get(instr_t, &p->code, offset + 1);
    printf("%-16s %4d ", name, constant.op_code);
    value_print(da_get(value_t, &p->constants, constant.op_code));
    putchar('\n');
    return offset + 2;
}

size_t prog_instr_debug(const program_t * p, size_t offset){
#define SIMPLE_INSTR_CASE(opcode) case opcode: puts(#opcode); break
#define CONST_INSTR_CASE(opcode)  case opcode: return print_constant_instr(#opcode, p, offset)
    instr_t instr = da_get(instr_t, &p->code, offset);

    printf("%04zu ", offset);
    if(offset > 0 && da_get(instr_t, &p->code, offset - 1).line == instr.line) {
        fputs("   | ", stdout);
    } else {
        printf("%4d ", instr.line);
    }

    switch(instr.op_code) {
        CONST_INSTR_CASE(OP_CONST);

        /*SIMPLE_INSTR_CASE(OP_PRINT);*/
        SIMPLE_INSTR_CASE(OP_ADD);
        SIMPLE_INSTR_CASE(OP_SUB);
        SIMPLE_INSTR_CASE(OP_MULT);
        SIMPLE_INSTR_CASE(OP_DIV);
        SIMPLE_INSTR_CASE(OP_RETURN);
        SIMPLE_INSTR_CASE(OP_NEG);


        SIMPLE_INSTR_CASE(OP_EQ);
        SIMPLE_INSTR_CASE(OP_LESS);
        SIMPLE_INSTR_CASE(OP_GREATER);
        
        // boolean
        SIMPLE_INSTR_CASE(OP_NOT);

        // value_t values
        SIMPLE_INSTR_CASE(OP_NIL);
        SIMPLE_INSTR_CASE(OP_TRUE);
        SIMPLE_INSTR_CASE(OP_FALSE);

        default:
            printf("Unknown opcode %d\n", instr.op_code);
    }
    
    return offset + 1;

#undef SIMPLE_INSTR_CASE
#undef CONST_INSTR_CASE
}

void prog_debug(const program_t * p, const char * title) {
    printf("=== %s ===\n", title);
    for(size_t offset = 0; offset < p->code.length;)
        offset = prog_instr_debug(p, offset);
}

