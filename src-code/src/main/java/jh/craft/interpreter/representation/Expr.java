
package jh.craft.interpreter.representation;

import jh.craft.interpreter.scanner.Token;

public interface Expr {

    interface Visitor<T> {
        T visitBinary( Binary binary );
        T visitLiteral( Literal literal );
        T visitGrouping( Grouping grouping );
        T visitUnary( Unary unary );
    }

    <T> T accept( Visitor<T> visitor );

    record Binary( Expr left, Token operator, Expr right ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitBinary( this );
        }
    }

    record Literal( Object value ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitLiteral( this );
        }
    }

    record Grouping( Expr expression ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitGrouping( this );
        }
    }

    record Unary( Token operator, Expr expression ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitUnary( this );
        }
    }

}