#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "hash-map.h"
#include "function.h"
#include "constants.h"
#include "utils.h"

#define MAX_STACK_SIZE (MAX_LOCALS * MAX_STACK_FRAMES)

typedef struct {
    LoxFunction * func;
    Instruction * ip;
    LoxValue * locals;
} LoxCallFrame;

typedef struct __lox_vm__ {
    struct {
        LoxValue values[MAX_STACK_SIZE];
        size_t length;
    } stack;

    LoxCallFrame frames[MAX_STACK_FRAMES];
    size_t frames_count;

    HashMap strings;
    HashMap globals;
    LoxObject * objects;
} LoxVM;

void vm_report_runtime_error(LoxVM * vm, const char * format, ...) 
    __attribute__((format (printf, 2, 3)));

LoxValue vm_stack_peek(LoxVM * vm, size_t distance);
LoxValue vm_stack_pop(LoxVM * vm);
void vm_stack_push(LoxVM * vm, LoxValue value);

static inline LoxValue vm_func_get_arg(LoxVM * vm, uint8_t arg_pos) {
    return vm_stack_peek(vm, arg_pos);
}

void vm_define_native_fn(LoxVM * vm, const char * name, Fn executor, uint8_t arity);

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} LoxInterpretResult;

LoxInterpretResult interpret(const char * source);

#endif
