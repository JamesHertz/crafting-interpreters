package jh.craft.interpreter.core;

import jh.craft.interpreter.types.LoxErrorReporter;
import jh.craft.interpreter.scanner.LoxScanner;

public class Lox {
    private boolean hasError;
    private final LoxErrorReporter reporter;
    private final Interpreter interpreter;

    public Lox(LoxErrorReporter reporter) {
        this.reporter = error -> {
            hasError = true;
            reporter.report( error );
        };
        this.hasError = false;

        this.interpreter = new Interpreter( this.reporter );
    }

    public void run(String sourceCode){
        var tokens = new LoxScanner(
                sourceCode, reporter
        ).getTokens();

        if (hasError){
            this.reset();
            return;
        }

        var statements = new LoxParser(
                tokens, reporter
        ).parse();

        if(hasError){
            this.reset();
            return;
        }

        var distances = new LoxStaticAnalyst(
                reporter
        ).declarationDistances(statements);

        if(hasError){
            this.reset();
            return;
        }

        interpreter.interpret( statements, distances );
    }


    private void reset(){
        hasError = false;
    }

}
