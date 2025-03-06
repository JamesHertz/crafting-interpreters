#include "vm.h"
#include "compiler.h"
#include "program.h"
#include "hash-map.h"
#include "memory.h"
#include "debug.h"
#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/*#include <assert.h>*/

typedef struct {
    LoxProgram program;    
    DaArray(LoxValue) stack;
    HashMap strings;
    HashMap globals;
    Instruction * ip;
    LoxObject * objects;
} LoxVM;

static void vm_init(LoxVM * vm){
    prog_init(&vm->program);
    da_init(&vm->stack);
    map_init(&vm->strings);
    map_init(&vm->globals);
    vm->ip = NULL;
    vm->objects = NULL;
} 

static void vm_free_objects(LoxVM * vm){
    for(LoxObject * curr = vm->objects; curr;) {
        LoxObject * next = curr->next;
        lox_obj_destroy(curr);
        curr = next;
    }
    vm->objects = NULL;
}

static void vm_destroy(LoxVM * vm){
    vm_free_objects(vm);
    prog_destroy(&vm->program);
    map_destroy(&vm->strings);
    map_destroy(&vm->globals);
    da_destroy(&vm->stack);
    vm->ip = NULL;
}

static inline void vm_stack_push(LoxVM * vm, LoxValue value){
    da_push(&vm->stack, value);
}

static inline LoxValue vm_stack_get(LoxVM * vm, size_t idx){
    ASSERTF(idx < vm->stack.length, "accessing invalid stack position");
    return da_get(&vm->stack, idx);
}

static inline void vm_stack_set(LoxVM * vm, size_t idx, LoxValue value){
    da_set(&vm->stack, idx, value);
}

static inline LoxValue vm_stack_pop(LoxVM * vm){
    return da_pop(&vm->stack);
}

static inline  LoxValue vm_stack_peek(LoxVM * vm, size_t distance){
    size_t idx = vm->stack.length - (1 + distance);
    return da_get(&vm->stack, idx);
}

static inline LoxValue vm_get_constant(LoxVM * vm, uint8_t idx){
    return prog_get_constant(&vm->program, idx);
}

static void vm_register_object(LoxVM * vm, LoxObject * obj){
    ASSERTF(obj->next == NULL, "invalid object: obj->next field not null");
    obj->next   = vm->objects;
    vm->objects = obj;
}

static void vm_report_runtime_error(LoxVM * vm, const char * format, ...) {
    va_list list;
    va_start(list, format);
    fputs("[ RunTimeError ] : ", stderr);
    vfprintf(stderr, format, list);
    va_end(list);

    Instruction instr = vm->ip[-1];
    fprintf(stderr, "\n[line %u] in script\n", instr.line);
}


static const LoxString * vm_stingify_value(LoxVM * vm, LoxValue value){
    if(VAL_IS_STRING(value)) return VAL_AS_STRING(value);

    char buffer[256];
    if(VAL_IS_NUMBER(value))
        sprintf(buffer, "%g", value.as.number);
    else if(VAL_IS_BOOL(value)) 
        strcpy(buffer, value.as.boolean ? "true" : "false");
    else if(VAL_IS_NIL(value))
        strcpy(buffer, "nil");
    else {
        UNREACHABLE();
    }


    size_t length = strlen(buffer);
    uint32_t hash = str_hash(buffer, length);

    const LoxString * str;
    if((str = map_find_str(&vm->strings, buffer, length, hash)) == NULL) {
        str = lox_str_copy(buffer, length, hash);
        map_set(&vm->strings, str, BOOL_VAL(true));
        vm_register_object(vm, (LoxObject*) str);
    }
    return str;
}

static bool is_falsely(LoxValue v) {
    return v.type == VAL_NIL || (v.type == VAL_BOOL && !v.as.boolean);
}


// TODO:
//  - About LoxProgram
//      - change its name to LoxChunk of something similar
//      - change the code struct to be just an bytearray and have another struct called metadata with other things
//  - Add 'utils.c' and take some things from utils.h and and put them into actual functions
static LoxInterpretResult vm_run(LoxVM * vm){
#define BINARY(op, value_constructor) do {                                                 \
        if(!VAL_IS_NUMBER(vm_stack_peek(vm, 0)) || !VAL_IS_NUMBER(vm_stack_peek(vm, 1))) { \
            vm_report_runtime_error(vm, "operands should both be numbers");                \
            return INTERPRET_RUNTIME_ERROR;                                                \
        }                                                                                  \
        double b = vm_stack_pop(vm).as.number;                                             \
        double a = vm_stack_pop(vm).as.number;                                             \
        vm_stack_push(vm, value_constructor(a op b));                                      \
    } while(0)

