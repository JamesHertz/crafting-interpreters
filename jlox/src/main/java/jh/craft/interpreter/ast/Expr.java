
package jh.craft.interpreter.ast;

import jh.craft.interpreter.scanner.Token;
import java.util.List;

public interface Expr {

    interface Visitor<T> {
        T visitBinary( Binary binary );
        T visitLiteral( Literal literal );
        T visitGrouping( Grouping grouping );
        T visitUnary( Unary unary );
        T visitVariable( Variable variable );
        T visitAssign( Assign assign );
        T visitLogical( Logical logical );
        T visitCall( Call call );
        T visitAnonymousFun( AnonymousFun anonymousFun );
        T visitGet( Get get );
        T visitSet( Set set );
        T visitThisExpr( ThisExpr thisExpr );
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

    record Variable( Token name ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitVariable( this );
        }
    }

    record Assign( Token name, Expr value ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitAssign( this );
        }
    }

    record Logical( Expr left, Token operator, Expr right ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitLogical( this );
        }
    }

    record Call( Expr callee, Token rightParen, List<Expr> arguments ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitCall( this );
        }
    }

    record AnonymousFun( List<Token> parameters, List<Stmt> body ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitAnonymousFun( this );
        }
    }

    record Get( Expr expression, Token property ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitGet( this );
        }
    }

    record Set( Expr expression, Token property, Expr value ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitSet( this );
        }
    }

    record ThisExpr( Token keyword ) implements Expr {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitThisExpr( this );
        }
    }

}