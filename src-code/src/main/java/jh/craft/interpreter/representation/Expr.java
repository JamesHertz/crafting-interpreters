
package jh.craft.interpreter.representation;

import jh.craft.interpreter.scanner.Token;

public abstract class Expr {

    public interface Visitor<T> {
        T visitBinary( Binary binary );
        T visitLiteral( Literal literal );
        T visitGroup( Group group );
        T visitUnary( Unary unary );
    }

    public abstract <T> T accept( Visitor<T> visitor );
    public record Binary( Expr left, Token operator, Expr right ) {};
    public record Literal( Object value ) {};
    public record Group( Expr expression ) {};
    public record Unary( Token operator, Expr expression ) {};

}

