#!/usr/bin/env python3

from io import TextIOWrapper
from os import write


FILE_NAME = "src/main/java/jh/craft/interpreter/representation/Expr.java"

CLASSES = { 
    'Binary' : 'Expr left, Token operator, Expr right',
    'Literal': 'Object value',
    'Group'  : 'Expr expression',
    'Unary'  : 'Token operator, Expr expression'
}

def generate_classes( file : TextIOWrapper ) :
    file.write("""
package jh.craft.interpreter.representation;

public abstract class Expr {

    public abstract <T> T accept( Visitor<T> visitor);

    public interface Visitor<T> {
""")

    for class_name in CLASSES:
        file.write(
            f'        T visit{class_name}({class_name} {class_name.lower()});\n'
        )
    
    file.write('    }')

def main():
    with open(FILE_NAME, "w") as file:
        generate_classes(file)

if __name__ == "__main__":
    main()


