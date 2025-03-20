#include <stdio.h>
#include <string.h>

#include "value.h"
#include "memory.h"
#include "utils.h"
#include "chunk.h"
#include "hash-map.h"

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
            switch(value.as.object->type) {
                case OBJ_STRING:
                    fputs(VAL_AS_CSTRING(value), stdout);
                    break;
                case OBJ_NATIVE_FN:
                    fputs("<native fn>", stdout);
                    break;
                case OBJ_FUNC: {
                    LoxFunction * func = VAL_AS_FUNC(value);

                    switch(func->type) {
                        case FUNC_SCRIPT    : fputs("<script fn>", stdout);      break;
                        case FUNC_ANONYMOUS : fputs("<anonymous fn>", stdout);   break;
                        case FUNC_ORDINARY  : printf("<fn %s>", func->name->chars); break;
                        default: UNREACHABLE();
                    }
                } break;
                default: UNREACHABLE();
            } break;
        default: UNREACHABLE();
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

LoxFunction * lox_func_create(const LoxString * name, LoxFuncType type) {
    LoxFunction * func = mem_alloc(sizeof(LoxFunction));

    func->obj.type = OBJ_FUNC;
    func->obj.next = NULL;

    func->type     = type;
    func->name     = name;
    func->arity    = 0;

    chunk_init(&func->chunk);
    return func;
}

const LoxString * lox_str_intern(struct __hash_map__ * strings, const char * str, size_t length) {
    uint32_t hash = str_hash(str, length);
    const LoxString * lox_str;
    if((lox_str = map_find_str(strings, str, length, hash)) == NULL) {
        lox_str = lox_str_copy(str, length, hash);
        map_set(strings, lox_str, BOOL_VAL(true));
    }
    return lox_str;
}

bool lox_make_callable(LoxCallable * callable, const LoxValue value) {
    if(VAL_IS_FUNC(value)) {
        LoxFunction * func = VAL_AS_FUNC(value);
        callable->arity = func->arity;
        callable->name  = 
            func->type == FUNC_ORDINARY ? func->name->chars 
                : (func->type == FUNC_SCRIPT ? "<script fn>" : "<anonymous fn>" );
        return true;
    } 

    if(VAL_IS_NATIVE_FN(value)) {
        callable->arity = VAL_AS_NATIVE_FN(value)->arity;
        callable->name  = "<native fn>";
        return true;
    }

    return false;
}
