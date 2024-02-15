package jh.craft.interpreter.types;

import jh.craft.interpreter.ast.Expr;
import jh.craft.interpreter.core.Interpreter;

import java.util.List;

public interface LoxCallable {
    int arity();
    Object call(Interpreter interpreter, List<Object> arguments);
}
