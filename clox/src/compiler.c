#include "compiler.h"
#include "program.h"
#include "scanner.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LOCALS (UINT8_MAX + 1)

typedef struct {
    Token name;
    uint32_t scope;
} LocalVar;

typedef struct {
    LoxScanner in;
    LoxProgram * out_prog;
    HashMap * strings;

    Token previous;
    Token current;

    // TODO: make this one var
    bool error_found;
    bool in_panic_mode;
    bool can_assign;

    LocalVar locals[MAX_LOCALS];
    uint32_t localsCount;
    uint32_t currentScope;
} LoxParser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} ExprPrecedence;

typedef void (*ParserFunc)(LoxParser *);

typedef struct {
    ParserFunc   prefix;
    ParserFunc   infix;
    ExprPrecedence precedence;
} LoxParserRule;

static LoxParserRule * get_parse_rule(TokenType tt);
static void psr_parse_expression(LoxParser * psr);
static void psr_parse_declaration(LoxParser * psr);

static void psr_init(LoxParser * psr, const char * source, LoxProgram * out_prog, HashMap * strings) {
    sc_init(&psr->in, source);
    psr->out_prog = out_prog;
    psr->strings  = strings;
    psr->previous = psr->current = (Token) {0};

    psr->error_found   = false;
    psr->in_panic_mode = false;
    psr->can_assign    = false;

    // locals things
    psr->localsCount  = 0;
    psr->currentScope = 0;
    // psr->locals = ...;
}

// helper functions
static void psr_error_at(LoxParser * psr, Token * pos, const char * msg) {
    if(psr->in_panic_mode) return;

    fprintf(stderr, "[line %d] Error", pos->line);
    if(pos->type == TOKEN_EOF)
        fprintf(stderr, " at the end"); 
    else if(pos->type != TOKEN_ERROR)
        fprintf(stderr, " at '%.*s'", (int) pos->length, pos->start); 

    fprintf(stderr, ": %s\n", msg);
    psr->in_panic_mode = true;
    psr->error_found   = true;
}

static void psr_emit_byte(LoxParser * psr, uint8_t byte) {
    prog_add_instr(psr->out_prog, byte, psr->previous.line);
}

static void psr_emit_bytes(LoxParser * psr, uint8_t byte1, uint8_t byte2) {
    psr_emit_byte(psr, byte1);
    psr_emit_byte(psr, byte2);
}

static uint8_t psr_add_constant(LoxParser * psr, LoxValue constant) {
    size_t constant_idx = prog_add_constant(psr->out_prog, constant);
    if(constant_idx > UINT16_MAX) {
        psr_error_at(psr, &psr->previous, "too many constants for current chunk (?)");
        constant_idx = 0;
    }

    return constant_idx;
}

static uint8_t psr_add_str_constant(LoxParser * psr, const char * chars, size_t length) {
    uint32_t hash = str_hash(chars, length);
    const LoxString * str;
    if((str = map_find_str(psr->strings, chars, length, hash)) == NULL){
        str = lox_str_copy(chars, length, hash);
        map_set(psr->strings, str, BOOL_VAL(true));
    }
    return psr_add_constant(psr, OBJ_VAL(str));
}

static void psr_emit_constant(LoxParser * psr, LoxValue constant) {
    uint8_t constant_idx = psr_add_constant(psr, constant);
    psr_emit_bytes(psr, OP_CONST, constant_idx);
}


// parsing related helping procedures
static void psr_advance(LoxParser * psr) {
    psr->previous = psr->current;
    for(;;) {
        psr->current = sc_next_token(&psr->in);
        if(psr->current.type != TOKEN_ERROR)
            break;
        Token * curr = &psr->current;
        psr_error_at(psr, curr, curr->start);
    }
}

static inline bool psr_check(LoxParser * psr, TokenType tt) {
    return psr->current.type == tt;
}

static bool psr_match(LoxParser * psr, TokenType tt) {
    bool result;
    if((result = psr_check(psr, tt))) 
        psr_advance(psr);
    return result;
}

static void psr_consume(LoxParser * psr, TokenType tt, const char * msg) {
    if(!psr_match(psr, tt))
        psr_error_at(psr, &psr->current, msg);
}

static inline void psr_consume_semicolon(LoxParser * psr) {
    psr_consume(psr, TOKEN_SEMICOLON, "expected ';' at the end of the statement");
}

