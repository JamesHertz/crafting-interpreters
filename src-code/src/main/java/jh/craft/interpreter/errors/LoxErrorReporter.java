package jh.craft.interpreter.errors;

@FunctionalInterface
public interface LoxErrorReporter {
    void error(LoxError error);
}
