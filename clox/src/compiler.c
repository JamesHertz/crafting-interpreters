#include "compiler.h"
#include "program.h"
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef struct {
    LoxScanner in;
    LoxProgram * out_prog;
    HashMap * strings;

    Token previous;
    Token current;

    bool error_found;
    bool in_panic_mode;
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

static void psr_init(LoxParser * psr, const char * source, LoxProgram * out_prog, HashMap * strings) {
    sc_init(&psr->in, source);
    psr->out_prog = out_prog;
    psr->strings  = strings;
    memset(&psr->previous, 0, sizeof(Token));
    memset(&psr->current, 0, sizeof(Token));

    psr->error_found   = false;
    psr->in_panic_mode = false;
}

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

static void psr_consume(LoxParser * psr, TokenType tt, const char * msg) {
    if(psr->current.type == tt)
        psr_advance(psr);
    else
        psr_error_at(psr, &psr->current, msg);
}

static void psr_parse_precedence(LoxParser * psr, ExprPrecedence prec) {
    psr_advance(psr);

    /*printf("psr_parse_precedence(): prec = %d, prev = %s, curr = %s\n", prec, tt2str(psr->previous.type), tt2str(psr->current.type));*/
    LoxParserRule * rule = get_parse_rule(psr->previous.type);
    if(rule->prefix == NULL) {
        psr_error_at(psr, &psr->previous, "expected expression");
        return;
    }

    rule->prefix(psr);

    while((rule = get_parse_rule(psr->current.type))->precedence >= prec) {
        /*printf("rule-precedence: %d, global-rec: %d\n", rule->precedence, prec);*/
        assert(rule->infix != NULL && "psr_parse_precedence(): rule->infix is NULL");
        psr_advance(psr);
        rule->infix(psr);
    }
}

static void psr_parse_expression(LoxParser * psr){
    psr_parse_precedence(psr, PREC_ASSIGNMENT);
}

static void psr_emit_byte(LoxParser * psr, uint8_t byte) {
    prog_add_instr(psr->out_prog, byte, psr->previous.line);
}

static void psr_emit_bytes(LoxParser * psr, uint8_t byte1, uint8_t byte2) {
    psr_emit_byte(psr, byte1);
    psr_emit_byte(psr, byte2);
}

static void psr_destroy(LoxParser * psr) {
    sc_destroy(&psr->in);

    psr->out_prog = NULL;
    memset(&psr->previous, 0, sizeof(Token));
    memset(&psr->current, 0, sizeof(Token));

    psr->error_found   = false;
    psr->in_panic_mode = false;
}

static void psr_emit_constant(LoxParser * psr, LoxValue constant) {
    size_t constant_idx = prog_add_constant(psr->out_prog, constant);
    if(constant_idx > UINT16_MAX) {
        psr_error_at(psr, &psr->previous, "too many constants for current chunk (?)");
        constant_idx = 0;
    }
    psr_emit_bytes(psr, OP_CONST, (uint8_t) constant_idx);
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
        default: assert(0 && "psr_parse_binary(): unexpected token");
    }
}

static void psr_parse_comparison(LoxParser * psr) {
    TokenType op = psr->previous.type;
    ExprPrecedence prec = get_parse_rule(op)->precedence;
    psr_parse_precedence(psr, prec + 1);

    switch(op) {
        case TOKEN_EQUAL_EQUAL: psr_emit_byte(psr, OP_EQ); break;
        case TOKEN_BANG_EQUAL : psr_emit_bytes(psr, OP_EQ, OP_NOT); break;
        case TOKEN_GREATER    : psr_emit_byte(psr, OP_GREATER); break;
        case TOKEN_LESS       : psr_emit_byte(psr, OP_LESS); break;
        case TOKEN_GREATER_EQUAL : psr_emit_bytes(psr, OP_LESS, OP_NOT); break;
        case TOKEN_LESS_EQUAL    : psr_emit_bytes(psr, OP_GREATER, OP_NOT); break;
        default: assert(0 && "psr_parse_comparison(): unexpected token");
    }
}

static void psr_parse_unary(LoxParser * psr) {
    TokenType op = psr->previous.type;
    psr_parse_precedence(psr, PREC_UNARY);

    switch(op) {
        case TOKEN_MINUS: psr_emit_byte(psr, OP_NEG); break;
        case TOKEN_BANG : psr_emit_byte(psr, OP_NOT); break;
        default: assert(0 && "psr_parse_unary(): unexpected token");
    }
}

static void psr_parse_primary(LoxParser * psr) {
    switch(psr->previous.type) {
        case TOKEN_TRUE  : psr_emit_byte(psr, OP_TRUE); break;
        case TOKEN_FALSE : psr_emit_byte(psr, OP_FALSE); break;
        case TOKEN_NIL   : psr_emit_byte(psr, OP_NIL); break;
        default: assert(0 && "psr_parse_primary(): unexpected token");
    }
}

static void psr_parse_string(LoxParser * psr) {
    Token * token = &psr->previous;

    const char * chars = &token->start[1];
    size_t length = token->length - 2;
    uint32_t hash = str_hash(chars, length);

    const LoxString * str;
    if((str = map_find_str(psr->strings, chars, length, hash)) == NULL){
        str = lox_str_copy(chars, length, hash);
        map_set(psr->strings, str, BOOL_VAL(true));
    }

    psr_emit_constant(psr, OBJ_VAL(str));
}

static void psr_parse_number(LoxParser * psr) {
    double value = strtod(psr->previous.start, NULL);
    psr_emit_constant(psr, NUMBER_VAL(value));
}

static void psr_parse_grouping(LoxParser * psr){
    psr_parse_expression(psr);
    psr_consume(psr, TOKEN_RIGHT_PAREN, "expected enclosing ')'");
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
    [TOKEN_BANG_EQUAL]    = { psr_parse_comparison, NULL, PREC_EQUALITY },
    [TOKEN_EQUAL]         = { NULL, NULL, PREC_NONE },
    [TOKEN_EQUAL_EQUAL]   = { NULL, psr_parse_comparison,  PREC_EQUALITY},
    [TOKEN_GREATER]       = { NULL, psr_parse_comparison, PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL] = { NULL, psr_parse_comparison, PREC_COMPARISON },
    [TOKEN_LESS]          = { NULL, psr_parse_comparison, PREC_COMPARISON },
    [TOKEN_LESS_EQUAL]    = { NULL, psr_parse_comparison, PREC_COMPARISON },

    // Literals.
    [TOKEN_IDENTIFIER]    = { NULL, NULL, PREC_NONE },
    [TOKEN_STRING]        = { psr_parse_string, NULL, PREC_PRIMARY },
    [TOKEN_NUMBER]        = { psr_parse_number, NULL, PREC_PRIMARY },

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
    assert(tt < sizeof(rules) / sizeof(LoxParserRule) && "get_parse_rule(): invalid token type");
    return &rules[tt];
}

bool compile(const char * source, LoxProgram * out_prog, HashMap * strings) {
    LoxParser psr;
    psr_init(&psr, source, out_prog, strings);

    psr_advance(&psr);
    psr_parse_expression(&psr);
    psr_consume(&psr, TOKEN_EOF, "expected end of file");
    psr_emit_byte(&psr, OP_RETURN);

    bool no_error = !psr.error_found;
    psr_destroy(&psr);
    return no_error;
}
