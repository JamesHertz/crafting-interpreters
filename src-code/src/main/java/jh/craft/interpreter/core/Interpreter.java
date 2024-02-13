package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.LoxError;
import jh.craft.interpreter.errors.LoxErrorReporter;
import jh.craft.interpreter.representation.Expr;
import jh.craft.interpreter.representation.Stmt;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.scanner.TokenType;
import jh.craft.interpreter.utils.Utils;

import java.util.List;

public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {

    private final LoxErrorReporter reporter;
    private Environment currentEnv;
    public Interpreter(LoxErrorReporter reporter){
        this.reporter = reporter;
        this.currentEnv = new Environment();
    }


    public void interpret(List<Stmt> statements){
        try{
            execute(statements);
        }catch (LoxError error){
            reporter.error( error );
        }
    }

    private void execute(List<Stmt> statements){
        for( var stmt : statements )
            stmt.accept(this);
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
        var left  = binary.left().accept( this );
        var right = binary.right().accept( this );
        var op = binary.operator();

        if(op.type() == TokenType.EQUAL_EQUAL)
            return isEqual(left, right);

        if(op.type() == TokenType.BANG_EQUAL)
            return !isEqual(left, right);

        if(op.type() == TokenType.PLUS) {
            if (right instanceof Double && left instanceof Double)
                return (double) left + (double) right;

            if (right instanceof String && left instanceof String)
                return (String) left + (String) right;

            throw new LoxError(op, String.format(
                    "Expected either two strings or two number but got: %s and %s",
                    Utils.stringify(left), Utils.stringify(right)
            ));

        }

        checkNumberOperands(op, left, right);
        var rightNr = (Double) right;
        var leftNr  = (Double) left;

        return switch (op.type()){
            case MINUS   -> leftNr - rightNr;
            case STAR    -> leftNr * rightNr;
            case SLASH   -> leftNr / rightNr;
            case GREATER -> leftNr > rightNr;
            case GREATER_EQUAL -> leftNr >= rightNr;
            case LESS -> leftNr < rightNr;
            case LESS_EQUAL -> leftNr <= rightNr;
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

    @Override
    public Object visitVariable(Expr.Variable variable) {
        var name = variable.name();
        return currentEnv.value( name );
    }

    @Override
    public Object visitAssign(Expr.Assign assign) {
        var value = assign.value().accept( this );
        currentEnv.assign(
                assign.name(), value
        );
        return null;
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
                throw new LoxError(
                        operator, String.format("Expected a number but found: %s", Utils.stringify( val ) )
                );
            }
    }

    @Override
    public Void visitExpression(Stmt.Expression expression) {
        // TODO: add a listener for when we are in REPL mode
        expression.expression().accept(this);
        return null;
    }

    @Override
    public Void visitPrint(Stmt.Print print) {
        var result = print.expression().accept( this );
        System.out.println(
                Utils.stringifyValue( result )
        );
        return null;
    }

    @Override
    public Void visitVar(Stmt.Var var) {
        var value = var.initializer().accept( this );
        currentEnv.initialize( var.name(), value );
        return null;
    }

    @Override
    public Void visitBlock(Stmt.Block block) {
        var previous = currentEnv;
        this.currentEnv = new Environment( previous );
        execute( block.body() );
        this.currentEnv = previous;
        return null;
    }

    @Override
    public Void visitIfStmt(Stmt.IfStmt ifStmt) {
        var condition = ifStmt.condition().accept( this );
        if( isTruly(condition) )
            ifStmt.body().accept(this);
        else if( ifStmt.elseStmt() != null )
            ifStmt.elseStmt().accept( this );

        return null;
    }

}
