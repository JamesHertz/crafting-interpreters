#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

char * read_file(char * filename){
    FILE * file = fopen(filename, "r");

    if(file == NULL){
        fprintf(stderr, "Error opening %s: ", filename);
        perror(""); exit(1);
    }

    struct stat st;
    fstat(fileno(file), &st);

    char * file_data = malloc( st.st_size + 1);
    if(file_data == NULL){
        fprintf(stderr, "Unable to reserve enough space for file %s\n", filename);
        exit(1);
    }

    fread(file_data, st.st_size, 1, file);
    fclose(file);

    file_data[st.st_size] = 0;
    return file_data;
}

void run_file(char * filename){
    char * file_data = read_file(filename);
    printf("file: %s\n\n--------\n\n: %s\n", filename, file_data);
    free(file_data);
}

void prompt(){
    printf("> "); fflush(stdout);
}

void repl(){
    char line[1024];
    for(;;){

        // TODO: THIS IS UNSAFE AND I NEED TO CHANGE IT LATER c:
        prompt();
        if(fgets(line, 1024, stdin) == NULL){
            putchar('\n');
            break;
        }
        // do something
        printf(">> %s\n", line);
    }

}

int main(int argc, char ** argv){

    if(argc == 1){
        repl();
    } else if(argc == 2){
        run_file(argv[1]);
    } else {
        printf("usage: %s <filename>\n", argv[0]);
    }

    return 0;
}