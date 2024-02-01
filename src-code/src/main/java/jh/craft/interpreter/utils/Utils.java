package jh.craft.interpreter.utils;

public class Utils {
    public static String stringify(Object value){
        if(value instanceof String)
            return String.format("'%s'", value);
        return value.toString();
    }
}
