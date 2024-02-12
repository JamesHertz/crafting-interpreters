package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.LoxErrorReporter;
import jh.craft.interpreter.scanner.LoxScanner;

public class Lox {
    private boolean hasError;
    private final LoxErrorReporter reporter;
    private final Interpreter interpreter;


    public Lox(LoxErrorReporter reporter) {
        this.reporter = error -> {
            hasError = true;
            reporter.error( error );
        };
        this.hasError = false;
        this.interpreter = new Interpreter();
    }


    public void run(String sourceCode){
        var tokens = new LoxScanner(
                sourceCode, reporter
        ).getTokens();

        if(!hasError){
            var statements = new LoxParser(
                    tokens, reporter
            ).parse();

            if(!hasError)
                interpreter.interpret( statements );
        }

        hasError = false;
    }

}