static void psr_parse_precedence(LoxParser * psr, ExprPrecedence prec) {
    psr_advance(psr);
    LoxParserRule * rule = get_parse_rule(psr->previous.type);
    if(rule->prefix == NULL) {
        psr_error_at(psr, &psr->previous, "expected expression");
        return;
    }

    psr->can_assign = prec <= PREC_ASSIGNMENT;
    rule->prefix(psr);

    while((rule = get_parse_rule(psr->current.type))->precedence >= prec) {
        ASSERTF(rule->infix != NULL, "BUG: rule->infix is NULL");
        psr_advance(psr);
        rule->infix(psr);
    }

    if(psr->can_assign && psr_match(psr, TOKEN_EQUAL)) {
        psr_error_at(psr, &psr->previous, "invalid assignment target");
    } 
}
static void psr_syncronize(LoxParser * psr) {
    psr->in_panic_mode = false;

    while (psr->current.type != TOKEN_EOF) {
        if (psr->previous.type == TOKEN_SEMICOLON) return;
        switch (psr->current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                ; // Do nothing.
        }

        psr_advance(psr);
    }
}

// scope helping functions
#define NO_LOCAL_VAR -1

static inline bool psr_in_global_scope(LoxParser * psr) {
    return psr->currentScope == 0;
}

static inline void psr_start_block(LoxParser * psr) {
    ASSERT(psr->currentScope < UINT32_MAX);
    psr->currentScope++;
}

static void psr_end_block(LoxParser * psr) {
    ASSERT(psr->currentScope > 0);

    // FIXME: use a OP_POP_N instruction
    ssize_t i;
    for(i = (ssize_t) psr->localsCount - 1; i >= 0 && psr->locals[i].scope == psr->currentScope; i--) {
        psr_emit_byte(psr, OP_POP);
    }

    psr->localsCount -= psr->localsCount - (i + 1);
    psr->currentScope--;
}

static ssize_t psr_find_local_var_in_scope(LoxParser * psr, Token * name, uint32_t scope) {
    for(ssize_t i = (ssize_t) psr->localsCount - 1; i >= 0 && psr->locals[i].scope >= scope; i--) {
        Token * current = &psr->locals[i].name;
        if(current->length == name->length && memcmp(current->start, name->start, current->length) == 0)
            return i;
    }
    return NO_LOCAL_VAR;
}

static inline size_t psr_find_local_var(LoxParser * psr, Token * name) {
    return psr_find_local_var_in_scope(psr, name, 0);
}

static void psr_alloc_local_var(LoxParser * psr, Token name) {
    ASSERT(psr->localsCount < UINT8_MAX);

    if(psr_find_local_var_in_scope(psr, &name, psr->currentScope) != NO_LOCAL_VAR) {
        psr_error_at(psr, &name, "variable already defined");
    } else if(psr->localsCount == UINT8_MAX) {
        psr_error_at(psr, &name, "stack overflow (reached max limit of local variables)");
    } else {
        LocalVar * new_var = &psr->locals[psr->localsCount++];
        new_var->name  = name;
        new_var->scope = psr->currentScope;
    }
}

// actually parsing stuffs
static void psr_parse_string(LoxParser * psr) {
    Token * token = &psr->previous;
    const char * chars = &token->start[1];
    size_t length = token->length - 2;

    psr_emit_bytes(psr, OP_CONST, psr_add_str_constant(psr, chars, length));
}

static void psr_parse_number(LoxParser * psr) {
    double value = strtod(psr->previous.start, NULL);
    psr_emit_constant(psr, NUMBER_VAL(value));
}


static void psr_parse_variable(LoxParser * psr) {
    ssize_t idx;
    bool is_global = false;

    Token * name = &psr->previous;
    if(psr_in_global_scope(psr) || (idx = psr_find_local_var(psr, name)) == NO_LOCAL_VAR) {
        idx = psr_add_str_constant(psr, name->start, name->length);
        is_global = true;
    }

    ASSERT(idx <= UINT8_MAX && idx >= 0);
    if(psr->can_assign && psr_match(psr, TOKEN_EQUAL)) {
        psr_parse_expression(psr);
        psr_emit_bytes(psr, is_global ? OP_SET_GLOBAL : OP_SET_LOCAL, (uint8_t) idx);
    } else {
        psr_emit_bytes(psr, is_global ? OP_GET_GLOBAL : OP_GET_LOCAL, (uint8_t) idx);
    }
}

static void psr_parse_primary(LoxParser * psr) {
    switch(psr->previous.type) {
        case TOKEN_TRUE  : psr_emit_byte(psr, OP_TRUE); break;
        case TOKEN_FALSE : psr_emit_byte(psr, OP_FALSE); break;
        case TOKEN_NIL   : psr_emit_byte(psr, OP_NIL); break;
        default: UNREACHABLE();
    }
}

