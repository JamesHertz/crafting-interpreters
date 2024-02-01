package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.ParsingError;
import jh.craft.interpreter.errors.ParsingException;
import jh.craft.interpreter.errors.Result;
import jh.craft.interpreter.representation.Expr;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.scanner.TokenType;

import static jh.craft.interpreter.scanner.TokenType.*;

import java.util.List;

public class Parser {

    // TODO: ADD NICE ERRORS HANDLING c:
    private final List<Token> tokens;
    private int current;

    public Parser(List<Token> tokens){
        this.tokens = tokens;
        this.current = 0;
    }

    public Expr parse(){
        return expression();
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

    public ParsingException error(String msg){
        var token = previous();
        return new ParsingException(
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

    private boolean isAtEnd(){
        return peek().type() == EOF;
    }

    private Token peek(){
        return tokens.get(current);
    }

    private Token advance(){
        return tokens.get(current++);
    }

    private Token previous(){
        return tokens.get( current - 1 );
    }


    public static Result<Expr, ParsingError> parse(List<Token> tokens){
        try{
            var aux = new Parser(tokens);
            return Result.ok( aux.parse() );
        } catch (ParsingException ex){
            return Result.error( ex.getError() );
        }
    }


}
