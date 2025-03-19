#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "value.h"
#include "chunk.h"

#define VAL_AS_FUNC(value)  ((LoxFunction *) (value).as.object)
#define VAL_IS_FUNC(value)  value_is_of_object_type((value), OBJ_FUNC)

typedef enum {
    FUNC_SCRIPT,
    FUNC_ORDINARY,
    FUNC_ANONYMOUS
} LoxFuncType;

typedef struct {
    LoxObject obj;
    LoxChunk chunk;
    const LoxString * name;
    LoxFuncType type;
    uint8_t arity;
} LoxFunction;

LoxFunction * lox_func_create(const LoxString * name, LoxFuncType type);
#endif
