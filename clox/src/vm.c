#include "vm.h"
#include "compiler.h"
#include "program.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct {
    LoxProgram program;    
    instr_t * ip;
    DaArray(LoxValue) stack;
    LoxObject * objects;
} vm_t;

static void vm_init(vm_t * vm){
    prog_init(&vm->program);
    da_init(LoxValue, &vm->stack);
    vm->ip = NULL;
    vm->objects = NULL;
} 

static void vm_free_objects(vm_t * vm){
    for(LoxObject * curr = vm->objects; curr;) {
        LoxObject * next = curr->next;
        obj_destroy(curr);
        curr = next;
    }
    vm->objects = NULL;
}

static void vm_destroy(vm_t * vm){
    vm_free_objects(vm);
    prog_destroy(&vm->program);
    da_destroy(&vm->stack);
    vm->ip      = NULL;
}

static void vm_push(vm_t * vm, LoxValue value){
    da_push(LoxValue, &vm->stack, &value);
}

static LoxValue vm_pop(vm_t * vm){
    assert(vm->stack.length > 0 && "vm_pop(): poping from empty stack");
    return da_pop(LoxValue, &vm->stack);
}

static LoxValue vm_peek(vm_t * vm, size_t distance){
    size_t idx = vm->stack.length - (1 + distance);
    return da_get(LoxValue, &vm->stack, idx);
}

static inline LoxValue vm_get_constant(vm_t * vm, uint8_t idx){
    return prog_get_constant(&vm->program, idx);
}

static void vm_register_object(vm_t * vm, LoxObject * obj){
    assert(obj->next == NULL && "vm_register_object(): obj->next field not null");
    obj->next   = vm->objects;
    vm->objects = obj;
}

static void vm_report_runtime_error(vm_t * vm, const char * format, ...) {
    // FIXME: use va_args and printf
    fputs("run-time-error: ", stdout);
    puts(format);
    instr_t instr = vm->ip[-1];
    printf("[line %u] in script\n", instr.line);
}


static LoxString * vm_stingify_value(vm_t * vm, LoxValue value){
    if(VAL_IS_STRING(value)) return VAL_AS_STRING(value);

    char buffer[256];
    if(VAL_IS_NUMBER(value))
        sprintf(buffer, "%g", value.as.number);
    else if(VAL_IS_BOOL(value)) 
        strcpy(buffer, value.as.boolean ? "true" : "false");
    else if(VAL_IS_NIL(value))
        strcpy(buffer, "nil");
    else {
        assert(0 && "vm_stingify_value(): reached unreachable branch");
    }

    LoxString * str = lox_str_copy(buffer, strlen(buffer));
    vm_register_object(vm, (LoxObject*) str);
    return str;
}


static LoxInterpretResult vm_run(vm_t * vm){
#define BINARY(op, value_constructor) do {                                     \
        if(!VAL_IS_NUMBER(vm_peek(vm, 0)) || !VAL_IS_NUMBER(vm_peek(vm, 1))) { \
            vm_report_runtime_error(vm, "operands should both be numbers");    \
            return INTERPRET_RUNTIME_ERROR;                                    \
        }                                                                      \
        double b = vm_pop(vm).as.number;                                       \
        double a = vm_pop(vm).as.number;                                       \
        vm_push(vm, value_constructor(a op b));                                \
    } while(0)

#define READ_BYTE() (*vm->ip++).op_code
    
    vm->ip = vm->program.code.values;
    for(;;){

#ifdef DEBUG_TRACE_EXECUTION
        fputs("          ", stdout);
        LoxValue * top = vm->stack.values + vm->stack.length;
        for(LoxValue * curr = vm->stack.values; curr < top; curr++){
            bool is_str = VAL_IS_STRING(*curr);
            fputs(is_str ? "[ \"" : "[ ", stdout);
            value_print(*curr);
            fputs(is_str ? "\" ]" : " ]", stdout);
        }
        putchar('\n');
        prog_instr_debug(&vm->program, (size_t) (vm->ip - vm->program.code.values));
#endif

        op_code_t instr = READ_BYTE();
        switch (instr) {
            case OP_CONST: {
                uint8_t data_idx = READ_BYTE();
                LoxValue data = prog_get_constant(&vm->program, data_idx);
                vm_push(vm, data);
                break;
            } 

            case OP_NOT: 
                if(!VAL_IS_BOOL(vm_peek(vm, 0))) {
                    vm_report_runtime_error(vm, "expected a boolean operand");
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm_push(vm, BOOL_VAL(!vm_pop(vm).as.boolean)); 
                break;

            case OP_NEG : 
                if(!VAL_IS_NUMBER(vm_peek(vm, 0))) {
                    vm_report_runtime_error(vm, "expected a number operand");
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm_push(vm, NUMBER_VAL(-vm_pop(vm).as.number)); 
                break;

            case OP_ADD : {
                LoxValue b = vm_peek(vm, 0);
                LoxValue a = vm_peek(vm, 1);

                if(VAL_IS_STRING(a) || VAL_IS_STRING(b)) {
                    LoxString * str2 = vm_stingify_value(vm, vm_pop(vm));
                    LoxString * str1 = vm_stingify_value(vm,vm_pop(vm));
                    LoxString * result = lox_str_concat(str1, str2); 

                    vm_register_object(vm, (LoxObject*) result);
                    vm_push(vm, OBJ_VAL(result));
                } else if(VAL_IS_NUMBER(a) && VAL_IS_NUMBER(b)) {
                    vm_push(vm, NUMBER_VAL( vm_pop(vm).as.number + vm_pop(vm).as.number));
                } else {
                    vm_report_runtime_error(vm, "operator '+' expects either two integers or at least 1 string");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
                break;
            case OP_SUB : BINARY(-, NUMBER_VAL); break;
            case OP_MULT: BINARY(*, NUMBER_VAL); break;
            case OP_DIV : BINARY(/, NUMBER_VAL); break;

            case OP_LESS    : BINARY(<, BOOL_VAL); break;
            case OP_GREATER : BINARY(>, BOOL_VAL); break;
            case OP_EQ      : 
                vm_push(vm, BOOL_VAL(value_eq(vm_pop(vm), vm_pop(vm))));
                break;

            case OP_TRUE  :  vm_push(vm, BOOL_VAL(true)); break;
            case OP_FALSE :  vm_push(vm, BOOL_VAL(false)); break;
            case OP_NIL   :  vm_push(vm, NIL_VAL); break;

            case OP_RETURN:
                 if(vm->stack.length > 0) {
                     value_print(vm_pop(vm)); putchar('\n');
                 }
                return INTERPRET_OK;
            default:
                assert(0 && "vm_run(): invalid opcode");
        }
    }

#undef BINARY
#undef READ_BYTE
}

LoxInterpretResult interpret(const char * source){
    vm_t vm;
    vm_init(&vm);

    LoxInterpretResult res;
    if(!compile(source, &vm.program)) {
       res = INTERPRET_COMPILE_ERROR;
    } else {
        vm_run(&vm);
        res = INTERPRET_OK;
    }

    vm_destroy(&vm);
    return res;
}
