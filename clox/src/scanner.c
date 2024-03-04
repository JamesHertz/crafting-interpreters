#include "scanner.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define isalpha(c) ((c) == '_' || isalpha(c))

static bool isover(const scanner_t * scan){
    return *scan->current == '\0';
}

static inline token_t make_token(scanner_t * scan, token_type_t type){
    token_t result =  {
        .type   = type,
        .start  = scan->start,
        .length = (int) (scan->current - scan->start),
        .line   = scan->line,
    };

    scan->start = scan->current;
    return result;
}

static inline token_t make_error(const scanner_t * scan, const char * msg){
    return (token_t) {
        .type   = TOKEN_EOF,
        .start  = msg,
        .length = strlen(msg),
        .line   = scan->line,
    };
}

static void skipspaces(scanner_t * scan){
    // TO BE COMPLETED
}

static inline char advance(scanner_t * scan){
    return *scan->current++;
}

static char peek(const scanner_t * scan){
    if(isover(scan))
        return '\0';
    return *scan->current;
}

static char peeknext(const scanner_t * scan){
    if(isover(scan))
        return '\0';
    return scan->current[1];
}

static bool match(scanner_t * scan, char value){
    if(peek(scan) != value)
        return false;
    
    advance(scan);
    return true;

}

static token_t scan_number(scanner_t * scan){
    return (token_t){};
}

static token_t scan_identifier(scanner_t * scan){
    return (token_t){};
}

static token_t scan_string(scanner_t * scan){
    return (token_t){};
}

void scanner_init(scanner_t * scan, const char * source){
    scan->start   = source;
    scan->current = source;
    scan->line = 0;
}

token_t scanner_next_token(scanner_t * scan){

    if(isover(scan))
        return make_token(scan, TOKEN_EOF);

    // skip comments c:
    skipspaces(scan);

    char next = advance(scan);

    if(isdigit(next))
        return scan_number(scan);

    if(isalpha(next))
        return scan_identifier(scan);

    switch (next){
        case '.' : return make_token(scan, TOKEN_DOT);

        case '+' : return make_token(scan, TOKEN_PLUS);
        case '-' : return make_token(scan, TOKEN_MINUS);
        case '*' : return make_token(scan, TOKEN_PLUS);
        case '/' : return make_token(scan, TOKEN_SLASH);

        case '(' : return make_token(scan, TOKEN_LEFT_PAREN);
        case ')' : return make_token(scan, TOKEN_RIGHT_PAREN);
        case '{' : return make_token(scan, TOKEN_LEFT_BRACE);
        case '}' : return make_token(scan, TOKEN_RIGHT_BRACE);

        case ';' : return make_token(scan, TOKEN_SEMICOLON);
        case ',' : return make_token(scan, TOKEN_COMMA);

        case '>' : return make_token(
            scan, match(scan, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
        );

        case '<' : return make_token(
            scan, match(scan, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
        );

        case '=' : return make_token(
            scan, match(scan, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
        );

        case '!' : return make_token(
            scan, match(scan, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
        );

        case '"' : return scan_string(scan);

        default:
            return make_error(scan, "Unexpected character.");
    }

}