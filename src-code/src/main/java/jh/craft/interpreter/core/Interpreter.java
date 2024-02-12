package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.LoxError;
import jh.craft.interpreter.representation.Expr;
import jh.craft.interpreter.representation.Stmt;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.utils.Utils;

import java.util.List;

public class Interpreter implements Expr.Visitor<Object> {


    public Interpreter(){}

    public Object eval(Expr expr){
        return expr.accept( this );
    }

    @Override
    public Object visitLiteral(Expr.Literal literal) {
        return literal.value();
    }

    @Override
    public Object visitGrouping(Expr.Grouping grouping) {
        var expr = grouping.expression();
        return expr.accept( this );
    }

    @Override
    public Object visitBinary(Expr.Binary binary) {
        var right = binary.right().accept( this );
        var left  = binary.left().accept( this );
        var op = binary.operator();

        return switch (op.type()){
            case EQUAL_EQUAL -> isEqual(left, right);
            case BANG_EQUAL  -> !isEqual(left, right);

            case PLUS -> {
                if( right instanceof Double && left instanceof Double)
                    yield (double) left + (double) right;

                if( right instanceof String && left instanceof String)
                    yield (String) left + (String) right;

                throw this.error( op, String.format(
                    "Expected either two strings or two number but got: %s and %s",
                    Utils.stringify(left), Utils.stringify(right)
                ));

            }

            case MINUS -> {
                checkNumberOperands(op, left, right);
                yield (double) left - (double) right;
            }

            case STAR -> {
                checkNumberOperands(op, left, right);
                yield (double) left * (double) right;
            }

            case SLASH -> {
                checkNumberOperands(op, left, right);
                yield (double) left / (double) right;
            }

            default -> {
                throw new RuntimeException("Unreachable");
            }
        };

    }

    @Override
    public Object visitUnary(Expr.Unary unary) {
        var op = unary.operator();
        var value = unary.expression().accept( this );
        return switch (op.type()){
            case BANG -> ! isTruly( value );
            case MINUS -> {
               checkNumberOperands(op, value);
               yield  - (double) value;
            }
            default -> {
                throw new RuntimeException("Unreachable");
            }
        };
    }


    public boolean isEqual(Object fst, Object snd){
        if(fst == null) return snd == null;
        else return fst.equals(snd);
    }

    public boolean isTruly(Object value){
        if( value == null ) return false;
        else if( value instanceof  Boolean ) return (Boolean) value;
        else return true;
    }


    public void checkNumberOperands(Token operator, Object ...values){
        for( var val : values )
            if(!( val instanceof Double)){
                throw this.error(
                        operator, String.format("Expected a number but found: %s", Utils.stringify( val ) )
                );
            }
    }


    public LoxError error(Token token, String msg){
        return new LoxError(
                token.line(), token.position(), msg
        );
    }

    public Object interpret(List<Stmt> statements){
        return null;
    }

//    public static Result<Object, ParsingError> evaluate(Expr expr){
//        try{
//            return Result.ok( INTER.eval(expr) );
//        }catch (ParsingException ex){
//            return Result.error( ex.getError() );
//        }
//    }
}
