#include "vm.h"
#include "compiler.h"
#include "chunk.h"
#include "hash-map.h"
#include "memory.h"
#include "debug.h"
#include "utils.h"

#include "constants.h"
#include "native-fn.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static void vm_init(LoxVM * vm){
    map_init(&vm->strings);
    map_init(&vm->globals);
    vm->objects      = NULL;
    vm->stack.length = 0;
    vm->frames_count = 0;

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
    map_destroy(&vm->strings);
    map_destroy(&vm->globals);
    vm->stack.length = 0;
}

static inline LoxValue vm_stack_get(LoxVM * vm, size_t idx){
    ASSERTF(idx < vm->stack.length, "accessing invalid stack position");
    return vm->stack.values[idx];
}

static inline void vm_stack_set(LoxVM * vm, size_t idx, LoxValue value){
    ASSERT(idx < vm->stack.length);
    vm->stack.values[idx] = value;
}

static inline LoxCallFrame * vm_current_frame(LoxVM * vm) {
    ASSERT(vm->frames_count > 0);
    return &vm->frames[vm->frames_count - 1];
}

static inline LoxValue vm_get_constant(LoxVM * vm, uint8_t idx){
    return chunk_get_constant(&vm_current_frame(vm)->func->chunk, idx);
}

static void vm_register_object(LoxVM * vm, LoxObject * obj){
    ASSERTF(obj->next == NULL, "invalid object: obj->next field not null");
    obj->next   = vm->objects;
    vm->objects = obj;
}

void vm_define_native_fn(LoxVM * vm, const char * name, Fn executor, uint8_t arity) {
    const LoxString * fn_name = lox_str_intern(&vm->strings, name, strlen(name));
    LoxNativeFn * fn = mem_alloc(sizeof(LoxNativeFn));

    fn->obj = (LoxObject) {
        .type = OBJ_NATIVE_FN,
        .next = NULL,
    };
    fn->arity    = arity;
    fn->executor = executor;

    vm_register_object(vm, &fn->obj);
    map_set(&vm->globals, fn_name, OBJ_VAL(fn));
}

void vm_stack_push(LoxVM * vm, LoxValue value){
    ASSERTF(vm->stack.length < MAX_STACK_SIZE, "stack overflow");
    vm->stack.values[vm->stack.length++] = value;
}

LoxValue vm_stack_pop(LoxVM * vm){
    ASSERT(vm->stack.length > 0);
    return vm->stack.values[--vm->stack.length];
}

LoxValue vm_stack_peek(LoxVM * vm, size_t distance){
    size_t idx = vm->stack.length - (1 + distance);
    ASSERT(idx <= MAX_STACK_SIZE);
    return vm->stack.values[idx];
}

