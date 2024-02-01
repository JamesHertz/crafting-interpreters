package jh.craft.interpreter.errors;

public class ParsingException extends RuntimeException {

    private final ParsingError error;

    public ParsingException(ParsingError error) {
        this.error = error;
    }

    public ParsingException(int line, int position, String msg){
        this(new ParsingError( line, position, msg));
    }

    public ParsingError getError() {
        return error;
    }
}
