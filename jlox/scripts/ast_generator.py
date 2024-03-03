#!/usr/bin/env python3

from io import TextIOWrapper
from typing import Dict

BASE_NAME = "src/main/java/jh/craft/interpreter/ast/{}.java"

def join_lines(lines):
    return '\n'.join(lines)

def generate_classes( baseclass: str, classes : Dict[str, str] ) :
    return f"""
package jh.craft.interpreter.ast;

import jh.craft.interpreter.scanner.Token;
import java.util.List;

public interface {baseclass} {{

    interface Visitor<T> {{
{ join_lines(
    [
        f'        T visit{cname}( {cname} {cname[0].lower() + cname[1:]} );' 
        for cname in classes
    ]
)}
    }}

    <T> T accept( Visitor<T> visitor );
{
    join_lines([
 f'''
    record {cname}( {args} ) implements {baseclass} {{
        @Override
        public <T> T accept( Visitor<T> visitor ){{ 
            return visitor.visit{cname}( this );
        }}
    }}'''   for cname, args in classes.items()
    ])
}

}}"""
   

def define_ast(filename : str, classes : Dict[str, str]):
    with open(BASE_NAME.format(filename), "w") as file:
        file.write(
            generate_classes(filename, classes)
        )

def main():
    define_ast('Expr', {
        'Binary'   : 'Expr left, Token operator, Expr right',
        'Literal'  : 'Object value',
        'Grouping' : 'Expr expression',
        'Unary'    : 'Token operator, Expr expression',
        'Variable' : 'Token name',
        'Assign'   : 'Token name, Expr value',
        'Logical'  : 'Expr left, Token operator, Expr right',
        'Call'     : 'Expr callee, Token rightParen, List<Expr> arguments',
        'AnonymousFun' : 'List<Token> parameters, List<Stmt> body',
        'Get'  : 'Expr expression, Token property',
        'Set'  : 'Expr expression, Token property, Expr value',
        'ThisExpr'  : 'Token keyword',
        'SuperExpr' : 'Token keyword, Token identifier',
    })
    
    define_ast('Stmt', {
        'Expression' : 'Expr expression',
        'Print'      : 'Expr expression',
        'Var'        : 'Token name, Expr initializer',
        'Block'      : 'List<Stmt> body',
        'IfStmt'     : 'Expr condition, Stmt body, Stmt elseStmt',
        'WhileStmt'  : 'Expr condition, Stmt body',
        'FunctionDecl' : 'Token name, List<Token> parameters, List<Stmt> body',
        'ReturnStmt'   : 'Token keyword, Expr value',
        'ClassDecl'    : 'Token name, Token superClass, List<FunctionDecl> methodsDecls',
    })


if __name__ == "__main__":
    main()


