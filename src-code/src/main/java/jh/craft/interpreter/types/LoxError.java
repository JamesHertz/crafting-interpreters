package jh.craft.interpreter.types;

import jh.craft.interpreter.scanner.Token;

public class LoxError extends RuntimeException{
    public final int line;
    public final int position;
    public final String msg ;

    public LoxError(int line, int position, String msg) {
        this.line = line;
        this.position = position;
        this.msg = msg;
    }

    public LoxError(Token token, String msg){
        this(token.line(), token.position(), msg);
    }
}
