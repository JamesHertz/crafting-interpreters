#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>

#define VAL_IS_BOOL(value)   ((value).type == VAL_BOOL)
#define VAL_IS_NIL(value)    ((value).type == VAL_NIL)
#define VAL_IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define VAL_IS_OBJ(value)    ((value).type == VAL_OBJ)
#define VAL_IS_STRING(value)  value_is_of_object_type((value), OBJ_STRING)

#define VAL_AS_STRING(value)  ((LoxString *) (value).as.object)
#define VAL_AS_CSTRING(value) (VAL_AS_STRING(value)->chars)

#define BOOL_VAL(val)    ((LoxValue) { .type = VAL_BOOL,   .as.boolean = (val) })
#define NUMBER_VAL(val)  ((LoxValue) { .type = VAL_NUMBER, .as.number  = (val)  })
#define OBJ_VAL(val)     ((LoxValue) { .type = VAL_OBJ,    .as.object  = (LoxObject *) (val) })
#define NIL_VAL          ((LoxValue) { .type = VAL_NIL,    .as.number  = 0 })

typedef enum {
    OBJ_STRING,
    OBJ_FUNC
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

// by now c:
typedef struct {
    LoxValueType type;
    union {
        double number;
        bool boolean;
        LoxObject * object;
    } as;
} LoxValue;

void value_print(LoxValue value);
bool value_eq(LoxValue v1, LoxValue v2);

LoxString * lox_str_copy(const char * str, size_t length, uint32_t hash);
LoxString * lox_str_take(const char * str, size_t length, uint32_t hash);
bool lox_str_eq(const LoxString * s1, const LoxString * s2);
void lox_obj_destroy(LoxObject * obj);

static inline bool value_is_of_object_type(LoxValue value, LoxObjectType type) {
    return VAL_IS_OBJ(value) && value.as.object->type == type;
}

#endif 
