package jh.craft.interpreter.scanner;

import jh.craft.interpreter.types.LoxError;
import jh.craft.interpreter.types.LoxErrorReporter;

import java.util.*;

public class LoxScanner {

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

    private final String sourceCode;
    private final LoxErrorReporter reporter;
    private int line;
    private int start;
    private int current;

    public LoxScanner(String sourceCode, LoxErrorReporter reporter){
        this.reporter = reporter;
        this.sourceCode = sourceCode;

        this.line    = 0;
        this.start   = 0;
        this.current = 0;
    }

    public boolean hasNext(){
        return current < sourceCode.length();
    }

    public List<Token> getTokens(){
        List<Token> tokens = new ArrayList<>();
        while(this.hasNext()){
            var token = this.nextToken();
            token.ifPresent(tokens::add);
        }

        tokens.add(
                this.nextToken().orElseThrow()
        );
        return tokens;
    }


    private Optional<Token> nextToken(){
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

                this.reportError( "Unexpected symbol" );
                yield null;
            }

        };

        start = current;
        return Optional.ofNullable( token );
    }



    // methods used to scan next characters
    private char peek(){
        if( !this.hasNext() ) return '\0';
        return sourceCode.charAt( current );
    }

    private char peekNext(){
        if( current + 1 >= sourceCode.length() )
            return '\0';
        return sourceCode.charAt( current + 1);
    }


    private char advance(){
        if( !this.hasNext() ) return '\0';
        return sourceCode.charAt( current++ );
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
                if (next == '\0'){
                    this.reportError( "Unended comment. Missing '*/'");
                    return null;
                }
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
            this.reportError(
                    "Unfinished string. Missing a '\"'"
            );
            return null;
        }

        var stringValue = sourceCode.substring(start + 1, current-1);
        return createToken(TokenType.STRING, stringValue);
    }

    private Token scanIdentifier(){
        while( this.isAlphaNumeric( this.peek() ) ){
            current++;
        }

        var word = sourceCode.substring( start, current );
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

            var value = sourceCode.substring(start, current);
            return createToken(
                    TokenType.NUMBER, Double.parseDouble( value )
            );
        }catch (NumberFormatException ex){
            this.reportError(
                    "Number out of range: %s", ex.getMessage()
            );
            return null;
        }
    }

    private Token createToken(TokenType type){
        return this.createToken( type, null);
    }

    private Token createToken(TokenType type, Object literal){
        var lexeme = sourceCode.substring(start, current);
        return new Token(
                type, lexeme, literal, line, current - 1
        );
    }


    // My shaky way of dealing with errors
    private void reportError(String fmt, Object ...args){
        reporter.error(new LoxError(
                line, current - 1, String.format(fmt, args)
           )
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
