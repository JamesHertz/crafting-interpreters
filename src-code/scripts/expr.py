#!/usr/bin/env python3

from io import TextIOWrapper

FILE_NAME = "src/main/java/jh/craft/interpreter/representation/Expr.java"

CLASSES = { 
    'Binary'   : 'Expr left, Token operator, Expr right',
    'Literal'  : 'Object value',
    'Grouping' : 'Expr expression',
    'Unary'    : 'Token operator, Expr expression'
}

def join_lines(lines):
    return '\n'.join(lines)


def generate_classes( file : TextIOWrapper ) :
    file.write(f"""
package jh.craft.interpreter.representation;

import jh.craft.interpreter.scanner.Token;

public interface Expr {{

    interface Visitor<T> {{
{ join_lines(
    [
        f'        T visit{cname}( {cname} {cname.lower()} );' 
        for cname in CLASSES
    ]
)}
    }}

    <T> T accept( Visitor<T> visitor );
{
    join_lines([
 f'''
    record {cname}( {args} ) implements Expr {{
        @Override
        public <T> T accept( Visitor<T> visitor ){{ 
            return visitor.visit{cname}( this );
        }}
    }}'''   for cname, args in CLASSES.items()
    ])
}

}}""")
   
def main():
    with open(FILE_NAME, "w") as file:
        generate_classes(file)

if __name__ == "__main__":
    main()


