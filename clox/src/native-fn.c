#include "native-fn.h"

#include <time.h>

void lox_clock(LoxVM *vm) {
    vm_stack_push(vm, NUMBER_VAL(time(NULL)));
}
