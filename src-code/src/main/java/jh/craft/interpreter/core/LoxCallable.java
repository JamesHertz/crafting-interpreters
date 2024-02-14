package jh.craft.interpreter.core;

import jh.craft.interpreter.representation.Expr;

import java.util.List;

public interface LoxCallable {
    int arity();
    Object call(Interpreter interpreter, List<Expr> arguments);
}
