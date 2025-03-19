#include "compiler.h"
#include "chunk.h"
#include "scanner.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "constants.h"

typedef struct {
    Token name;
    uint32_t scope;
} LocalVar;

typedef struct {
    LoxScanner in;
    HashMap * strings;

    LoxFunction * script;

    Token previous;
    Token current;

    // TODO: make this one var
    bool error_found;
    bool in_panic_mode;
    bool can_assign;

    LocalVar locals[MAX_LOCALS * MAX_STACK_FRAMES];
    uint32_t localsCount;
    uint32_t currentScope;
    uint32_t funcLocalsStart;
} LoxSPCompiler; // stands for LoxSinglePassCompiler

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

typedef void (*ParserFunc)(LoxSPCompiler *);

typedef struct {
    ParserFunc   prefix;
    ParserFunc   infix;
    ExprPrecedence precedence;
} LoxParserRule;

static LoxParserRule * get_parse_rule(TokenType tt);
static void cpl_compile_expression(LoxSPCompiler * cpl);
static void cpl_compile_declaration(LoxSPCompiler * cpl);

static void cpl_init(LoxSPCompiler * cpl, const char * source, HashMap * strings) {
    sc_init(&cpl->in, source);
    cpl->strings  = strings;
    cpl->previous = cpl->current = (Token) {0};

    cpl->error_found   = false;
    cpl->in_panic_mode = false;
    cpl->can_assign    = false;

    // locals things
    cpl->localsCount     = 0;
    cpl->currentScope    = 0;
    cpl->funcLocalsStart = 0;
    cpl->script          = lox_func_create(NULL, FUNC_SCRIPT);
}

static inline LoxChunk * cpl_chunk(LoxSPCompiler * cpl) {
    return &cpl->script->chunk;
}

// helper functions
static void cpl_error_at(LoxSPCompiler * cpl, Token * pos, const char * msg) {
    if(cpl->in_panic_mode) return;

    fprintf(stderr, "[line %d] Error", pos->line);
    if(pos->type == TOKEN_EOF)
        fprintf(stderr, " at the end"); 
    else if(pos->type != TOKEN_ERROR)
        fprintf(stderr, " at '%.*s'", (int) pos->length, pos->start); 

    fprintf(stderr, ": %s\n", msg);
    cpl->in_panic_mode = true;
    cpl->error_found   = true;
}

static void cpl_emit_byte(LoxSPCompiler * cpl, uint8_t byte) {
    chunk_add_instr(cpl_chunk(cpl), byte, cpl->previous.line);
}

static void cpl_emit_bytes(LoxSPCompiler * cpl, uint8_t byte1, uint8_t byte2) {
    cpl_emit_byte(cpl, byte1);
    cpl_emit_byte(cpl, byte2);
}

static uint8_t cpl_add_constant(LoxSPCompiler * cpl, LoxValue constant) {
    size_t constant_idx = chunk_add_constant(cpl_chunk(cpl), constant);
    if(constant_idx > UINT8_MAX) {
        cpl_error_at(cpl, &cpl->previous, "too many constants for current chunk (?)");
        constant_idx = 0;
    }
    return constant_idx;
}

static const LoxString * cpl_intern_str(LoxSPCompiler * cpl, const char * chars, size_t length) {
    uint32_t hash = str_hash(chars, length);
    const LoxString * str;
    if((str = map_find_str(cpl->strings, chars, length, hash)) == NULL){
        str = lox_str_copy(chars, length, hash);
        map_set(cpl->strings, str, BOOL_VAL(true));
    }
    return str;
}

static inline uint8_t cpl_add_str_constant(LoxSPCompiler * cpl, const char * chars, size_t length) {
    return cpl_add_constant(cpl, OBJ_VAL(cpl_intern_str(cpl, chars, length)));
}

static void cpl_emit_constant(LoxSPCompiler * cpl, LoxValue constant) {
    uint8_t constant_idx = cpl_add_constant(cpl, constant);
    cpl_emit_bytes(cpl, OP_CONST, constant_idx);
}

// parsing related helping procedures
static void cpl_advance(LoxSPCompiler * cpl) {
    cpl->previous = cpl->current;
    for(;;) {
        cpl->current = sc_next_token(&cpl->in);
        if(cpl->current.type != TOKEN_ERROR)
            break;
        Token * curr = &cpl->current;
        cpl_error_at(cpl, curr, curr->start);
    }
}

