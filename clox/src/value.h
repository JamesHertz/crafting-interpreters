#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include <stdbool.h>
#include <sys/types.h>

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


void value_print(value_t value);
bool value_eq(value_t v1, value_t v2);
obj_string_t * value_copy_string(const char * str, size_t size);
obj_string_t * value_take_string(const char * str, size_t size);

void obj_destroy(object_t * obj);
obj_string_t * str_concat(const obj_string_t * str1, const obj_string_t * str2);

static inline bool value_is_of_object_type(value_t value, object_type_t type) {
    return VAL_IS_OBJ(value) && value.as.object->type == type;
}
#endif 
