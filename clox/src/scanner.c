#include "scanner.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define isalpha(c) ((c) == '_' || isalpha(c))

static bool isover(const scanner_t * scan){
    return *scan->current == '\0';
}

static inline token_t make_token(scanner_t * scan, token_type_t type){
    return (token_t) {
        .type   = type,
        .start  = scan->start,
        .length = (int) (scan->current - scan->start),
        .line   = scan->line,
    };
}

static inline token_t make_error(const scanner_t * scan, const char * msg){
    return (token_t) {
        .type   = TOKEN_ERROR,
        .start  = msg,
        .length = strlen(msg),
        .line   = scan->line,
    };
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

// static inline bool check(const scanner_t * scan, char value){
//     return peek(scan) == value;
// }

static void skipspaces(scanner_t * scan){
    for(;;){
        char next = peek(scan);
        switch (next) {
            case '\n':
                scan->line++;
            case '\t':
            case ' ':
            case '\r':
                advance(scan);
            break;

            case '/':
                if(!match(scan, '/')) return ;
                while(!isover(scan) && peek(scan) != '\n') 
                    advance(scan);
                break;
            default:
                return;
        }
    }
}


static token_t scan_number(scanner_t * scan){
    while( isdigit( peek(scan) ))
        advance(scan);

    if(peek(scan) == '.' && isdigit( peeknext(scan) ) ){
        advance(scan); // for the period :)

        do{
            advance(scan);
        } while( isdigit( peek(scan) ));
    }

    return make_token(scan, TOKEN_NUMBER);
}

static token_t scan_identifier(scanner_t * scan){
    return (token_t){};
}

static token_t scan_string(scanner_t * scan){

    while(peek(scan) != '"' && !isover(scan)){
        if(peek(scan) == '\n') scan->line++;
        advance(scan);
    }

    if(isover(scan))
        return make_error(scan, "Unterminated string.");

    advance(scan); // enclosing quote c:
    return make_token(scan, TOKEN_STRING);
}

void scanner_init(scanner_t * scan, const char * source){
    scan->start   = source;
    scan->current = source;
    scan->line = 0;
}

token_t scanner_next_token(scanner_t * scan){
    // skip comments c:
    skipspaces(scan);

    scan->start = scan->current;
    if(isover(scan))
        return make_token(scan, TOKEN_EOF);

    char next = advance(scan);
    if(isdigit(next))
        return scan_number(scan);

    // if(isalpha(next))
    //     return scan_identifier(scan);

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


// for debugging purpose c:
#define CASE(token) case token : return #token
char * tt2str(token_type_t token){

    switch (token)
    {
        CASE(TOKEN_LEFT_PAREN); CASE(TOKEN_RIGHT_PAREN);
        CASE(TOKEN_LEFT_BRACE); CASE(TOKEN_RIGHT_BRACE);

        CASE(TOKEN_COMMA); CASE(TOKEN_DOT);
        CASE(TOKEN_MINUS); CASE(TOKEN_PLUS);

        CASE(TOKEN_SEMICOLON); CASE(TOKEN_SLASH); 
        CASE(TOKEN_STAR);

        // One or two character tokens.
        CASE(TOKEN_BANG); CASE(TOKEN_BANG_EQUAL);
        CASE(TOKEN_EQUAL); CASE(TOKEN_EQUAL_EQUAL);
        CASE(TOKEN_GREATER); CASE(TOKEN_GREATER_EQUAL);

        CASE(TOKEN_LESS); CASE(TOKEN_LESS_EQUAL);

        // Literals.
        CASE(TOKEN_IDENTIFIER); CASE(TOKEN_STRING);
        CASE(TOKEN_NUMBER);

        // Keywords.
        CASE(TOKEN_AND); CASE(TOKEN_CLASS); 
        CASE(TOKEN_ELSE); CASE(TOKEN_FALSE);

        CASE(TOKEN_FOR); CASE(TOKEN_FUN); CASE(TOKEN_IF);
        CASE(TOKEN_NIL); CASE(TOKEN_OR);

        CASE(TOKEN_PRINT); CASE(TOKEN_RETURN); 
        CASE(TOKEN_SUPER); CASE(TOKEN_THIS);

        CASE(TOKEN_TRUE); CASE(TOKEN_VAR);
        CASE(TOKEN_WHILE);

        // extra
        CASE(TOKEN_ERROR); CASE(TOKEN_EOF);
        default:
            return NULL;
    }


#undef CASE
}