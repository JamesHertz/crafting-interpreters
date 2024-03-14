#include "vm.h"
#include "program.h"
#include "memory.h"
#include <assert.h>

typedef struct {
    program_t program;    
    uint8_t * ip;
    struct {
        value_t * values;
        size_t length;
        size_t size;
    } stack;
} vm_t;

static void vm_init(vm_t * vm){
    prog_init(&vm->program);
    da_init(&vm->stack, value_t);
    vm->ip = vm->program.code.values;
}

static void vm_push(vm_t * vm, value_t value){
    da_add(&vm->stack, value, value_t);
}

static value_t vm_pop(vm_t * vm){
    assert(vm->stack.length > 0);
    return vm->stack.values[ --vm->stack.length ];
}


static inline value_t vm_prog_data(vm_t * vm, uint8_t idx){
    return vm->program.data.values[ idx ];
}


static void run(vm_t * vm){
#define BINARY(op) vm_push(vm, vm_pop(vm) op vm_pop(vm))
#define READ_BYTE() *vm->ip++
    
    for(;;){
        op_code_t instr = READ_BYTE();
        switch (instr) {
            case OP_CONST: {
                uint8_t data_idx = READ_BYTE();
                value_t data = prog_data(&vm->program, data_idx);
                vm_push(vm, data);
                break;
            } 
            case OP_NEG : vm_push(vm, -vm_pop(vm)); break;
            case OP_ADD : BINARY(+); break;
            case OP_SUB : BINARY(-); break;
            case OP_MULT: BINARY(*); break;
            case OP_DIV : BINARY(+); break;
            case OP_RETURN:
                return;
        }
    }

#undef BINARY
#undef READ_BYTE
}

interpret_result_t interpret(const char * source){
    // compile(source);

    return INTERPRET_OK;
}
