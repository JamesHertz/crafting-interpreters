package jh.craft.interpreter.core;

import jh.craft.interpreter.types.LoxCallable;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LoxClass implements LoxCallable {

    private final String name;
    private final Map<String, LoxFunction> classMethods;
    private final LoxCallable constructor;
    public LoxClass(String name, List<LoxFunction> methods) {
        this.name = name;
        this.classMethods = new HashMap<>();

        for(var m : methods)
            this.classMethods.put( m.name(), m );

        var defined = classMethods.get("init");
        this.constructor = defined != null ? defined : EMPTY_CONSTRUCTOR;
    }

    @Override
    public int arity() {
        return constructor.arity();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        var object = new LoxInstance(this);

        if(constructor instanceof LoxFunction aux){
            aux.bind(object)
               .call(interpreter, arguments);
        }

        return object;
    }

    LoxFunction findMethod(String name){
        return classMethods.get( name );
    }

    public String name() {
        return name;
    }

    @Override
    public String toString() {
        return "<class " + name + ">";
    }

    private static final LoxCallable EMPTY_CONSTRUCTOR = new LoxCallable(){
        @Override
        public int arity() {
            return 0;
        }

        @Override
        public Object call(Interpreter interpreter, List<Object> arguments) {
            return null;
        }
    };
}
