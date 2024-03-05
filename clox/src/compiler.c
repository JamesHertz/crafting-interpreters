#include "compiler.h"
#include "scanner.h"

#include <stdio.h>

void compile(const char * source){
    scanner_t scan;
    scanner_init(&scan, source);

    token_t token;
    do{
        token = scanner_next_token(&scan);
        printf(
            "type: %s, line: %d, lexeme/msg: %.*s\n",
            tt2str(token.type), token.line, token.length, token.start
        );
    }while(token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

}