void vm_report_runtime_error(LoxVM * vm, const char * format, ...) {
    va_list list;
    va_start(list, format);
    fputs("[ RunTimeError ] : ", stderr);
    vfprintf(stderr, format, list);
    va_end(list);

    for(ssize_t i = vm->frames_count - 1; i >= 0; i--) {
        LoxCallFrame * frame = &vm->frames[i];
        Instruction instr = frame->ip[-1];

        fprintf(stderr, "\n[line %u] in ", instr.line);
        switch(frame->func->type) {
            case FUNC_SCRIPT   : fputs("script\n", stderr); break;
            case FUNC_ORDINARY : 
                fprintf(stderr, "%s()", frame->func->name->chars);
                break;
            case FUNC_ANONYMOUS: fprintf(stderr, "<%p>()", frame->func); break;
            default:
                UNREACHABLE();
        }

    }
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

static LoxCallFrame * vm_frames_push(LoxVM * vm, LoxFunction * func, uint8_t args_nr) {
    ASSERTF(vm->frames_count < MAX_STACK_FRAMES, "call frame overflow");

    LoxCallFrame * frame = &vm->frames[vm->frames_count++];
    frame->func    = func;
    frame->ip      = func->chunk.code.values;
    frame->locals  = &vm->stack.values[vm->stack.length - (1 + args_nr)];
    return frame;
}

static LoxCallFrame * vm_frames_pop(LoxVM * vm) {
    ASSERT(vm->frames_count > 0);
    vm->frames_count--;
    return vm->frames_count == 0 ? NULL : &vm->frames[vm->frames_count - 1];
}

// TODO:
//  - [x] Make vm.stack be a static array c:
//  - [x] About LoxChunk
//      - [x] change its name to LoxChunk of something similar
//      - [ ] change the code struct to be just an bytearray and have another struct called metadata with other things
//  - [x] Add 'utils.c' and take some things from utils.h and and put them into actual functions
//  - [ ] Add support for anonymous functions and native functions
static LoxInterpretResult vm_run(LoxVM * vm, LoxFunction * script){
#define BINARY(op, value_constructor) do {                                                 \
        if(!VAL_IS_NUMBER(vm_stack_peek(vm, 0)) || !VAL_IS_NUMBER(vm_stack_peek(vm, 1))) { \
            vm_report_runtime_error(vm, "operands should both be numbers");                \
            return INTERPRET_RUNTIME_ERROR;                                                \
        }                                                                                  \
        double b = vm_stack_pop(vm).as.number;                                             \
        double a = vm_stack_pop(vm).as.number;                                             \
        vm_stack_push(vm, value_constructor(a op b));                                      \
    } while(0)

#define READ_BYTE()   (*frame->ip++).op_code
#define READ_SHORT()  (frame->ip += 2, (uint16_t) frame->ip[-1].op_code << 8 | frame->ip[-2].op_code)
#define READ_STRING() VAL_AS_STRING(vm_get_constant(vm, READ_BYTE()))

    ASSERT(vm->frames_count == 0);
    vm_stack_push(vm, OBJ_VAL(script));
    LoxCallFrame * frame = vm_frames_push(vm, script, 0);
    for(;;){

#ifdef DEBUG_TRACE_EXECUTION
        fputs("          ", stdout);
        if(vm->stack.length == 0)
            puts("[ ]");
        else {
            for(size_t i = 0; i < vm->stack.length; i++) {
                LoxValue curr = vm->stack.values[i];
                bool is_str = VAL_IS_STRING(curr);
                fputs(is_str ? "[ \"" : "[ ", stdout);
                value_print(curr);
                fputs(is_str ? "\" ]" : " ]", stdout);
            }
            putchar('\n');
        }
        chunk_instr_debug(&frame->func->chunk, (size_t) (frame->ip - frame->func->chunk.code.values));
#endif

        OpCode instr = READ_BYTE();
        switch (instr) {
            case OP_POP   : vm_stack_pop(vm); break;
            case OP_CONST : {
                uint8_t data_idx = READ_BYTE();
                LoxValue data = vm_get_constant(vm, data_idx);
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
                    return INTERPRET_RUNTIME_ERROR;
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

            case OP_GET_LOCAL: vm_stack_push(vm, frame->locals[READ_BYTE()]);        break;
            case OP_SET_LOCAL: frame->locals[READ_BYTE()] = vm_stack_peek(vm, 0); break;

            case OP_IF_FALSE : {
                uint16_t offset = READ_SHORT();
                if(is_falsely(vm_stack_peek(vm, 0))) 
                    frame->ip += offset;
            } break;

            case OP_JUMP : {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
            } break;

            case OP_LOOP : {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
            } break;

            case OP_CALL : {
                uint8_t args_nr = READ_BYTE();
                LoxValue value  = vm_stack_peek(vm, args_nr);

                LoxCallable callable;
                if(!lox_make_callable(&callable, value)) {
                    vm_report_runtime_error(vm, "can only call functions");
                    return INTERPRET_RUNTIME_ERROR;
                }

                if(callable.arity != args_nr) {
                    vm_report_runtime_error(
                        vm, "function '%s' expects %u arguments but %u was provided", 
                        callable.name, (unsigned) callable.arity, (unsigned) args_nr
                    );
                    return INTERPRET_RUNTIME_ERROR;
                }

                if(VAL_IS_FUNC(value))
                    frame = vm_frames_push(vm, VAL_AS_FUNC(value), args_nr);
                else {
                    size_t stack_top = vm->stack.length;
                    VAL_AS_NATIVE_FN(value)->executor(vm);

                    if(stack_top + 1 != vm->stack.length) {
                        vm_report_runtime_error(vm, "native function call left stack in bad state");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    LoxValue return_value = vm_stack_pop(vm);
                    vm_stack_pop(vm);
                    vm_stack_push(vm, return_value);
                }
            } break;

            case OP_RETURN: {
                LoxCallFrame * old = frame;
                frame = vm_frames_pop(vm);
                if(frame == NULL) {
                    ASSERT(vm->stack.length == 0);
                    return INTERPRET_OK;
                }

                LoxValue value   = vm_stack_pop(vm);
                vm->stack.length = old->locals - vm->stack.values;
                vm_stack_push(vm, value);
            } break;
            default:
                UNREACHABLE();
        }
    }

    ASSERT(vm->frames_count == 0);
#undef BINARY
#undef READ_BYTE
#undef READ_STRING
}

LoxInterpretResult interpret(const char * source){
    LoxVM vm;
    vm_init(&vm);
    load_native_funcs(&vm);

    LoxFunction * script = compile(source, &vm.strings);
    LoxInterpretResult res = script == NULL ? INTERPRET_COMPILE_ERROR : vm_run(&vm, script);

    vm_destroy(&vm);
    return res;
}