static void psr_parse_unary(LoxParser * psr) {
    TokenType op = psr->previous.type;
    psr_parse_precedence(psr, PREC_UNARY);

    switch(op) {
        case TOKEN_MINUS: psr_emit_byte(psr, OP_NEG); break;
        case TOKEN_BANG : psr_emit_byte(psr, OP_NOT); break;
        default: UNREACHABLE();
    }
}

static void psr_parse_grouping(LoxParser * psr){
    psr_parse_expression(psr);
    psr_consume(psr, TOKEN_RIGHT_PAREN, "expected enclosing ')'");
}

static void psr_parse_binary(LoxParser * psr) {
    TokenType op = psr->previous.type;
    ExprPrecedence prec = get_parse_rule(op)->precedence;
    psr_parse_precedence(psr, prec + 1);

    switch(op) {
        case TOKEN_PLUS  : psr_emit_byte(psr, OP_ADD);  break;
        case TOKEN_MINUS : psr_emit_byte(psr, OP_SUB);  break;
        case TOKEN_STAR  : psr_emit_byte(psr, OP_MULT); break;
        case TOKEN_SLASH : psr_emit_byte(psr, OP_DIV);  break;
        default: UNREACHABLE();
    }
}

static void psr_parse_comparison(LoxParser * psr) {
    TokenType op = psr->previous.type;
    ExprPrecedence prec = get_parse_rule(op)->precedence;
    psr_parse_precedence(psr, prec + 1);

    switch(op) {
        case TOKEN_EQUAL_EQUAL   : psr_emit_byte(psr, OP_EQ); break;
        case TOKEN_BANG_EQUAL    : psr_emit_bytes(psr, OP_EQ, OP_NOT); break;
        case TOKEN_GREATER       : psr_emit_byte(psr, OP_GREATER); break;
        case TOKEN_LESS          : psr_emit_byte(psr, OP_LESS); break;
        case TOKEN_GREATER_EQUAL : psr_emit_bytes(psr, OP_LESS, OP_NOT); break;
        case TOKEN_LESS_EQUAL    : psr_emit_bytes(psr, OP_GREATER, OP_NOT); break;
        default: UNREACHABLE();
    }
}

static void psr_parse_expression(LoxParser * psr){
    psr_parse_precedence(psr, PREC_ASSIGNMENT);
}



static void psr_parse_statement(LoxParser * psr) {
    if(psr_match(psr, TOKEN_PRINT)) {
        psr_parse_expression(psr);
        psr_emit_byte(psr, OP_PRINT);
        psr_consume_semicolon(psr);
    } else if(psr_match(psr, TOKEN_LEFT_BRACE)) {
        psr_start_block(psr);
        while(!(psr_check(psr, TOKEN_RIGHT_BRACE) || psr_check(psr, TOKEN_EOF))){
            psr_parse_declaration(psr);
        }
        psr_consume(psr, TOKEN_RIGHT_BRACE, "missing enclosing '}'");
        psr_end_block(psr);
    } else {
        psr_parse_expression(psr);
        psr_emit_byte(psr, OP_POP);
        psr_consume_semicolon(psr);
    }
}

static void psr_parse_declaration(LoxParser * psr) {
    if(psr_match(psr, TOKEN_VAR)) {
        psr_consume(psr, TOKEN_IDENTIFIER, "expected an identifier after 'var' keyword");

        Token name = psr->previous;
        if(psr_match(psr, TOKEN_EQUAL))
            psr_parse_expression(psr);
        else
            psr_emit_byte(psr, OP_NIL);

        if(psr_in_global_scope(psr)) {
            uint8_t idx = psr_add_str_constant(psr, name.start, name.length);
            psr_emit_bytes(psr, OP_DEFINE_GLOBAL, idx);
        } else {
            psr_alloc_local_var(psr, name);
        }

        psr_consume_semicolon(psr);
    } else {
        psr_parse_statement(psr);
    }


    if(psr->in_panic_mode) psr_syncronize(psr);
}

static void psr_destroy(LoxParser * psr) {
    sc_destroy(&psr->in);

    psr->strings  = NULL;
    psr->out_prog = NULL;
    memset(&psr->previous, 0, sizeof(Token));
    memset(&psr->current, 0, sizeof(Token));

    psr->error_found   = false;
    psr->in_panic_mode = false;
}

