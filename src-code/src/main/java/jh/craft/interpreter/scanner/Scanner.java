package jh.craft.interpreter.scanner;

import jh.craft.interpreter.errors.ParsingError;
import jh.craft.interpreter.errors.ParsingException;
import jh.craft.interpreter.errors.Result;

import java.util.*;

public class Scanner {


    // Result<Token, Error>
    public static Result<List<Token>, ParsingError> scanTokens(String source){
        List<Token> tokens = new ArrayList<>();
        var scanner = new TextScanner(source);

        while(scanner.hasNext()){

            try{
                var token = scanner.scanToken();
                token.ifPresent(tokens::add);
            }catch (ParsingException ex){
                return Result.error( ex.getError() );
            }

        }

        // when the text is over the scanner
        // will output only EOF
        tokens.add(
                scanner.scanToken().orElseThrow()
        );

        return Result.ok(tokens);
    }

}
