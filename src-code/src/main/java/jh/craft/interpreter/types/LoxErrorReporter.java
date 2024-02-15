package jh.craft.interpreter.types;

@FunctionalInterface
public interface LoxErrorReporter {
    void error(LoxError error);
}
