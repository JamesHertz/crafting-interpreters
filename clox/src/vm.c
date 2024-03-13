#include "vm.h"
#include "program.h"
#include "memory.h"
#include <assert.h>

interpret_result_t interpret(const char * source){
    // compile(source);

    return INTERPRET_OK;
}

typedef struct {
    program_t * program;    
    uint8_t *ip;
    struct {
        value_t * values;
        size_t length;
        size_t size;
    } stack;
} vm_t;

static void vm_init(vm_t * vm, program_t * prog){
    vm->program = prog;
    vm->ip = prog->code.values;
    da_init(&vm->stack, value_t);
}

static void vm_push(vm_t * vm, value_t value){
    da_add(&vm->stack, value, value_t);
}

static value_t vm_pop(vm_t * vm){
    assert(vm->stack.length > 0);
    return vm->stack.values[ --vm->stack.length ];
}


static inline value_t get_data(program_t * prog, uint8_t idx){
    return prog->data.values[ idx ];
}


#define BINARY(op) vm_push( vm, vm_pop(vm) op vm_pop(vm) )
static size_t execute(vm_t * vm, op_code_t instr){
    switch (instr) {
        case OP_CONST:
            vm_push(vm, get_data(vm->program, *vm->ip));
            return 1;

        case OP_NEG : vm_push(vm, -vm_pop(vm)); break;
        case OP_ADD : BINARY(+); break;
        case OP_SUB : BINARY(-); break;
        case OP_MULT: BINARY(*); break;
        case OP_DIV : BINARY(+); break;
        default: break;
    }
    return 0;
}
#undef BINARY

void run(program_t * prog){
    vm_t vm;
    vm_init(&vm, prog);
    
    for(;;){
        op_code_t instr = *vm.ip++;
        switch (instr) {
            case OP_RETURN:
                return;
            default:
                vm.ip += execute(&vm, instr);
        }
    }

}
