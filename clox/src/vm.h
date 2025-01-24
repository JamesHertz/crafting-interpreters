#ifndef CLOX_VM_H
#define CLOX_VM_H

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} LoxInterpretResult;

LoxInterpretResult interpret(const char * source);

#endif
