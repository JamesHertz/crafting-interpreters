#!/usr/bin/env python3

from io import TextIOWrapper
from typing import Dict

BASE_NAME = "src/main/java/jh/craft/interpreter/representation/{}.java"

def join_lines(lines):
    return '\n'.join(lines)

def generate_classes( baseclass: str, classes : Dict[str, str] ) :
    return f"""
package jh.craft.interpreter.representation;

import jh.craft.interpreter.scanner.Token;

public interface {baseclass} {{

    interface Visitor<T> {{
{ join_lines(
    [
        f'        T visit{cname}( {cname} {cname.lower()} );' 
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
        'Unary'    : 'Token operator, Expr expression'
    })

    define_ast('Stmt', {
        'Expression' : 'Expr expression',
        'Print'      : 'Expr expression'
    })


if __name__ == "__main__":
    main()