#define READ_BYTE()  (*vm->ip++).op_code
#define READ_SHORT() (vm->ip += 2, (uint16_t) vm->ip[-1].op_code << 8 | vm->ip[-2].op_code)
#define READ_STRING() VAL_AS_STRING(vm_get_constant(vm, READ_BYTE()))

    vm->ip = vm->program.code.values;
    for(;;){

#ifdef DEBUG_TRACE_EXECUTION
        fputs("          ", stdout);
        if(vm->stack.length == 0)
            puts("[ ]");
        else {
            DA_FOR_EACH_ELEM(curr, &vm->stack, {
                bool is_str = VAL_IS_STRING(curr);
                fputs(is_str ? "[ \"" : "[ ", stdout);
                value_print(curr);
                fputs(is_str ? "\" ]" : " ]", stdout);
            });
            putchar('\n');
        }
        prog_instr_debug(&vm->program, (size_t) (vm->ip - vm->program.code.values));
#endif

        OpCode instr = READ_BYTE();
        switch (instr) {
            case OP_POP   : vm_stack_pop(vm); break;
            case OP_CONST : {
                uint8_t data_idx = READ_BYTE();
                LoxValue data = prog_get_constant(&vm->program, data_idx);
                vm_stack_push(vm, data);
            } break;

            case OP_NOT : 
                if(!VAL_IS_BOOL(vm_stack_peek(vm, 0))) {
                    vm_report_runtime_error(vm, "expected a boolean operand");
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm_stack_push(vm, BOOL_VAL(!vm_stack_pop(vm).as.boolean)); 
                break;

            case OP_NEG : 
                if(!VAL_IS_NUMBER(vm_stack_peek(vm, 0))) {
                    vm_report_runtime_error(vm, "expected a number operand");
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm_stack_push(vm, NUMBER_VAL(-vm_stack_pop(vm).as.number)); 
                break;

            case OP_ADD : {
                LoxValue b = vm_stack_peek(vm, 0);
                LoxValue a = vm_stack_peek(vm, 1);

                if(VAL_IS_STRING(a) || VAL_IS_STRING(b)) {
                    const LoxString * str2 = vm_stingify_value(vm, vm_stack_pop(vm));
                    const LoxString * str1 = vm_stingify_value(vm,vm_stack_pop(vm));

                    const LoxString * result;
                    { 
                        size_t length = strlen(str1->chars) + strlen(str2->chars);
                        char * buffer = mem_alloc(length + 1);

                        buffer[0] = 0;
                        strcat(buffer, str1->chars);
                        strcat(buffer + str1->length, str2->chars);

                        uint32_t hash = str_hash(buffer, length);
                        if((result = map_find_str(&vm->strings, buffer, length, hash)) == NULL) {
                            result = lox_str_take(buffer, length, hash);
                            vm_register_object(vm, (LoxObject*) result);
                        } else {
                            mem_dealloc(buffer);
                        }
                    }
                    vm_stack_push(vm, OBJ_VAL(result));
                } else if(VAL_IS_NUMBER(a) && VAL_IS_NUMBER(b)) {
                    vm_stack_push(vm, NUMBER_VAL( vm_stack_pop(vm).as.number + vm_stack_pop(vm).as.number));
                } else {
                    vm_report_runtime_error(vm, "operator '+' expects either two integers or at least 1 string");
                    return INTERPRET_RUNTIME_ERROR;
                }
            } break;

            case OP_SUB : BINARY(-, NUMBER_VAL); break;
            case OP_MULT: BINARY(*, NUMBER_VAL); break;
            case OP_DIV : BINARY(/, NUMBER_VAL); break;

            case OP_LESS    : BINARY(<, BOOL_VAL); break;
            case OP_GREATER : BINARY(>, BOOL_VAL); break;
            case OP_EQ      : 
                vm_stack_push(vm, BOOL_VAL(value_eq(vm_stack_pop(vm), vm_stack_pop(vm))));
                break;

            case OP_TRUE  : vm_stack_push(vm, BOOL_VAL(true)); break;
            case OP_FALSE : vm_stack_push(vm, BOOL_VAL(false)); break;
            case OP_NIL   : vm_stack_push(vm, NIL_VAL); break;


            case OP_PRINT : 
                value_print(vm_stack_pop(vm)); 
                putchar('\n');
                break;

            case OP_DEFINE_GLOBAL : {
                LoxString * name = READ_STRING();
                map_set(&vm->globals, name, vm_stack_peek(vm, 0));
                vm_stack_pop(vm);
            } break;

            case OP_SET_GLOBAL : {
                LoxString * name = READ_STRING();
                LoxValue * value = map_get_mut(&vm->globals, name);
                if(value == NULL) {
                    vm_report_runtime_error(vm, "assigment variable '%s' not defined", name->chars);
                    return  INTERPRET_RUNTIME_ERROR;
                }
                *value = vm_stack_peek(vm, 0);
            } break;

            case OP_GET_GLOBAL : {
                LoxString * name       = READ_STRING();
                const LoxValue * value = map_get(&vm->globals, name);
                if(value == NULL) {
                    vm_report_runtime_error(vm, "undefined identifier '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                vm_stack_push(vm, *value);
            } break;

            case OP_GET_LOCAL: vm_stack_push(vm, vm_stack_get(vm, READ_BYTE())); break;
            case OP_SET_LOCAL: vm_stack_set(vm, READ_BYTE(), vm_stack_peek(vm, 0)); break;

            case OP_IF_FALSE : {
                uint16_t offset = READ_SHORT();
                if(is_falsely(vm_stack_peek(vm, 0))) 
                    vm->ip += offset;
            } break;

            case OP_JUMP : {
                uint16_t offset = READ_SHORT();
                vm->ip += offset;
            } break;

            case OP_LOOP : {
                uint16_t offset = READ_SHORT();
                vm->ip -= offset;
            } break;

            case OP_RETURN:
                ASSERT(vm->stack.length == 0);
                return INTERPRET_OK;

            default:
                UNREACHABLE();
        }
    }

#undef BINARY
#undef READ_BYTE
#undef READ_STRING
}

LoxInterpretResult interpret(const char * source){
    LoxVM vm;
    vm_init(&vm);

    LoxInterpretResult res;
    if(!compile(source, &vm.program, &vm.strings)) {
       res = INTERPRET_COMPILE_ERROR;
    } else {
       res = vm_run(&vm);
    }

    vm_destroy(&vm);
    return res;
}
