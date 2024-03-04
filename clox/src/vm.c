#include "vm.h"
#include "compiler.h"

interpret_result_t interpret(const char * source){
    compile(source);
    return INTERPRET_OK;
}