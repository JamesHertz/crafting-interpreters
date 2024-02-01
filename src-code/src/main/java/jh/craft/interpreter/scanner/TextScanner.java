package jh.craft.interpreter.scanner;

import jh.craft.interpreter.errors.ParsingError;
import jh.craft.interpreter.errors.ParsingException;

import java.util.*;

class TextScanner {

    private final Map<String, TokenType> keywords = new HashMap<>(){{
        put("and", TokenType.AND);
        put("class", TokenType.CLASS);
        put("else", TokenType.ELSE);
        put("false", TokenType.FALSE);
        put("fun", TokenType.FUN);
        put("for", TokenType.FOR);
        put("if", TokenType.IF);
        put("nil", TokenType.NIL);
        put("or", TokenType.OR);

        put("print", TokenType.PRINT);
        put("return", TokenType.RETURN);
        put("super", TokenType.SUPER);
        put("this", TokenType.THIS);
        put("true", TokenType.TRUE);
        put("var", TokenType.VAR);
        put("while", TokenType.WHILE);
    }};

    private final String source;
    private int line;
    private int start;
    private int current;

    public TextScanner(String source){
        this.source = source;

        this.line    = 0;
        this.start   = 0;
        this.current = 0;
    }

    public boolean hasNext(){
        return current < source.length();
    }

    public Optional<Token> scanToken(){
        char value  = this.advance();
        Token token = switch ( value ) {
            // basic signs
            case '.' -> createToken( TokenType.DOT );
            case '(' -> createToken( TokenType.LEFT_PAREN );
            case ')' -> createToken( TokenType.RIGHT_PAREN );
            case '{' -> createToken( TokenType.LEFT_BRACE );
            case '}' -> createToken( TokenType.RIGHT_BRACE );
            case ';' -> createToken( TokenType.SEMICOLON );
            case ',' -> createToken( TokenType.COMMA );

            // arithmetic signs
            case '+' -> createToken( TokenType.PLUS );
            case '-' -> createToken( TokenType.MINUS );
            case '*' -> createToken( TokenType.STAR );
            case '/' -> checkComments();

            // handle space c:

            // comparison signs
            case '>' -> createToken(
                    match( '=' ) ? TokenType.GREATER_EQUAL : TokenType.GREATER
            );
            case '<' -> createToken(
                    match( '=' ) ? TokenType.LESS_EQUAL : TokenType.LESS
            );
            case '=' -> createToken(
                    match('=' )  ? TokenType.EQUAL_EQUAL : TokenType.EQUAL
            );
            case '!' -> createToken(
                    match('=' )  ? TokenType.BANG_EQUAL : TokenType.BANG
            );

            case '"' -> scanString();

            // nothing c:
            case ' ', '\r', '\t', '\n' -> {
                if( value == '\n' ) line++;
                yield null;
            }

            // end of file c:
            case '\0' -> createToken(
                    TokenType.EOF
            );

            // numbers and identifiers
            default -> {

                if( this.isAlpha( value ) )
                    yield scanIdentifier();

                if( this.isDigit( value ) )
                    yield scanNumber();

                throw this.error( "Unexpected symbol" );
            }

        };

        start = current;
        return Optional.ofNullable( token );
    }


    // methods used to scan next characters
    private char peek(){
        if( !this.hasNext() ) return '\0';
        return source.charAt( current );
    }

    private char peekNext(){
        if( current + 1 >= source.length() )
            return '\0';
        return source.charAt( current + 1);
    }


    private char advance(){
        if( !this.hasNext() ) return '\0';
        return source.charAt( current++ );
    }


    // used to parse more complex constructs such as Strings, Identifiers, etc...
    private Token checkComments(){
        if( match('/') ) {
            char next;
            do {
                next = advance();
            } while (next != '\n' && next != '\0');
            line++;
        } else if( match('*') ){
            char next;
            do{
                next = advance();
                if(next == '\n') line++;
                if (next == '\0')
                    throw this.error( "Unended comment. Missing '*/'");
            }while( next != '*' || !match('/') );

        } else {
            return createToken( TokenType.SLASH );
        }
        return null;
    }

    private Token scanString(){
        char value;
        do{
            value = this.advance();
            if( value == '\n') line++;
        }while(value != '"' && value != '\0');

        if(value == '\0'){
            throw this.error( "Unfinished string. Missing a '\"'" );
        }

        var stringValue = source.substring(start + 1, current-1);
        return createToken(TokenType.STRING, stringValue);
    }

    private Token scanIdentifier(){
        while( this.isAlphaNumeric( this.peek() ) ){
            current++;
        }

        var word = source.substring( start, current );
        return this.createToken(
                keywords.getOrDefault(word, TokenType.IDENTIFIER)
        );
    }

    private Token scanNumber(){
        while( this.isDigit( this.peek() ))
            current++;

        if( this.peek() == '.' && this.isDigit( this.peekNext() ) ){
            do{
                current++;
            } while( this.isDigit( this.peek() ));
        }

        try{

            var value = source.substring(start, current);
            return createToken(
                    TokenType.NUMBER, Double.parseDouble( value )
            );
        }catch (NumberFormatException ex){
            throw this.error(
                    "Number out of range: %s", ex.getMessage()
            );
        }
    }

    private Token createToken(TokenType type){
        return this.createToken( type, null);
    }

    private Token createToken(TokenType type, Object literal){
        var lexeme = source.substring(start, current);
        return new Token(
                type, lexeme, literal, line, current - 1
        );
    }


    // My shaky way of dealing with errors
    private ParsingException error(String fmt, Object ...args){
        return new ParsingException(
                line, current - 1, String.format(fmt, args)
        );
    }


    // some other methods that does some validations on characters
    private boolean match( char expected ){
        var result = peek() == expected;
        if ( result ) current++;
        return result;
    }

    private boolean isAlpha(char value){
        return value >= 'a' && value <= 'z' ||
                value >= 'A' && value <= 'Z' ||
                value == '_';
    }

    private boolean isAlphaNumeric(char value ){
        return this.isAlpha( value ) || this.isDigit( value );
    }

    private boolean isDigit(char value){
        return value >= '0' && value <= '9';
    }

}
