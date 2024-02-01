package jh.craft.interpreter.scanner;

public record Token(TokenType type, String lexeme, Object literal, int line, int position) { }