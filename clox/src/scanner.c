#include "scanner.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define isletter(c) ((c) == '_' || isalpha(c))

static bool isover(const scanner_t * sc){
    return *sc->current == '\0';
}

static inline token_t make_token(const scanner_t * sc, token_type_t type){
    return (token_t) {
        .type   = type,
        .start  = sc->start,
        .length = (int) (sc->current - sc->start),
        .line   = sc->line,
    };
}

static inline token_t make_error(const scanner_t * sc, const char * msg){
    return (token_t) {
        .type   = TOKEN_ERROR,
        .start  = msg,
        .length = strlen(msg),
        .line   = sc->line,
    };
}

static inline char advance(scanner_t * sc){
    return *sc->current++;
}

static char peek(const scanner_t * sc){
    if(isover(sc))
        return '\0';
    return *sc->current;
}

static char peeknext(const scanner_t * sc){
    if(isover(sc))
        return '\0';
    return sc->current[1];
}

static bool match(scanner_t * sc, char value){
    if(peek(sc) != value)
        return false;
    
    advance(sc);
    return true;

}

static void skipspaces(scanner_t * sc){
    for(;;){
        char next = peek(sc);
        switch (next) {
            case '\n':
                sc->line++;
            case '\t':
            case ' ':
            case '\r':
                advance(sc);
            break;
            case '/':
                if(peeknext(sc) != '/') return;
                while(!isover(sc) && peek(sc) != '\n') 
                    advance(sc);
                break;
            default:
                return;
        }
    }
}


static token_t sc_number(scanner_t * sc){
    while( isdigit( peek(sc) ))
        advance(sc);

    if(peek(sc) == '.' && isdigit( peeknext(sc) ) ){
        advance(sc); // for the period :)

        do{
            advance(sc);
        } while( isdigit( peek(sc) ));
    }

    return make_token(sc, TOKEN_NUMBER);
}

static inline token_type_t check_keyword(
    const char * text, 
    size_t text_size,
    const char * keyword,
    size_t keyword_size,
    token_type_t type
){
    return keyword_size == text_size 
           && strncmp(text, keyword, text_size) == 0 
           ? type : TOKEN_IDENTIFIER;
}

// This uses a Trie (https://en.wikipedia.org/wiki/Trie) to distinguish 
// the identifiers from the reserved keyword
static token_type_t identifier_type(const char * ident, size_t size){

#define check_keyword(text, size, keyword, type) (                 \
    check_keyword(text, size, keyword, sizeof(keyword) - 1, type)  \
)

    switch (ident[0]) {
        case 'f':
            if(size > 1) {
                switch (ident[1]) {
                    case 'o':
                        return check_keyword(&ident[2], size - 2, "r", TOKEN_FOR);
                    case 'u':
                        return check_keyword(&ident[2], size - 2, "n",  TOKEN_FUN);
                    case 'a':
                        return check_keyword(&ident[2], size - 2, "lse", TOKEN_FALSE);
                }
            }
        break;
        case 't':
            if(size > 1) {
                switch (ident[1]) {
                    case 'h':
                        return check_keyword(&ident[2], size - 2, "is", TOKEN_THIS);
                    case 'r':
                        return check_keyword(&ident[2], size - 2, "ue",  TOKEN_TRUE);
                }
            }
        break;

        case 'a': return check_keyword(&ident[1], size - 1, "nd", TOKEN_AND);
        case 'o': return check_keyword(&ident[1], size - 1, "r", TOKEN_OR);
        case 'n': return check_keyword(&ident[1], size - 1, "il", TOKEN_NIL);
        case 'c': return check_keyword(&ident[1], size - 1, "lass", TOKEN_CLASS);
        case 's': return check_keyword(&ident[1], size - 1, "uper", TOKEN_SUPER);
        case 'v': return check_keyword(&ident[1], size - 1, "ar", TOKEN_VAR);
        case 'i': return check_keyword(&ident[1], size - 1, "f", TOKEN_IF);
        case 'e': return check_keyword(&ident[1], size - 1, "lse", TOKEN_ELSE);
        case 'w': return check_keyword(&ident[1], size - 1, "hile", TOKEN_WHILE);
        case 'r': return check_keyword(&ident[1], size - 1, "eturn", TOKEN_RETURN);
        case 'p': return check_keyword(&ident[1], size - 1, "rint", TOKEN_PRINT);
    }

#undef check_keyword

    return TOKEN_IDENTIFIER;
}

static token_t scan_identifier(scanner_t * sc){

    char next = peek(sc);
    while( isletter(next) || isdigit(next) ){
        advance(sc);
        next = peek(sc);
    }

    size_t ident_size = (size_t) (sc->current - sc->start);
    return make_token(
        sc, identifier_type(sc->start, ident_size)
    );
}

static token_t scan_string(scanner_t * sc){

    while(peek(sc) != '"' && !isover(sc)){
        if(peek(sc) == '\n') sc->line++;
        advance(sc);
    }

    if(isover(sc))
        return make_error(sc, "Unterminated string.");

    advance(sc); // enclosing quote c:
    return make_token(sc, TOKEN_STRING);
}

void sc_init(scanner_t * sc, const char * source){
    sc->start   = source;
    sc->current = source;
    sc->line = 0;
}

void sc_destroy(scanner_t * sc){
    sc->start   = NULL;
    sc->current = NULL;
    sc->line    = 0;
}

token_t sc_next_token(scanner_t * sc){
    // skip comments c:
    skipspaces(sc);

    sc->start = sc->current;
    if(isover(sc))
        return make_token(sc, TOKEN_EOF);

    char next = advance(sc);
    if(isdigit(next))
        return sc_number(sc);

    if(isletter(next))
        return scan_identifier(sc);

    switch (next){
        case '.' : return make_token(sc, TOKEN_DOT);

        case '+' : return make_token(sc, TOKEN_PLUS);
        case '-' : return make_token(sc, TOKEN_MINUS);
        case '*' : return make_token(sc, TOKEN_STAR);
        case '/' : return make_token(sc, TOKEN_SLASH);

        case '(' : return make_token(sc, TOKEN_LEFT_PAREN);
        case ')' : return make_token(sc, TOKEN_RIGHT_PAREN);
        case '{' : return make_token(sc, TOKEN_LEFT_BRACE);
        case '}' : return make_token(sc, TOKEN_RIGHT_BRACE);

        case ';' : return make_token(sc, TOKEN_SEMICOLON);
        case ',' : return make_token(sc, TOKEN_COMMA);

        case '>' : return make_token(
            sc, match(sc, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
        );

        case '<' : return make_token(
            sc, match(sc, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
        );

        case '=' : return make_token(
            sc, match(sc, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
        );

        case '!' : return make_token(
            sc, match(sc, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
        );

        case '"' : return scan_string(sc);

        default:
            return make_error(sc, "Unexpected character.");
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
        default: assert(0 && "unexpected token type");
    }

}
#undef CASE
