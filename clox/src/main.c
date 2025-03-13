#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <ctype.h>

#include "vm.h"

static char * read_file(const char * path){

    FILE * file = fopen(path, "r");
    if(file == NULL){
        fprintf(stderr, "Error opening '%s': %s\n", path, strerror(errno));
        exit(1);
    }

    struct stat st;
    fstat(fileno(file), &st);

    char * file_data = malloc(st.st_size + 1);
    if(file_data == NULL){
        fprintf(stderr, "Unable to reserve enough space for file '%s'\n", path);
        exit(1);
    }

    // should I ... size_t read_bytes = ... if(read_bytes < st.st_size ) ...
    fread(file_data, sizeof(char), st.st_size, file);
    fclose(file);

    file_data[st.st_size] = 0;
    return file_data;
}

static void run_file(const char * path){
    char * file_data = read_file(path);
    interpret(file_data);
    free(file_data);
    // TODO: print status
}

static inline void prompt(){
    printf("> "); 
    fflush(stdout);
}

static bool is_empty(const char * str){
    while(*str && isspace(*str)) str++;
    return *str == '\0';
}

static void repl(){
    char line[1024];
    for(;;){

        prompt();
        if(fgets(line, sizeof(line), stdin) == NULL){
            putchar('\n');
            break;
        }

        if(!is_empty(line))
            interpret(line);
    }

}

int main(int argc, char ** argv){

    if(argc == 1){
        repl();
    } else if(argc == 2){
        run_file(argv[1]);
    } else {
        fprintf(stderr, "usage: %s <path>\n", argv[0]);
    }

    return 0;
}
