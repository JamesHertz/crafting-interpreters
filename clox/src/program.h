#ifndef CLOX_PROGRAM_H
#define CLOX_PROGRAM_H

#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>

#include "darray.h"

#define VAL_IS_BOOL(value)   ((value).type == VAL_BOOL)
#define VAL_IS_NIL(value)    ((value).type == VAL_NIL)
#define VAL_IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define VAL_IS_OBJ(value)    ((value).type == VAL_OBJ)
#define VAL_IS_STRING(value)  value_is_of_object_type((value), OBJ_STRING)

#define VAL_AS_STRING(value)  ((obj_string_t *) (value).as.object)
#define VAL_AS_CSTRING(value) (VAL_AS_STRING(value)->chars)

#define BOOL_VAL(bool)   ((value_t) { .type = VAL_BOOL,   .as.boolean = (bool) })
#define NUMBER_VAL(num)  ((value_t) { .type = VAL_NUMBER, .as.number  = (num)  })
#define OBJ_VAL(obj)     ((value_t) { .type = VAL_OBJ,    .as.object  = (object_t *) (obj) })
#define NIL_VAL          ((value_t) { .type = VAL_NIL,    .as.number  = 0 })

typedef enum {
    OBJ_STRING
} object_type_t;

typedef struct _obj_ {
    object_type_t type;
    struct _obj_ * next;
} object_t;

typedef struct {
    object_t obj;
    size_t length;
    const char * chars;
} obj_string_t;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
} value_type_t;

// by now c:
typedef struct {
    value_type_t type;
    union {
        double number;
        bool boolean;
        object_t * object;
    } as;
} value_t;


typedef enum {
    OP_CONST,
    OP_RETURN,

    // arithmetic horsemans
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,

    // comparison
    OP_EQ,
    OP_LESS,
    OP_GREATER,
    
    // boolean
    OP_NOT,

    // value_t values
    OP_NIL,
    OP_TRUE,
    OP_FALSE
} op_code_t;

// WARN: please don't alter the order <values>, <length>, <size> of the inner structs, this is crucial!
typedef struct {
    uint32_t line;
    uint8_t op_code;
} instr_t;

typedef struct {
    DA_DECLARE_ARRAY(instr_t) code;
    DA_DECLARE_ARRAY(value_t) constants;
} program_t;

void prog_init(program_t * p);
size_t prog_add_constant(program_t * p, value_t value);
value_t prog_get_constant(const program_t * p, size_t idx);
void prog_add_instr(program_t * p, uint8_t value, uint32_t line);
void prog_destroy(program_t * p);

void prog_debug(const program_t * p, const char * title);
size_t prog_instr_debug(const program_t * p, size_t offset);

static inline bool value_is_of_object_type(value_t value, object_type_t type) {
    return VAL_IS_OBJ(value) && value.as.object->type == type;
}

void value_print(value_t value);
bool value_eq(value_t v1, value_t v2);
obj_string_t * value_copy_string(const char * str, size_t size);
obj_string_t * value_take_string(const char * str, size_t size);

void obj_destroy(object_t * obj);
obj_string_t * str_concat(const obj_string_t * str1, const obj_string_t * str2);

#endif
