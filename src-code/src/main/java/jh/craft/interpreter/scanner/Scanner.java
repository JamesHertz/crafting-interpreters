package jh.craft.interpreter.scanner;

import jh.craft.interpreter.errors.Result;

import java.util.*;

public class Scanner {


    // Result<Token, Error>
    public static Result<List<Token>, SyntaxError> scanTokens(String source){
        List<Token> tokens = new ArrayList<>();
        var scanner = new TextScanner(source);

        while(scanner.hasNext()){
            var token = scanner.scanToken();

            if( token.isPresent() )
                tokens.add( token.get() );
            else {
                var error = scanner.getError();
                if ( error != null )
                    return Result.error( error );
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