static LoxParserRule rules[] = {
    [TOKEN_LEFT_PAREN]    = { psr_parse_grouping, NULL, PREC_PRIMARY },
    [TOKEN_RIGHT_PAREN]   = { NULL, NULL, PREC_NONE },
    [TOKEN_LEFT_BRACE]    = { NULL, NULL, PREC_NONE },
    [TOKEN_RIGHT_BRACE]   = { NULL, NULL, PREC_NONE },
    [TOKEN_COMMA]         = { NULL, NULL, PREC_NONE },
    [TOKEN_DOT]           = { NULL, NULL, PREC_NONE },
    [TOKEN_MINUS]         = { psr_parse_unary, psr_parse_binary, PREC_TERM },
    [TOKEN_PLUS]          = { NULL, psr_parse_binary, PREC_TERM },
    [TOKEN_SEMICOLON]     = { NULL, NULL, PREC_NONE },
    [TOKEN_SLASH]         = { NULL, psr_parse_binary, PREC_FACTOR },
    [TOKEN_STAR]          = { NULL, psr_parse_binary, PREC_FACTOR },

    // One or two character tokens.
    [TOKEN_BANG]          = { psr_parse_unary, NULL, PREC_UNARY },
    [TOKEN_BANG_EQUAL]    = { psr_parse_comparison, NULL, PREC_EQUALITY   },
    [TOKEN_EQUAL]         = { NULL, NULL, PREC_NONE },
    [TOKEN_EQUAL_EQUAL]   = { NULL, psr_parse_comparison, PREC_EQUALITY   },
    [TOKEN_GREATER]       = { NULL, psr_parse_comparison, PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL] = { NULL, psr_parse_comparison, PREC_COMPARISON },
    [TOKEN_LESS]          = { NULL, psr_parse_comparison, PREC_COMPARISON },
    [TOKEN_LESS_EQUAL]    = { NULL, psr_parse_comparison, PREC_COMPARISON },

    // Literals.
    [TOKEN_IDENTIFIER]    = { psr_parse_variable, NULL, PREC_NONE },
    [TOKEN_STRING]        = { psr_parse_string, NULL, PREC_NONE },
    [TOKEN_NUMBER]        = { psr_parse_number, NULL, PREC_NONE },

    // Keywords.
    [TOKEN_NIL]           = { psr_parse_primary, NULL, PREC_PRIMARY },
    [TOKEN_TRUE]          = { psr_parse_primary, NULL, PREC_PRIMARY },
    [TOKEN_FALSE]         = { psr_parse_primary, NULL, PREC_PRIMARY },

    [TOKEN_AND]           = { NULL, NULL, PREC_NONE },
    [TOKEN_CLASS]         = { NULL, NULL, PREC_NONE },
    [TOKEN_ELSE]          = { NULL, NULL, PREC_NONE },
    [TOKEN_FOR]           = { NULL, NULL, PREC_NONE },
    [TOKEN_FUN]           = { NULL, NULL, PREC_NONE },
    [TOKEN_IF]            = { NULL, NULL, PREC_NONE },
    [TOKEN_OR]            = { NULL, NULL, PREC_NONE },
    [TOKEN_PRINT]         = { NULL, NULL, PREC_NONE },
    [TOKEN_RETURN]        = { NULL, NULL, PREC_NONE },
    [TOKEN_SUPER]         = { NULL, NULL, PREC_NONE },
    [TOKEN_THIS]          = { NULL, NULL, PREC_NONE },
    [TOKEN_VAR]           = { NULL, NULL, PREC_NONE },
    [TOKEN_WHILE]         = { NULL, NULL, PREC_NONE },
    [TOKEN_ERROR]         = { NULL, NULL, PREC_NONE },
    [TOKEN_EOF]           = { NULL, NULL, PREC_NONE },
};

static LoxParserRule * get_parse_rule(TokenType tt){ 
    ASSERTF(tt < sizeof(rules) / sizeof(LoxParserRule), "invalid token type");
    return &rules[tt];
}

static void psr_parse_program(LoxParser * psr) {
    psr_advance(psr);
    while(!psr_match(psr, TOKEN_EOF)){
        psr_parse_declaration(psr);
    }
    psr_emit_byte(psr, OP_RETURN);
}

bool compile(const char * source, LoxProgram * out_prog, HashMap * strings) {
    LoxParser psr;
    psr_init(&psr, source, out_prog, strings);
    psr_parse_program(&psr);
    bool no_error = !psr.error_found;
    psr_destroy(&psr);
    return no_error;
}
