#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>
#include "darray.h"

#define VAL_IS_BOOL(value)   ((value).type == VAL_BOOL)
#define VAL_IS_NIL(value)    ((value).type == VAL_NIL)
#define VAL_IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define VAL_IS_OBJ(value)    ((value).type == VAL_OBJ)
#define VAL_IS_STRING(value) value_is_of_object_type((value), OBJ_STRING)

#define VAL_AS_STRING(value)  ((LoxString *) (value).as.object)
#define VAL_AS_CSTRING(value) (VAL_AS_STRING(value)->chars)

#define VAL_AS_FUNC(value)  ((LoxFunction *) (value).as.object)
#define VAL_IS_FUNC(value)  value_is_of_object_type((value), OBJ_FUNC)

#define VAL_IS_NATIVE_FN(value)  value_is_of_object_type((value), OBJ_NATIVE_FN)
#define VAL_AS_NATIVE_FN(value)  ((LoxNativeFn *) (value).as.object)

#define BOOL_VAL(val)    ((LoxValue) { .type = VAL_BOOL,   .as.boolean = (val) })
#define NUMBER_VAL(val)  ((LoxValue) { .type = VAL_NUMBER, .as.number  = (val)  })
#define OBJ_VAL(val)     ((LoxValue) { .type = VAL_OBJ,    .as.object  = (LoxObject *) (val) })
#define NIL_VAL          ((LoxValue) { .type = VAL_NIL,    .as.number  = 0 })

typedef enum {
    OBJ_STRING,
    OBJ_FUNC,
    OBJ_NATIVE_FN
} LoxObjectType;

typedef struct __lox_object__ {
    LoxObjectType type;
    struct __lox_object__ * next;
} LoxObject;

typedef struct {
    LoxObject obj;
    size_t length;
    uint32_t hash;
    const char * chars;
} LoxString;

typedef enum {
    VAL_NIL,
    VAL_OBJ,
    VAL_BOOL,
    VAL_NUMBER,
} LoxValueType;

typedef struct {
    LoxValueType type;
    union {
        double number;
        bool boolean;
        LoxObject * object;
    } as;
} LoxValue;

typedef struct {
    uint32_t line;
    uint8_t  op_code;
} Instruction;

typedef struct {
    DaArray(Instruction) code;
    DaArray(LoxValue) constants;
} LoxChunk;

typedef enum {
    FUNC_SCRIPT,
    FUNC_ORDINARY,
    FUNC_ANONYMOUS,
} LoxFuncType;

typedef struct {
    LoxObject obj;
    LoxChunk chunk;
    const LoxString * name;
    LoxFuncType type;
    uint8_t arity;
} LoxFunction;

struct __lox_vm__;
typedef void (*Fn)(struct __lox_vm__ * vm);

typedef struct {
    LoxObject obj;
    uint8_t arity;
    Fn executor;
} LoxNativeFn;

typedef struct {
    const char * name;
    uint32_t arity;
} LoxCallable;

void value_print(LoxValue value);
bool value_eq(LoxValue v1, LoxValue v2);
static inline bool value_is_of_object_type(LoxValue value, LoxObjectType type) {
    return VAL_IS_OBJ(value) && value.as.object->type == type;
}

struct __hash_map__;
const LoxString * lox_str_intern(struct __hash_map__ * strings, const char * str, size_t length);

LoxString * lox_str_copy(const char * str, size_t length, uint32_t hash);
LoxString * lox_str_take(const char * str, size_t length, uint32_t hash);
bool lox_str_eq(const LoxString * s1, const LoxString * s2);
void lox_obj_destroy(LoxObject * obj);

LoxFunction * lox_func_create(const LoxString * name, LoxFuncType type);

bool lox_make_callable(LoxCallable * callable, const LoxValue value);

#endif 
