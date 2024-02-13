package jh.craft.interpreter.core;

import jh.craft.interpreter.errors.LoxError;
import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.scanner.TokenType;

import java.util.HashMap;
import java.util.Map;

public class Environment {
    private final Map<String, Object> values;
    private final Environment parent;

    public Environment(){
        this(null);
    }
    public Environment(Environment parent){
        this.values = new HashMap<>();
        this.parent = parent;
    }

    public void initialize(Token name, Object value){
        values.put( name.lexeme(), value );
    }

    public Object value(Token name){
        var identifier = name.lexeme();

        if(values.containsKey( identifier ))
            return values.get( identifier );

        if( parent != null ) return parent.value( name );

        throw new LoxError(
                name.line(), name.position(),
                String.format("'%s' not defined.", identifier)
        );
    }

    public void assign(Token name, Object value){
        var identifier = name.lexeme();

        if(values.containsKey( identifier ))
            values.put( identifier , value );
        else if( parent != null )
            parent.assign( name, value );
        else {
            throw new LoxError(
                    name.line(), name.position(),
                    String.format("'%s' not defined.", identifier)
            );
        }
    }
}
