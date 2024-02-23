package jh.craft.interpreter.types;

@FunctionalInterface
public interface LoxErrorReporter {
    void report(LoxError error);
}
