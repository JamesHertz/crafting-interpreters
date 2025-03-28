#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "darray.h"
#include "memory.h"
#include "utils.h"

void chunk_init(LoxChunk * p){
    da_init(&p->code);
    da_init(&p->constants);
}

static void free_objects(LoxChunk * p) {
    for(size_t i = 0; i < p->constants.length; i++) {
        LoxValue val = p->constants.values[i];
        if(VAL_IS_STRING(val)) {
            LoxString * str = (LoxString *) val.as.object;
            mem_dealloc((void *) str->chars);

            str->chars    = NULL;
            str->length   = 0;
            str->obj.type = 0;
            ASSERT(str->obj.next == NULL);

            mem_dealloc(str);
        }
    }
}

void chunk_destroy(LoxChunk * p){
    free_objects(p);
    da_destroy(&p->code);
    da_destroy(&p->constants);
}

LoxValue chunk_get_constant(const LoxChunk * p, size_t idx){
    return da_get(&p->constants, idx);
}

size_t chunk_add_constant(LoxChunk * p, LoxValue value){
    da_push(&p->constants, value);
    return p->constants.length - 1;
}

void chunk_add_instr(LoxChunk * p, uint8_t op_code, uint32_t line){
    Instruction tmp = {
        .op_code = op_code,
        .line    = line
    };
    da_push(&p->code, tmp);
}

static size_t print_byte_instr(const char * name, const LoxChunk * p, size_t offset){
    Instruction constant = da_get(&p->code, offset + 1);
    printf("%-16s %4d\n", name, constant.op_code);
    return offset + 2;
}

static size_t print_constant_instr(const char * name, const LoxChunk * p, size_t offset){
    Instruction constant = da_get(&p->code, offset + 1);
    printf("%-16s %4d ", name, constant.op_code);

    LoxValue value = da_get(&p->constants, constant.op_code);
    if(VAL_IS_STRING(value)) {
        putchar('"');
        value_print(value);
        putchar('"');
    } else 
        value_print(value);

    putchar('\n');
    return offset + 2;
}

static size_t print_jump_instr(const char * name, const LoxChunk * p, size_t offset, int sign) {
    size_t jump_length = (size_t) da_get(&p->code, offset + 2).op_code << 8 | da_get(&p->code, offset + 1).op_code;
    size_t jump_target = offset + 3 + jump_length * sign;
    printf("%-16s %4zu (%04zu)", name, jump_length, jump_target);
    putchar('\n');
    return offset + 3;
}

size_t chunk_instr_debug(const LoxChunk * p, size_t offset){
#define SIMPLE_INSTR_CASE(opcode)       case opcode: puts(#opcode); break
#define CONST_INSTR_CASE(opcode)        case opcode: return print_constant_instr(#opcode, p, offset)
#define BYTE_INSTR_CASE(opcode)         case opcode: return print_byte_instr(#opcode, p, offset)
#define JUMP_INSTR_CASE(opcode, sign)   case opcode: return print_jump_instr(#opcode, p, offset, sign)
    Instruction instr = da_get(&p->code, offset);

    printf("%04zu ", offset);
    if(offset > 0 && da_get(&p->code, offset - 1).line == instr.line) {
        fputs("   | ", stdout);
    } else {
        printf("%4d ", instr.line);
    }

    switch(instr.op_code) {
        CONST_INSTR_CASE(OP_CONST);
        CONST_INSTR_CASE(OP_SET_GLOBAL);

        CONST_INSTR_CASE(OP_GET_GLOBAL);
        CONST_INSTR_CASE(OP_DEFINE_GLOBAL);

        BYTE_INSTR_CASE(OP_SET_LOCAL);
        BYTE_INSTR_CASE(OP_GET_LOCAL);
        BYTE_INSTR_CASE(OP_CALL);

        SIMPLE_INSTR_CASE(OP_POP);
        SIMPLE_INSTR_CASE(OP_PRINT);
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

        JUMP_INSTR_CASE(OP_JUMP, 1);
        JUMP_INSTR_CASE(OP_IF_FALSE, 1);
        JUMP_INSTR_CASE(OP_LOOP, -1);

        default: UNREACHABLE();
    }
    
    return offset + 1;

#undef SIMPLE_INSTR_CASE
#undef CONST_INSTR_CASE
#undef BYTE_INSTR_CASE
#undef JUMP_INSTR_CASE
}

void chunk_debug(const LoxChunk * p, const char * title) {
    printf("=== %s ===\n", title);
    for(size_t offset = 0; offset < p->code.length;)
        offset = chunk_instr_debug(p, offset);
}