static inline bool cpl_check(LoxSPCompiler * cpl, TokenType tt) {
    return cpl->current.type == tt;
}

static bool cpl_match(LoxSPCompiler * cpl, TokenType tt) {
    bool result;
    if((result = cpl_check(cpl, tt))) 
        cpl_advance(cpl);
    return result;
}

static void cpl_consume(LoxSPCompiler * cpl, TokenType tt, const char * msg) {
    if(!cpl_match(cpl, tt))
        cpl_error_at(cpl, &cpl->current, msg);
}

static inline void cpl_consume_semicolon(LoxSPCompiler * cpl) {
    cpl_consume(cpl, TOKEN_SEMICOLON, "expected ';' at the end of the statement");
}

static void cpl_parse_precedence(LoxSPCompiler * cpl, ExprPrecedence prec) {
    cpl_advance(cpl);
    LoxParserRule * rule = get_parse_rule(cpl->previous.type);
    if(rule->prefix == NULL) {
        cpl_error_at(cpl, &cpl->previous, "expected expression");
        return;
    }

    cpl->can_assign = prec <= PREC_ASSIGNMENT;
    rule->prefix(cpl);

    while((rule = get_parse_rule(cpl->current.type))->precedence >= prec) {
        ASSERTF(rule->infix != NULL, "BUG: rule->infix is NULL (token = %s)", tt2str(cpl->current.type));
        cpl_advance(cpl);
        rule->infix(cpl);
    }

    if(!cpl->can_assign && cpl_match(cpl, TOKEN_EQUAL)) {
        cpl_error_at(cpl, &cpl->previous, "invalid assignment target");
    } 
}
static void cpl_syncronize(LoxSPCompiler * cpl) {
    cpl->in_panic_mode = false;

    while (cpl->current.type != TOKEN_EOF) {
        if (cpl->previous.type == TOKEN_SEMICOLON) return;
        switch (cpl->current.type) {
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

        cpl_advance(cpl);
    }
}

// scope helping functions
#define NO_LOCAL_VAR -1

static inline bool cpl_in_global_scope(LoxSPCompiler * cpl) {
    return cpl->currentScope == 0;
}

static inline void cpl_begin_scope(LoxSPCompiler * cpl) {
    ASSERT(cpl->currentScope < UINT32_MAX);
    cpl->currentScope++;
}

static void cpl_end_scope(LoxSPCompiler * cpl, bool of_func) {
    ASSERT(cpl->currentScope > 0);

    // FIXME: use a OP_POP_N instruction
    ssize_t i;
    for(i = (ssize_t) cpl->localsCount - 1; i >= 0 && cpl->locals[i].scope == cpl->currentScope; i--) {
        if(!of_func) cpl_emit_byte(cpl, OP_POP);
    }

    cpl->localsCount -= cpl->localsCount - (i + 1);
    cpl->currentScope--;
}

static uint32_t cpl_begin_func(LoxSPCompiler * cpl) {
    if(cpl->script->type != FUNC_SCRIPT) 
        cpl_begin_scope(cpl);

    uint32_t localsStart = cpl->funcLocalsStart;
    cpl->funcLocalsStart = cpl->localsCount; // reserving space for function itself

    // reserving place for function
    cpl->locals[cpl->localsCount++] = (LocalVar) {
        .scope = cpl->currentScope,
        .name = (Token) {
            .start  = "",
            .length = 0,
            .line   = 0,
            .type   = TOKEN_IDENTIFIER,
        }
    };
    return localsStart;
}

static void cpl_end_func(LoxSPCompiler * cpl, uint32_t prevLocalsStart) {
    if(cpl->script->type != FUNC_SCRIPT) 
        cpl_end_scope(cpl, true);
    cpl->funcLocalsStart = prevLocalsStart;
}

static ssize_t cpl_find_local_var_in_scope(LoxSPCompiler * cpl, Token * name, uint32_t scope) {
    for(ssize_t i = (ssize_t) cpl->localsCount - 1; i >= 0 && cpl->locals[i].scope >= scope; i--) {
        Token * current = &cpl->locals[i].name;
        if(current->length == name->length && memcmp(current->start, name->start, current->length) == 0)
            return i - cpl->funcLocalsStart;
    }
    return NO_LOCAL_VAR;
}

static inline size_t cpl_find_local_var(LoxSPCompiler * cpl, Token * name) {
    return cpl_find_local_var_in_scope(cpl, name, 0);
}

static void cpl_alloc_local_var(LoxSPCompiler * cpl, Token name) {
    ASSERT(cpl->localsCount < UINT8_MAX);

    if(cpl_find_local_var_in_scope(cpl, &name, cpl->currentScope) != NO_LOCAL_VAR) {
        cpl_error_at(cpl, &name, "variable already defined");
    } else if(cpl->localsCount == UINT8_MAX) {
        cpl_error_at(cpl, &name, "stack overflow (reached max limit of local variables)");
    } else {
        LocalVar * new_var = &cpl->locals[cpl->localsCount++];
        new_var->name  = name;
        new_var->scope = cpl->currentScope;
    }
}

static inline size_t cpl_current_offset(LoxSPCompiler * cpl) {
    return cpl_chunk(cpl)->code.length;
}

static void cpl_write_jump_length(LoxSPCompiler * cpl, size_t offset, size_t length) {
    if(length > UINT16_MAX) {
        cpl_error_at(cpl, &cpl->previous, "jump length larger than 65535");
    } else {
        da_get_ptr(&cpl_chunk(cpl)->code, offset - 1)->op_code = length >> 8 & 0xFF;
        da_get_ptr(&cpl_chunk(cpl)->code, offset - 2)->op_code = length & 0xFF;
    }
}

static size_t cpl_emit_jump(LoxSPCompiler * cpl, uint8_t jump_instr) {
    cpl_emit_byte(cpl, jump_instr);
    cpl_emit_bytes(cpl, 0, 0);
    return cpl_chunk(cpl)->code.length;
}

static void cpl_emit_loop(LoxSPCompiler * cpl, size_t target_instr_offset) {
    cpl_emit_jump(cpl, OP_LOOP);
    size_t length = cpl_chunk(cpl)->code.length - target_instr_offset;
    cpl_write_jump_length(cpl, cpl_chunk(cpl)->code.length, length);
}

static inline void cpl_complete_jump(LoxSPCompiler * cpl, size_t jump_op_offset) {
    size_t length = cpl_chunk(cpl)->code.length - jump_op_offset;
    cpl_write_jump_length(cpl, jump_op_offset, length);
}

// actually parsing stuffs
static void cpl_compile_string(LoxSPCompiler * cpl) {
    Token * token = &cpl->previous;
    const char * chars = &token->start[1];
    size_t length = token->length - 2;

    cpl_emit_bytes(cpl, OP_CONST, cpl_add_str_constant(cpl, chars, length));
}

static void cpl_compile_number(LoxSPCompiler * cpl) {
    double value = strtod(cpl->previous.start, NULL);
    cpl_emit_constant(cpl, NUMBER_VAL(value));
}

static void cpl_compile_variable(LoxSPCompiler * cpl) {
    ssize_t idx;
    bool is_global = false;

    Token * name = &cpl->previous;
    if(cpl_in_global_scope(cpl) || (idx = cpl_find_local_var(cpl, name)) == NO_LOCAL_VAR) {
        idx = cpl_add_str_constant(cpl, name->start, name->length);
        is_global = true;
    }

    ASSERT(idx <= UINT8_MAX && idx >= 0);
    if(cpl->can_assign && cpl_match(cpl, TOKEN_EQUAL)) {
        cpl_compile_expression(cpl);
        cpl_emit_bytes(cpl, is_global ? OP_SET_GLOBAL : OP_SET_LOCAL, (uint8_t) idx);
    } else {
        cpl_emit_bytes(cpl, is_global ? OP_GET_GLOBAL : OP_GET_LOCAL, (uint8_t) idx);
    }
}

static void cpl_compile_call(LoxSPCompiler * cpl) {
    size_t args_nr = 0;

    if(!cpl_match(cpl, TOKEN_RIGHT_PAREN)) {
        do {
            cpl_compile_expression(cpl);
            args_nr++;
        } while(cpl_match(cpl, TOKEN_COMMA));
        cpl_consume(cpl, TOKEN_RIGHT_PAREN, "expected ')' after function arguments");
    }

    if(args_nr > MAX_ARGS) {
        cpl_error_at(cpl, &cpl->previous, "exceed limited of function arguments (256)");
    } else {
        cpl_emit_bytes(cpl, OP_CALL, (uint8_t) args_nr);
    }
}

static void cpl_compile_primary(LoxSPCompiler * cpl) {
    switch(cpl->previous.type) {
        case TOKEN_TRUE  : cpl_emit_byte(cpl, OP_TRUE); break;
        case TOKEN_FALSE : cpl_emit_byte(cpl, OP_FALSE); break;
        case TOKEN_NIL   : cpl_emit_byte(cpl, OP_NIL); break;
        default: UNREACHABLE();
    }
}

static void cpl_compile_unary(LoxSPCompiler * cpl) {
    TokenType op = cpl->previous.type;
    cpl_parse_precedence(cpl, PREC_UNARY);

    switch(op) {
        case TOKEN_MINUS: cpl_emit_byte(cpl, OP_NEG); break;
        case TOKEN_BANG : cpl_emit_byte(cpl, OP_NOT); break;
        default: UNREACHABLE();
    }
}

static void cpl_compile_grouping(LoxSPCompiler * cpl){
    cpl_compile_expression(cpl);
    cpl_consume(cpl, TOKEN_RIGHT_PAREN, "expected enclosing ')'");
}

static void cpl_compile_binary(LoxSPCompiler * cpl) {
    TokenType op = cpl->previous.type;
    ExprPrecedence prec = get_parse_rule(op)->precedence;

    if(prec != PREC_AND && prec != PREC_OR)
        cpl_parse_precedence(cpl, prec + 1);

    switch(op) {
        // arithmetic
        case TOKEN_PLUS  : cpl_emit_byte(cpl, OP_ADD);  break;
        case TOKEN_MINUS : cpl_emit_byte(cpl, OP_SUB);  break;
        case TOKEN_STAR  : cpl_emit_byte(cpl, OP_MULT); break;
        case TOKEN_SLASH : cpl_emit_byte(cpl, OP_DIV);  break;

        // and and or
        case TOKEN_AND   : {
            size_t offset = cpl_emit_jump(cpl, OP_IF_FALSE);
            cpl_emit_byte(cpl, OP_POP);
            cpl_parse_precedence(cpl, prec + 1);
            cpl_complete_jump(cpl, offset);
        } break;
        case TOKEN_OR    : {
            size_t false_offset = cpl_emit_jump(cpl, OP_IF_FALSE);
            size_t true_offset  = cpl_emit_jump(cpl, OP_JUMP);
            cpl_complete_jump(cpl, false_offset);
            cpl_emit_byte(cpl, OP_POP);
            cpl_parse_precedence(cpl, prec + 1);
            cpl_complete_jump(cpl, true_offset);
        } break;

        // comparison
        case TOKEN_EQUAL_EQUAL   : cpl_emit_byte(cpl, OP_EQ);                       break;
        case TOKEN_BANG_EQUAL    : cpl_emit_bytes(cpl, OP_EQ, OP_NOT);      break;
        case TOKEN_GREATER       : cpl_emit_byte(cpl, OP_GREATER);                  break;
        case TOKEN_LESS          : cpl_emit_byte(cpl, OP_LESS);                     break;
        case TOKEN_GREATER_EQUAL : cpl_emit_bytes(cpl, OP_LESS, OP_NOT);    break;
        case TOKEN_LESS_EQUAL    : cpl_emit_bytes(cpl, OP_GREATER, OP_NOT); break;

        default: UNREACHABLE();
    }
}

static void cpl_compile_expression(LoxSPCompiler * cpl){
    cpl_parse_precedence(cpl, PREC_ASSIGNMENT);
}

static void cpl_define_var(LoxSPCompiler * cpl, Token name) {
    if(cpl_in_global_scope(cpl)) {
        uint8_t idx = cpl_add_str_constant(cpl, name.start, name.length);
        cpl_emit_bytes(cpl, OP_DEFINE_GLOBAL, idx);
    } else {
        cpl_alloc_local_var(cpl, name);
    }
}

static void cpl_compile_var_declaration(LoxSPCompiler * cpl) {
    cpl_consume(cpl, TOKEN_IDENTIFIER, "expected an identifier after 'var' keyword");

    Token name = cpl->previous;
    if(cpl_match(cpl, TOKEN_EQUAL))
        cpl_compile_expression(cpl);
    else
        cpl_emit_byte(cpl, OP_NIL);

    cpl_define_var(cpl, name);
}

static void cpl_compile_statement(LoxSPCompiler * cpl) {
    if(cpl_match(cpl, TOKEN_PRINT)) { // print statement
        cpl_compile_expression(cpl);
        cpl_emit_byte(cpl, OP_PRINT);
        cpl_consume_semicolon(cpl);
    } else if(cpl_match(cpl, TOKEN_LEFT_BRACE)) { // block statement
        cpl_begin_scope(cpl);
            while(!(cpl_check(cpl, TOKEN_RIGHT_BRACE) || cpl_check(cpl, TOKEN_EOF))){
                cpl_compile_declaration(cpl);
            }
            cpl_consume(cpl, TOKEN_RIGHT_BRACE, "missing enclosing '}'");
        cpl_end_scope(cpl, false);
    } else if(cpl_match(cpl, TOKEN_IF)) { // if statement
        cpl_consume(cpl, TOKEN_LEFT_PAREN, "expected '(' after if keyword");
        cpl_compile_expression(cpl);
        cpl_consume(cpl, TOKEN_RIGHT_PAREN, "expected ')' after if expression");

        size_t if_jump_offset = cpl_emit_jump(cpl, OP_IF_FALSE);
        cpl_emit_byte(cpl, OP_POP);
        cpl_compile_statement(cpl);

        if(cpl_match(cpl, TOKEN_ELSE)) {
            size_t else_jump_offset = cpl_emit_jump(cpl, OP_JUMP);
            cpl_complete_jump(cpl, if_jump_offset);
            cpl_emit_byte(cpl, OP_POP);
            cpl_compile_statement(cpl);
            cpl_complete_jump(cpl, else_jump_offset);
        } else {
            size_t silly_jump = cpl_emit_jump(cpl, OP_JUMP);
            cpl_complete_jump(cpl, if_jump_offset);
            cpl_emit_byte(cpl, OP_POP);
            cpl_complete_jump(cpl, silly_jump);
        }

    } else if(cpl_match(cpl, TOKEN_FOR)) { // for statement
        cpl_consume(cpl, TOKEN_LEFT_PAREN, "expected '(' after for keyword");

        cpl_begin_scope(cpl);

            if(!cpl_match(cpl, TOKEN_SEMICOLON)) {
                if(cpl_match(cpl, TOKEN_VAR))
                    cpl_compile_var_declaration(cpl);
                else {
                    cpl_compile_expression(cpl);
                    cpl_emit_byte(cpl, OP_POP);
                }
                cpl_consume(cpl, TOKEN_SEMICOLON, "expected ';' after for loop initialization");
            }

            size_t cond_offset = cpl_current_offset(cpl);
            if(!cpl_match(cpl, TOKEN_SEMICOLON)) {
                cpl_compile_expression(cpl);
                cpl_consume(cpl, TOKEN_SEMICOLON, "expected ';' after for loop expression");
            } else {
                cpl_emit_byte(cpl, OP_TRUE);
            }

            size_t jump_to_end_offset  = cpl_emit_jump(cpl, OP_IF_FALSE);
            cpl_emit_byte(cpl, OP_POP);
            size_t jump_to_body_offset = cpl_emit_jump(cpl, OP_JUMP);

            size_t end_offset = cpl_current_offset(cpl);
            if(!cpl_match(cpl, TOKEN_RIGHT_PAREN)) {
                cpl_compile_expression(cpl);
                cpl_emit_byte(cpl, OP_POP);
                cpl_consume(cpl, TOKEN_RIGHT_PAREN, "expected ')' before for loop body");
            }
            cpl_emit_loop(cpl, cond_offset);

            cpl_complete_jump(cpl, jump_to_body_offset);
            cpl_compile_statement(cpl);
            cpl_emit_loop(cpl, end_offset);

            cpl_complete_jump(cpl, jump_to_end_offset);
            cpl_emit_byte(cpl, OP_POP); // for `for's condition`

        cpl_end_scope(cpl, false);
    } else if(cpl_match(cpl, TOKEN_WHILE)) { // while statement
        size_t while_expr_offset = cpl_current_offset(cpl);

        cpl_consume(cpl, TOKEN_LEFT_PAREN, "expected '(' after while keyword");
        cpl_compile_expression(cpl);
        cpl_consume(cpl, TOKEN_RIGHT_PAREN, "expected ')' after while expression");

        size_t while_jump_offset = cpl_emit_jump(cpl, OP_IF_FALSE);
        cpl_emit_byte(cpl, OP_POP);
        cpl_compile_statement(cpl);
        cpl_emit_loop(cpl, while_expr_offset);

        cpl_complete_jump(cpl, while_jump_offset);
        cpl_emit_byte(cpl, OP_POP);
    } else if (cpl_match(cpl, TOKEN_RETURN)) {
        if(cpl->script->type == FUNC_SCRIPT)
            cpl_error_at(cpl, &cpl->previous, "cannot return from the top level");
        else if(cpl_match(cpl, TOKEN_SEMICOLON))
            cpl_emit_bytes(cpl, OP_NIL, OP_RETURN);
        else {
            cpl_compile_expression(cpl);
            cpl_emit_byte(cpl, OP_RETURN);
            cpl_consume_semicolon(cpl);
        }
    } else { // expression statement
        cpl_compile_expression(cpl);
        cpl_emit_byte(cpl, OP_POP);
        cpl_consume_semicolon(cpl);
    }
}

static void cpl_compile_function_body(LoxSPCompiler * cpl, const LoxString * func_name, LoxFuncType type) {
    LoxFunction * func     = lox_func_create(func_name, type);
    cpl_emit_bytes(cpl, OP_CONST, cpl_add_constant(cpl, OBJ_VAL(func)));

    if(type == FUNC_ORDINARY) 
        cpl_define_var(cpl, cpl->previous);

    LoxFunction * backup   = cpl->script;
    cpl->script = func;
    uint32_t lastLocalsStart = cpl_begin_func(cpl);
    cpl_consume(cpl, TOKEN_LEFT_PAREN, "expected '(' before function parameters");
    if(!cpl_match(cpl, TOKEN_RIGHT_PAREN)) {
        do {
            cpl_consume(cpl, TOKEN_IDENTIFIER, "expected indentifier for function arguments");
            cpl_alloc_local_var(cpl, cpl->previous);
            func->arity++;
        } while(cpl_match(cpl, TOKEN_COMMA));
        cpl_consume(cpl, TOKEN_RIGHT_PAREN, "expected ')' after function parameter(s)");
    }
    cpl_consume(cpl, TOKEN_LEFT_BRACE, "expected '{' before function body");
    while(!(cpl_check(cpl, TOKEN_RIGHT_BRACE) || cpl_check(cpl, TOKEN_EOF))){
        cpl_compile_declaration(cpl);
    }
    cpl_consume(cpl, TOKEN_RIGHT_BRACE, "expected '}' after function body");
    cpl_emit_bytes(cpl, OP_NIL, OP_RETURN);
    cpl_end_func(cpl, lastLocalsStart); // TODO: fix this
    cpl->script = backup;
}

static void cpl_compile_anonymous_function(LoxSPCompiler * cpl) {
    cpl_compile_function_body(cpl, NULL, FUNC_ANONYMOUS);
}

static void cpl_compile_declaration(LoxSPCompiler * cpl) {
    if(cpl_match(cpl, TOKEN_VAR)) {
        cpl_compile_var_declaration(cpl);
        cpl_consume_semicolon(cpl);
    } else if (cpl_match(cpl, TOKEN_FUN)) {
        cpl_consume(cpl, TOKEN_IDENTIFIER, "expected identifier after 'fun' keyword");
        const LoxString * name = cpl_intern_str(cpl, cpl->previous.start, cpl->previous.length);
        cpl_compile_function_body(cpl, name, FUNC_ORDINARY);
    } else {
        cpl_compile_statement(cpl);
    }
    if(cpl->in_panic_mode) cpl_syncronize(cpl);
}

static void cpl_destroy(LoxSPCompiler * cpl) {
    sc_destroy(&cpl->in);

    cpl->strings  = NULL;
    memset(&cpl->previous, 0, sizeof(Token));
    memset(&cpl->current, 0, sizeof(Token));

    cpl->error_found   = false;
    cpl->in_panic_mode = false;
}

static LoxParserRule rules[] = {
    [TOKEN_LEFT_PAREN]    = { cpl_compile_grouping, cpl_compile_call, PREC_PRIMARY },
    [TOKEN_RIGHT_PAREN]   = { NULL, NULL, PREC_NONE },
    [TOKEN_LEFT_BRACE]    = { NULL, NULL, PREC_NONE },
    [TOKEN_RIGHT_BRACE]   = { NULL, NULL, PREC_NONE },
    [TOKEN_COMMA]         = { NULL, NULL, PREC_NONE },
    [TOKEN_DOT]           = { NULL, NULL, PREC_NONE },
    [TOKEN_MINUS]         = { cpl_compile_unary, cpl_compile_binary, PREC_TERM },
    [TOKEN_PLUS]          = { NULL, cpl_compile_binary, PREC_TERM },
    [TOKEN_SEMICOLON]     = { NULL, NULL, PREC_NONE },
    [TOKEN_SLASH]         = { NULL, cpl_compile_binary, PREC_FACTOR },
    [TOKEN_STAR]          = { NULL, cpl_compile_binary, PREC_FACTOR },

    // One or two character tokens.
    [TOKEN_BANG]          = { cpl_compile_unary, NULL, PREC_UNARY },
    [TOKEN_BANG_EQUAL]    = { NULL, cpl_compile_binary, PREC_EQUALITY   },
    [TOKEN_EQUAL]         = { NULL, NULL, PREC_NONE },
    [TOKEN_EQUAL_EQUAL]   = { NULL, cpl_compile_binary, PREC_EQUALITY   },
    [TOKEN_GREATER]       = { NULL, cpl_compile_binary, PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL] = { NULL, cpl_compile_binary, PREC_COMPARISON },
    [TOKEN_LESS]          = { NULL, cpl_compile_binary, PREC_COMPARISON },
    [TOKEN_LESS_EQUAL]    = { NULL, cpl_compile_binary, PREC_COMPARISON },

    // Literals.
    [TOKEN_IDENTIFIER]    = { cpl_compile_variable, NULL, PREC_NONE },
    [TOKEN_STRING]        = { cpl_compile_string,   NULL, PREC_NONE },
    [TOKEN_NUMBER]        = { cpl_compile_number,   NULL, PREC_NONE },

    // Keywords.
    [TOKEN_NIL]           = { cpl_compile_primary, NULL, PREC_NONE },
    [TOKEN_TRUE]          = { cpl_compile_primary, NULL, PREC_NONE },
    [TOKEN_FALSE]         = { cpl_compile_primary, NULL, PREC_NONE },

    [TOKEN_AND]           = { NULL, cpl_compile_binary, PREC_AND  },
    [TOKEN_ELSE]          = { NULL, NULL, PREC_NONE },
    [TOKEN_FOR]           = { NULL, NULL, PREC_NONE },
    [TOKEN_FUN]           = { cpl_compile_anonymous_function, NULL, PREC_PRIMARY },
    [TOKEN_IF]            = { NULL, NULL, PREC_NONE },
    [TOKEN_OR]            = { NULL, cpl_compile_binary, PREC_OR   },
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

static LoxFunction * cpl_compile(LoxSPCompiler * cpl) {
    cpl_begin_func(cpl);
        cpl_advance(cpl);
        while(!cpl_match(cpl, TOKEN_EOF)){
            cpl_compile_declaration(cpl);
        }
        cpl_emit_bytes(cpl, OP_POP, OP_RETURN);
    cpl_end_func(cpl, 0);
    return cpl->script;
}

LoxFunction * compile(const char * source, HashMap * strings) {
    LoxSPCompiler cpl;
    cpl_init(&cpl, source, strings);

    LoxFunction * script = cpl_compile(&cpl);
    if(cpl.error_found) {
        lox_obj_destroy((LoxObject*) script);
        script = NULL;
    }

    cpl_destroy(&cpl);
    return script;
}
