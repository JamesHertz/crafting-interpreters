#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "value.h"
#include "memory.h"

void value_print(LoxValue value){
    switch(value.type) {
        case VAL_NUMBER:
            printf("%g", value.as.number);
            break;
        case VAL_BOOL:
            fputs(value.as.boolean ? "true" : "false", stdout);
           break;
        case VAL_NIL:
            fputs("nil", stdout);
            break;
        case VAL_OBJ:
            assert(VAL_IS_STRING(value));
            fputs(VAL_AS_CSTRING(value), stdout);
            break;
        default:
            assert( 0 && "value_print(): invalid value type" );
    }
}

bool value_eq(LoxValue v1, LoxValue v2) {
    if(v1.type != v2.type) return false;

    if(VAL_IS_STRING(v1)) {
        LoxString * str1 = VAL_AS_STRING(v1);
        LoxString * str2 = VAL_AS_STRING(v2);
        return str1->length == str2->length && strcmp(str1->chars, str2->chars) == 0;
    }

    switch(v1.type) {
        case VAL_NUMBER:
            return v1.as.number == v2.as.number;
        case VAL_BOOL:
            return v1.as.boolean == v2.as.boolean;
        case VAL_NIL:
            return true;
        default:
            assert( 0 && "value_print(): invalid value type" );
    }
    return false;
}

LoxString * lox_str_take(const char * str, size_t size) {
    LoxString * value = mem_alloc(sizeof(LoxString));

    value->obj.type = OBJ_STRING;
    value->obj.next = NULL;

    value->chars  = str;
    value->length = size;
    return value;
}

LoxString * lox_str_copy(const char * str, size_t size) {
    char * str_value = mem_alloc(size + 1);

    // I could've used `strncpy(str_value, str, size)` but valgrind didn't like it
    // so I used the following two lines just to shut it up c:
    memcpy(str_value, str, size);
    str_value[size] = 0;

    return lox_str_take(str_value, size);
}

void obj_destroy(LoxObject * obj) {
    obj->next = NULL;
    mem_dealloc(obj);
}

LoxString * lox_str_concat(const LoxString * str1, const LoxString * str2) {
    size_t total_length = str1->length + str2->length;
    char * result = mem_alloc(total_length + 1);
    result[0] = 0;

    strcat(result, str1->chars);
    strcat(result + str1->length, str2->chars);
    return lox_str_take(result, total_length);
}
