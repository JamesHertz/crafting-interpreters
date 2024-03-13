#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "compiler.h"

static char * read_file(const char * path){

    FILE * file = fopen(path, "r");

    if(file == NULL){
        fprintf(stderr, "Error opening '%s': ", path);
        perror(""); exit(1);
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
    compile(file_data);
    free(file_data);
}

static void prompt(){
    printf("> "); fflush(stdout);
}

static void repl(){
    char line[1024];
    for(;;){

        prompt();
        if(fgets(line, sizeof(line), stdin) == NULL){
            putchar('\n');
            break;
        }

        compile(line);
        


        // do something
        // printf(">> %s\n", line);
    }

}

#include "memory.h"
typedef struct {
    int * values;
    size_t length;
    size_t size;
} int_list_t; 

int main(int argc, char ** argv){
    int_list_t list;
    da_init(&list, int);

    printf("length : %zu\n", list.length);
    printf("size   : %zu\n", list.size);

    for(int i = 1; i <= 10; i++){
        da_add(&list, i, int);
        printf("length : %zu\n", list.length);
        printf("size   : %zu\n", list.size);
        printf("-----------------------\n");
    }

    for(int i = 0; i < list.length; i++){
        printf("> %d\n", list.values[i]);
    }

    // if(argc == 1){
    //     repl();
    // } else if(argc == 2){
    //     run_file(argv[1]);
    // } else {
    //     printf("usage: %s <path>\n", argv[0]);
    // }

    return 0;
}
