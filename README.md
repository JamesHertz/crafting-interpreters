# Crafting an Interpreter

This is just me following along with the implementation of two interpreters from this [book](https://craftinginterpreters.com/). My implementation is not exactly the same as the one provided by the [*book repo*](https://github.com/munificent/craftinginterpreters). I also added some features that were suggested on the challenges at the end of the chapters.

# Folder structure
As you might already notice there are two folders on the root of this repository, as listed below.
- [*clox*](clox) - has a VM interpreter in C Programming Language which the parser outputs an bytecode to be interpreter by an stack based VM.
- [*jlox*](jlox) - has an simple Lox interpreter in java which basically builds and AST and transverse it evaluating the nodes.
