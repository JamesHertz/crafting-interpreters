package jh.craft.interpreter.core;

import jh.craft.interpreter.scanner.Token;
import jh.craft.interpreter.types.LoxError;

import java.util.HashMap;
import java.util.Map;

public class LoxInstance {

    private final LoxClass klass;
    private final Map<String, Object> fields;

    public LoxInstance(LoxClass klass) {
        this.klass = klass;
        this.fields = new HashMap<>();
    }

    public Object get( Token property ){
        var identifier = property.lexeme();

        if( fields.containsKey(identifier) )
            return fields.get(identifier);

        var method = klass.findMethod( identifier );
        if(method != null)
            return method.bind(this);

        throw new LoxError(
            property, String.format(
                        "Property '%s' not defined.", identifier
            )
        );
    }

    public void set( String name, Object value ){
        fields.put(name, value);
    }

    @Override
    public String toString() {
        return "<instance of " + klass.name() + ">";
    }
}
