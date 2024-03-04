#ifndef CLOX_VM_H
#define CLOX_VM_H


typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} interpret_result_t;


interpret_result_t interpret(const char * source);

#endif