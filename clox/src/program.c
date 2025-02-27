#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "program.h"
#include "darray.h"
#include "memory.h"

void prog_init(LoxProgram * p){
    da_init(&p->code);
    da_init(&p->constants);
}

static void free_objects(LoxProgram * p) {
    for(size_t i = 0; i < p->constants.length; i++) {
        LoxValue val = p->constants.values[i];
        if(VAL_IS_STRING(val)) {
            LoxString * str = (LoxString *) val.as.object;
            mem_dealloc((void *) str->chars);

            str->chars    = NULL;
            str->length   = 0;
            str->obj.type = 0;
            assert(str->obj.next == NULL);

            mem_dealloc(str);
        }
    }
}

void prog_destroy(LoxProgram * p){
    free_objects(p);
    da_destroy(&p->code);
    da_destroy(&p->constants);
}

LoxValue prog_get_constant(const LoxProgram * p, size_t idx){
    return deref_as(LoxValue, da_get(&p->constants, idx));
}

size_t prog_add_constant(LoxProgram * p, LoxValue value){
    da_push(&p->constants, &value);
    return p->constants.length - 1;
}

void prog_add_instr(LoxProgram * p, uint8_t op_code, uint32_t line){
    Instruction tmp = {
        .op_code = op_code,
        .line    = line
    };
    da_push(&p->code, &tmp);
}

static size_t print_constant_instr(const char * name, const LoxProgram * p, size_t offset){
    Instruction constant = deref_as(Instruction, da_get(&p->code, offset + 1));
    printf("%-16s %4d ", name, constant.op_code);
    value_print(deref_as(LoxValue, da_get(&p->constants, constant.op_code)));
    putchar('\n');
    return offset + 2;
}

size_t prog_instr_debug(const LoxProgram * p, size_t offset){
#define SIMPLE_INSTR_CASE(opcode) case opcode: puts(#opcode); break
#define CONST_INSTR_CASE(opcode)  case opcode: return print_constant_instr(#opcode, p, offset)
    Instruction instr = deref_as(Instruction, da_get( &p->code, offset));

    printf("%04zu ", offset);
    if(offset > 0 && deref_as(Instruction, da_get(&p->code, offset - 1)).line == instr.line) {
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

        // LoxValue values
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

void prog_debug(const LoxProgram * p, const char * title) {
    printf("=== %s ===\n", title);
    for(size_t offset = 0; offset < p->code.length;)
        offset = prog_instr_debug(p, offset);
}

