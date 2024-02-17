package jh.craft.interpreter.core;

import jh.craft.interpreter.types.LoxCallable;
import jh.craft.interpreter.types.LoxError;
import jh.craft.interpreter.types.LoxErrorReporter;
import jh.craft.interpreter.ast.Expr;
import jh.craft.interpreter.ast.Stmt;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.scanner.TokenType;
import jh.craft.interpreter.utils.Utils;

import java.util.ArrayList;
import java.util.List;

public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {

    private final Environment globalEnv;
    private final LoxErrorReporter reporter;
    private Environment currentEnv;
    public Interpreter(LoxErrorReporter reporter){
        this.reporter = reporter;
        this.globalEnv = new Environment();
        this.currentEnv = globalEnv;
        this.initGlobalEnvironment();
    }

    private void initGlobalEnvironment(){
        this.globalEnv.define("clock", new LoxCallable() {
            @Override
            public int arity() {
                return 0;
            }

            @Override
            public Object call(Interpreter interpreter, List<Object> arguments) {
                return (Double) (System.currentTimeMillis() / 1000.0);
            }

            @Override
            public String toString() {
                return "<native fn>";
            }
        });
    }

    public void interpret(List<Stmt> statements){
        try{
            for( var stmt : statements )
                execute(stmt);
        }catch (LoxError error){
            reporter.error( error );
        }
    }

    private void execute(Stmt statement){
        statement.accept( this );
    }

    protected void executeBlock(List<Stmt> stmts, Environment environment){
        var previous = this.currentEnv;
        this.currentEnv = environment;
        for( var stmt : stmts )
            execute(stmt);
        this.currentEnv = previous;
    }

    private Object evaluate(Expr expression){
        return expression.accept( this );
    }

    @Override
    public Object visitLiteral(Expr.Literal literal) {
        return literal.value();
    }

    @Override
    public Object visitGrouping(Expr.Grouping grouping) {
        var expr = grouping.expression();
        return evaluate( expr );
    }

    @Override
    public Object visitBinary(Expr.Binary binary) {
        var left  = evaluate(binary.left());
        var right = evaluate(binary.right());
        var op = binary.operator();

        if(op.type() == TokenType.EQUAL_EQUAL)
            return isEqual(left, right);

        if(op.type() == TokenType.BANG_EQUAL)
            return !isEqual(left, right);

        if(op.type() == TokenType.PLUS) {
            if (right instanceof Double rightNr && left instanceof Double leftNr)
                return leftNr + rightNr;

            if(right instanceof String rightStr)
                return Utils.stringifyValue(left) + rightStr;

            if(left instanceof String leftStr)
                return leftStr + Utils.stringifyValue(right);

            throw new LoxError(op, String.format(
                    "Expected either number or at least one string operand but got: %s and %s",
                    Utils.stringify(left), Utils.stringify(right)
            ));

        }

        // TODO: add a representation token for each thing c:
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
        var value = evaluate(unary.expression());
        return switch (op.type()){
            case BANG -> ! isTruly( value );
            case MINUS -> {
               checkNumberOperands(op, value);
               yield  - (Double) value;
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
        var value = evaluate(assign.value());
        currentEnv.assign(
                assign.name(), value
        );
        return null;
    }

    @Override
    public Object visitLogical(Expr.Logical logical) {
        var left = evaluate(logical.left());
        var right = logical.right();
        var op = logical.operator().type();

        if( op == TokenType.AND )
            return !isTruly(left) ? left : evaluate(right);
        else
            return isTruly(left) ? left : evaluate(right);
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
        evaluate(expression.expression());
        return null;
    }

    @Override
    public Void visitPrint(Stmt.Print print) {
        var result = evaluate(print.expression());
        System.out.println(
                Utils.stringifyValue( result )
        );
        return null;
    }

    @Override
    public Void visitVar(Stmt.Var var) {
        var initializer = var.initializer();
        if(initializer == null)
            currentEnv.declare( var.name() );
        else
            currentEnv.initialize( var.name(), evaluate(initializer) );
        return null;
    }

    @Override
    public Void visitBlock(Stmt.Block block) {
        this.executeBlock(
                block.body(), new Environment( currentEnv )
        );
        return null;
    }

    @Override
    public Void visitIfStmt(Stmt.IfStmt ifStmt) {
        var condition = evaluate( ifStmt.condition() );
        if( isTruly(condition) )
            execute( ifStmt.body() );
        else if( ifStmt.elseStmt() != null )
            execute( ifStmt.elseStmt() );
        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.WhileStmt whileStmt) {
        var condition = whileStmt.condition();
        while( isTruly( evaluate( condition ) ) )
            execute( whileStmt.body() );
        return null;
    }

    @Override
    public Void visitFunctionDecl(Stmt.FunctionDecl function) {
        currentEnv.define(
                function.name().lexeme(), new LoxFunction(currentEnv, function)
        );
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.ReturnStmt returnStmt) {
        var returnExpr = returnStmt.expression();
        throw new Return(
            returnExpr == null ? null : evaluate(returnExpr)
        );
    }

    @Override
    public Object visitCall(Expr.Call call) {
        var callee = evaluate( call.callee() );
        if(!(callee instanceof LoxCallable function)){
            throw new LoxError(
                    call.rightParen(), "Can only call functions and classes constructors."
            );
        }

        var arguments = call.arguments();
        if(function.arity() != arguments.size()){
            throw new LoxError(
                    call.rightParen(), String.format(
                            "Expected '%d' arguments but got '%d'.", function.arity(), arguments.size()
                    )
            );
        }

        var values = new ArrayList<>( arguments.size() );
        for(var expr : arguments)
            values.add( this.evaluate( expr ) );

        return function.call( this, values );
    }

    @Override
    public Object visitAnonymousFun(Expr.AnonymousFun anonymousFun) {
        return new LoxFunction.AnonymousFunction(currentEnv, anonymousFun);
    }

}
