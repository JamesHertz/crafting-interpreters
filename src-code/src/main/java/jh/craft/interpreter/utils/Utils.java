package jh.craft.interpreter.utils;

public class Utils {
    public static String stringify(Object value){
        if(value instanceof String)
            return String.format("'%s'", value);
        else if( value == null )
            return "nil";
        return value.toString();
    }

    public static String stringifyValue(Object value){
        if( value == null ) return "nil";
        else if(value instanceof Double){
            var result = value.toString();
            if(result.endsWith(".0"))
                return result.split(".0")[0];
        }
        return value.toString();
    }


}
