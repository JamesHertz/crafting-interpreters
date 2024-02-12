package jh.craft.interpreter.errors;

public class LoxError extends RuntimeException{
    public final int line;
    public final int position;
    public final String msg ;

    public LoxError(int line, int position, String msg) {
        this.line = line;
        this.position = position;
        this.msg = msg;
    }
}
