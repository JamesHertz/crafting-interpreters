package jh.craft.interpreter.errors;

public record ParsingError(int line, int position, String msg) { }
