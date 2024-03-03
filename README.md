# Crafting an Interpreter

This is just me following along with the implementation of two interpreters from this [book](https://craftinginterpreters.com/contents.html). The specification of the programming language, named Lox, can be found [here](https://craftinginterpreters.com/the-lox-language.html). My implementation is not exactly the same as the one provided by the [book repo](https://github.com/munificent/craftinginterpreters). I added some of the features that were suggested on the challenges at the end of the chapters and I organized the code my way.

# Folder structure
As you might already notice there are two folders on the root of this repository, as listed below.
- [*clox*](clox) - has a VM interpreter in C Programming Language which the parser outputs an bytecode to be interpreter by an stack based VM.
- [*jlox*](jlox) - has an simple Lox interpreter in java which basically builds and AST and transverse it evaluating the nodes.

# Jlox
To run this version of the interpreter just run the following commands:
```sh
cd jlox
./run.sh -b
```
This will build the interpreter and run a REPL. On the following attempts you don't need to provide the `-b` flag anymore. You can also provide a filename if you don't want to use the REPL.
