#include <stdio.h>
#include <string.h>

#include "value.h"
#include "memory.h"
#include "utils.h"

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
            ASSERT(VAL_IS_STRING(value));
            fputs(VAL_AS_CSTRING(value), stdout);
            break;
        default:
            UNREACHABLE();
    }
}


bool value_eq(LoxValue v1, LoxValue v2) {
    if(v1.type != v2.type) return false;

    switch(v1.type) {
        case VAL_NIL:
            return true;
        case VAL_NUMBER:
            return v1.as.number == v2.as.number;
        case VAL_BOOL:
            return v1.as.boolean == v2.as.boolean;
        case VAL_OBJ:
            return v1.as.object == v2.as.object;
        default:
            UNREACHABLE();
    }
    return false;
}

LoxString * lox_str_take(const char * str, size_t length, uint32_t hash) {
    LoxString * lstr = mem_alloc(sizeof(LoxString));

    lstr->obj.type = OBJ_STRING;
    lstr->obj.next = NULL;

    lstr->chars  = str;
    lstr->length = length;
    lstr->hash   = hash;
    return lstr;
}

LoxString * lox_str_copy(const char * str, size_t length, uint32_t hash) {
    char * str_value = mem_alloc(length + 1);

    // I could've used `strncpy(str_value, str, size)` but valgrind doesn't like it
    // so I used the following two lines just to shut it up c:
    memcpy(str_value, str, length);
    str_value[length] = 0;

    return lox_str_take(str_value, length, hash);
}

bool lox_str_eq(const LoxString * s1, const LoxString * s2) {
    return s1->hash == s2->hash 
        && s1->length == s2->length 
        && memcmp(s1->chars, s2->chars, s1->length) == 0;
}

void lox_obj_destroy(LoxObject * obj) {
    obj->next = NULL;
    mem_dealloc(obj);
}
