package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.LoxError;
import jh.craft.interpreter.scanner.Token;

import java.util.HashMap;
import java.util.Map;

public class Environment {
    private final Map<String, Object> values;

    public Environment(){
        this.values = new HashMap<>();
    }

    public void initialize(Token name, Object value){
        values.put( name.lexeme(), value );
    }

    public Object value(Token name){
        var identifier = name.lexeme();

        if( !values.containsKey( identifier ) ){
            throw new LoxError(
                    name.line(), name.position(),
                    String.format("'%s' not defined.", identifier)
            );
        }

        return values.get( identifier );
    }

}
