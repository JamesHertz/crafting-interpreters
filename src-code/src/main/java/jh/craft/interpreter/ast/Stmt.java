
package jh.craft.interpreter.ast;

import jh.craft.interpreter.scanner.Token;
import java.util.List;

public interface Stmt {

    interface Visitor<T> {
        T visitExpression( Expression expression );
        T visitPrint( Print print );
        T visitVar( Var var );
        T visitBlock( Block block );
        T visitIfStmt( IfStmt ifStmt );
        T visitWhileStmt( WhileStmt whileStmt );
        T visitFunctionDecl( FunctionDecl functionDecl );
        T visitReturnStmt( ReturnStmt returnStmt );
    }

    <T> T accept( Visitor<T> visitor );

    record Expression( Expr expression ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitExpression( this );
        }
    }

    record Print( Expr expression ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitPrint( this );
        }
    }

    record Var( Token name, Expr initializer ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitVar( this );
        }
    }

    record Block( List<Stmt> body ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitBlock( this );
        }
    }

    record IfStmt( Expr condition, Stmt body, Stmt elseStmt ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitIfStmt( this );
        }
    }

    record WhileStmt( Expr condition, Stmt body ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitWhileStmt( this );
        }
    }

    record FunctionDecl( Token name, List<Token> parameters, List<Stmt> body ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitFunctionDecl( this );
        }
    }

    record ReturnStmt( Token keyword, Expr expression ) implements Stmt {
        @Override
        public <T> T accept( Visitor<T> visitor ){ 
            return visitor.visitReturnStmt( this );
        }
    }

}