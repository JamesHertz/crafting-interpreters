package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.*;
import jh.craft.interpreter.representation.Expr;
import jh.craft.interpreter.representation.Stmt;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.scanner.TokenType;

import static jh.craft.interpreter.scanner.TokenType.*;

import java.util.ArrayList;
import java.util.List;

public class LoxParser {

    private final List<Token> tokens;
    private final LoxErrorReporter reporter;
    private int current;

    public LoxParser(List<Token> tokens, LoxErrorReporter reporter){
        this.tokens = tokens;
        this.reporter = reporter;
        this.current = 0;
    }

    public List<Stmt> parse(){
        var stmts = new ArrayList<Stmt>();
        while(!isAtEnd()){
            try{
                stmts.add( statement() );
            }catch (LoxError error){
                reporter.error(error);
                synchronize();
            }
        }
        return stmts;
    }

    private Stmt statement(){
        if(match(PRINT)) return printStatement();
        return expressionStatement();
    }

    private Stmt printStatement(){
        var statement = new Stmt.Print(
                expression()
        );

        consume(SEMICOLON, "Expected ';' after expression.");
        return statement;
    }

    private Stmt expressionStatement(){
        var statement = new Stmt.Expression(
                expression()
        );

        consume(SEMICOLON, "Expected ';' after expression.");
        return statement;
    }

    private Expr expression(){
        return equality();
    }

    private Expr equality(){
        var expr = comparison();

        while(match( BANG_EQUAL, EQUAL_EQUAL )){
            var operator = previous();
            var second = comparison();
            expr = new Expr.Binary(expr, operator, second);
        }

        return expr;
    }

    private Expr comparison(){
        var expr = term();

        while (match( GREATER, GREATER_EQUAL, LESS, LESS_EQUAL )){
            var operator = previous();
            var second = comparison();
            expr = new Expr.Binary(expr, operator, second);
        }

        return expr;
    }

    private Expr term(){
        var expr = factor();

        while( match(PLUS, MINUS) ){
            var operator = previous();
            var second = term();
            expr = new Expr.Binary(expr, operator, second);
        }

        return expr;
    }

    private Expr factor(){
        var expr = unary();

        while(match(SLASH, STAR)){
            var operator = previous();
            var second = factor();
            expr = new Expr.Binary(expr, operator, second);
        }

        return expr;
    }

    private Expr unary() {

        if(match(MINUS, BANG)){
            var operator = previous();
            var second = unary();
            return new Expr.Unary(operator, second);
        }

        return primary();
    }

    private Expr primary(){
        var token = advance();
        return switch (token.type()){
            case NUMBER, STRING -> new Expr.Literal( token.literal() );
            case NIL   -> new Expr.Literal( null );
            case TRUE  -> new Expr.Literal( true );
            case FALSE -> new Expr.Literal( false );
            case LEFT_PAREN -> {
                var expr = expression();
                if( match( RIGHT_PAREN ) ){
                    yield new Expr.Grouping( expr );
                }
                throw error("Expected ')' :c");
            }
            default -> {
                throw error("Expected an expression :c");
            }
        };
    }

    public LoxError error(String msg){
        var token = previous();
        return new LoxError(
                token.line(), token.position(), msg
        );
    }

    public boolean match(TokenType...types){
        if( isAtEnd() )  // by now c:
            return false;

        var expr = peek();
        for( var t : types ){
            if( peek().type() == t ){
                advance();
                return true;
            }
        }
        return false;
    }

    // The idea is to skip enough tokens until we
    // get to a token that starts a new statement
    // this way we can report several syntax errors
    // at once.
    private void synchronize(){
        advance(); // skip error token c:

        while(!isAtEnd()){
            if(previous().type() == SEMICOLON )
                return;

            switch (peek().type()){
                case CLASS:
                case IF:
                case VAR:
                case FOR:
                case WHILE:
                case FUN:
                case PRINT:
                case RETURN:
                    return;
            }

            advance();
        }
    }

    private void consume(TokenType type, String msg){
        var token = peek();
        if( token.type() != type ){
            throw new LoxError(
                    token.line(), token.position(), msg
            );
        }
        advance();
    }

    private boolean isAtEnd(){
        return peek().type() == EOF;
    }

    private Token peek(){
        return tokens.get(current);
    }

    private Token advance(){
        if(!isAtEnd()) current++;
        return previous();
    }

    private Token previous(){
        return tokens.get( current - 1 );
    }

}
