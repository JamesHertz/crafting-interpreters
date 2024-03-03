#include <stdio.h>

void repl(){

}

void runFile(char * filename){

}

int main(int argc, char ** argv){

    if(argc == 1){
        repl();
    } else if(argc == 2){
        runFile(argv[1]);
    } else {
        printf("usage: %s <filename>\n", argv[0]);
    }

    return 0;
}