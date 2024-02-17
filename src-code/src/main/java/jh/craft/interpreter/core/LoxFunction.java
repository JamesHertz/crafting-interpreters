package jh.craft.interpreter.core;

import jh.craft.interpreter.ast.Expr;
import jh.craft.interpreter.ast.Stmt;
import jh.craft.interpreter.types.LoxCallable;

import java.util.List;

public class LoxFunction implements LoxCallable {

    private final Environment closure;
    private final Stmt.Function declaration;

    public LoxFunction(Environment closure, Stmt.Function declaration){
        this.closure = closure;
        this.declaration = declaration;
    }

    @Override
    public int arity() {
        return declaration.parameters().size();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        var callEnv = new Environment( closure );

        var params = declaration.parameters();
        for(var i = 0; i < params.size(); i++){
            var param = params.get(i);
            // TODO: think about this...
            callEnv.define(
                    param.lexeme(), arguments.get(i)
            );
        }

        try{
            interpreter.executeBlock( declaration.body(), callEnv );
        }catch (Return ret){
            return ret.value;
        }

        return null;
    }


    @Override
    public String toString() {
        return "<fn " +  declaration.name().lexeme()  + ">";
    }


    static class AnonymousFunction extends LoxFunction {
        public AnonymousFunction(Environment closure, Expr.AnonymousFun declaration) {
            // TODO: fix this later
            super(closure, new Stmt.Function(null, declaration.parameters(), declaration.body()));
        }

        @Override
        public String toString() {
            return "<anonymous fn>";
        }
    }
}
