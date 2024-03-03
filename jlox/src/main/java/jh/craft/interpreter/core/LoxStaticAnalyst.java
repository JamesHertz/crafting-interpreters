package jh.craft.interpreter.core;

import jh.craft.interpreter.ast.Expr;
import jh.craft.interpreter.ast.Stmt;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.types.LoxError;
import jh.craft.interpreter.types.LoxErrorReporter;

import java.util.*;

public class LoxStaticAnalyst implements Expr.Visitor<Void>, Stmt.Visitor<Void>{

    // TODO: rename this c:
    private final Map<Token, Integer> distanceToDeclaration;
    private final Stack<Set<String>> declarations;
    private final LoxErrorReporter reporter;
    private final Flags flags;


    public LoxStaticAnalyst(LoxErrorReporter reporter) {
        this.reporter = reporter;
        this.declarations = new Stack<>();
        this.distanceToDeclaration = new TreeMap<>(Comparator.comparingInt(System::identityHashCode));

        this.flags = new Flags();
    }

    public Map<Token, Integer> declarationDistances(List<Stmt> statements) {
        try{
            for (var stmt : statements)
                evaluate(stmt);
        }catch (LoxError error){
            reporter.report( error );
        }
        return distanceToDeclaration;
    }

    private void evaluate(Stmt statement){
        statement.accept( this );
    }

    private void evaluate(Expr statement){
        statement.accept( this );
    }


    @Override
    public Void visitBinary(Expr.Binary binary) {
        evaluate( binary.left() );
        evaluate( binary.right() );
        return null;
    }

    @Override
    public Void visitLiteral(Expr.Literal literal) {
        return null;
    }

    @Override
    public Void visitGrouping(Expr.Grouping grouping) {
        evaluate( grouping.expression() );
        return null;
    }

    @Override
    public Void visitUnary(Expr.Unary unary) {
        evaluate(unary.expression());
        return null;
    }

    @Override
    public Void visitVariable(Expr.Variable variable) {
        resolve(variable.name());
        return null;
    }

    @Override
    public Void visitAssign(Expr.Assign assign) {
        evaluate(assign.value());
        define( assign.name() );
        return null;
    }

    @Override
    public Void visitLogical(Expr.Logical logical) {
        evaluate(logical.left());
        evaluate(logical.right());
        return null;
    }

    @Override
    public Void visitCall(Expr.Call call) {
        evaluate(call.callee());
        for(var expr : call.arguments())
            evaluate(expr);
        return null;
    }

    @Override
    public Void visitAnonymousFun(Expr.AnonymousFun anonymousFun) {
        evalFunction(
                anonymousFun.parameters(),
                anonymousFun.body()
        );
        return null;
    }

    @Override
    public Void visitGet(Expr.Get get) {
        evaluate(get.expression());
        return null;
    }

    @Override
    public Void visitSet(Expr.Set set) {
        evaluate(set.expression());
        evaluate(set.value());
        return null;
    }

    @Override
    public Void visitThisExpr(Expr.ThisExpr thisExpr) {
        if( !flags.inMethod() ){
            throw new LoxError(
                    thisExpr.keyword(),
                    "'this' keyword should not be used outside a method."
            );
        }

        resolve( thisExpr.keyword() );
        return null;
    }

    @Override
    public Void visitExpression(Stmt.Expression expression) {
        evaluate(expression.expression());
        return null;
    }

    @Override
    public Void visitPrint(Stmt.Print print) {
        evaluate(print.expression());
        return null;
    }

    @Override
    public Void visitVar(Stmt.Var var) {
        if( var.initializer() != null )
            evaluate(var.initializer());
        define(var.name());
        return null;
    }

    @Override
    public Void visitBlock(Stmt.Block block) {
        beginScope();
            for( var stmt : block.body() )
                evaluate(stmt);
        endScope();
        return null;
    }

    @Override
    public Void visitIfStmt(Stmt.IfStmt ifStmt) {
        evaluate(ifStmt.condition());
        evaluate(ifStmt.body());
        if( ifStmt.elseStmt() != null )
            evaluate(ifStmt.elseStmt());

        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.WhileStmt whileStmt) {
        evaluate(whileStmt.condition());
        evaluate(whileStmt.body());
        return null;
    }

    @Override
    public Void visitFunctionDecl(Stmt.FunctionDecl functionDecl) {
        define(functionDecl.name());
        evalFunction(
                functionDecl.parameters(),
                functionDecl.body()
        );
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.ReturnStmt returnStmt) {

        if( !flags.inFunction() ){
            throw new LoxError(
                    returnStmt.keyword(),
                    "'return' keyword should not be used outside a function/method."
            );
        }

        if(returnStmt.value() != null)
            evaluate(returnStmt.value());

        return null;
    }

    @Override
    public Void visitClassDecl(Stmt.ClassDecl classDecl) {
        define( classDecl.name() );

        flags.beginClass();
        beginScope();

            // this is safe to add, since there is
            // no way a user can define an identifier
            // named 'this', since this itself is a token.
            declarations.peek().add("this");
                for(var decl : classDecl.methodsDecls())
                    evaluate(decl);
        endScope();
        flags.endClass();

        return null;
    }

    private void define(Token name){
        // global scope c:
        if( declarations.empty() ) return;

        var current = declarations.peek();
        var identifier = name.lexeme();

        if( !current.add(identifier) )
            throw new LoxError(
                    name, String.format("Identifier '%s' already defined.", identifier)
            );
    }

    private int findScope(Token name){
        var identifier = name.lexeme();
        var scope = declarations.size() - 1;

        while(scope >= 0 && !declarations.get(scope).contains(identifier))
            scope--;

        return declarations.size() - (scope + 1);
    }


    private void evalFunction(List<Token> params, List<Stmt> body){
        flags.beginFun();

        beginScope();
            for(var name : params )
                define(name);
            for(var stmt : body )
                evaluate(stmt);
        endScope();

        flags.endFun();
    }


    private void resolve(Token name){
        var scope = this.findScope(name);
        distanceToDeclaration.put( name, scope );
    }

    private void beginScope(){
        declarations.push(new HashSet<>());
    }

    private void endScope(){
        declarations.pop();
    }

    private static class Flags{
        private int funDepth = 0, classDepth = 0;

        void beginFun(){ funDepth++; }

        void endFun(){ funDepth--; }

        void beginClass(){ classDepth++; }

        void endClass(){ classDepth--; }

        boolean inMethod(){
            return classDepth > 0 && funDepth > 0;
        }

        boolean inClass(){
            return classDepth > 0;
        }

        boolean inFunction(){
            return funDepth > 0;
        }

    }

}
