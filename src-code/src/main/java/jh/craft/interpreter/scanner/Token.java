package jh.craft.interpreter.scanner;

public record Token(TokenType type, String lexeme, Object literal, int line, int position) {

    public static Token from(Token other, TokenType type){
        return new Token(type, other.lexeme(), other.literal, other.line(), other.position());
    }
}