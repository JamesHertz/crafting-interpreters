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

/*#include "hash-map.h"*/
/*#include "memory.h"*/
/*#include "darray.h"*/
/**/
/*#include <sys/time.h>*/
/*#include <string.h>*/
/*#include <errno.h>*/
/**/
/*typedef DaArray(const char *) StrArray;*/
/*bool da_find_value(StrArray * array, const char * value) {*/
/*    DA_FOR_EACH_ELEM(const char *, elem, array, {*/
/*        if(strcmp(elem, value) == 0)*/
/*            return true;*/
/*    });*/
/*    return true;*/
/*}*/
/**/
/*struct LLNode {*/
/*    const char * data;*/
/*    struct LLNode * next;*/
/*};*/
/**/
/*typedef struct {*/
/*    struct LLNode * head;*/
/*    struct LLNode * tail;*/
/*} LinkedList;*/
/**/
/*void ll_init(LinkedList * list) {*/
/*    *list = (LinkedList) {0};*/
/*}*/
/**/
/*void ll_push_d(LinkedList * list, const char * value) {*/
/*    assert(*/
/*        (list->head == list->tail || (list->head != NULL && list->tail != NULL))*/
/*        && "either use ssl_push() or ssl_push_d() don't miss and match those"*/
/*    );*/
/**/
/*    struct LLNode * new_node = mem_alloc(sizeof(struct LLNode));*/
/*    new_node->data = value;*/
/*    new_node->next = NULL;*/
/**/
/**/
/*    if(list->tail == NULL)*/
/*        list->head = list->tail = new_node;*/
/*    else {*/
/*        list->tail->next = new_node;*/
/*        list->tail = new_node;*/
/*    }*/
/*}*/
/**/
/*void ll_push(LinkedList * list, const char * value) {*/
/*    assert(list->tail == NULL && "either use ssl_push() or ssl_push_d() don't miss and match those");*/
/**/
/*    struct LLNode dummy = {*/
/*        .data = NULL,*/
/*        .next = list->head,*/
/*    };*/
/**/
/*    struct LLNode * prev = &dummy;*/
/*    while(prev->next != NULL)*/
/*        prev = prev->next;*/
/**/
/*    struct LLNode * new_node = mem_alloc(sizeof(struct LLNode));*/
/*    new_node->data = value;*/
/*    new_node->next = NULL;*/
/**/
/*    prev->next = new_node;*/
/*    list->head = dummy.next;*/
/*}*/
/**/
/*bool ll_find_value(LinkedList * list, const char * value) {*/
/*    for(struct LLNode * current = list->head; current; current = current->next) {*/
/*        if(strcmp(current->data, value) == 0) */
/*            return true;*/
/*    }*/
/*    return false;*/
/*}*/
/**/
/*void ll_destroy(LinkedList * list) {*/
/*    struct LLNode * current = list->head;*/
/**/
/*    while(current != NULL) {*/
/*        struct LLNode * next = current->next;*/
/*        mem_dealloc(current);*/
/*        current = next;*/
/*    }*/
/**/
/*    *list = (LinkedList) {0};*/
/*}*/
/**/
/*void ll_debug(LinkedList * list) {*/
/*    putchar('[');*/
/*    for(struct LLNode * current = list->head; current; current = current->next) {*/
/*        if(current != list->head)*/
/*            fputs(", ", stdout);*/
/*        printf("'%s'", current->data);*/
/*    }*/
/*    puts("]");*/
/*}*/
/**/
/*typedef struct {*/
/*    size_t size;*/
/*    const char ** values;*/
/*    size_t * search_order;*/
/*    size_t * insertion_order;*/
/**/
/*    char * file_data;*/
/*} BenckMarkData;*/
/**/
/*void shuffle_array(size_t * values, size_t size) {*/
/*    for(size_t max = size; max > 1; max--) {*/
/*        size_t destin = rand() % max;*/
/**/
/*        size_t aux = values[destin];*/
/*        values[destin] = values[0];*/
/*        values[0] = aux;*/
/*    }*/
/*}*/
/**/
/*void bench_load_data(BenckMarkData * data, const char * filename) {*/
/*    FILE * file = fopen(filename, "r");*/
/**/
/*    if(file == NULL) {*/
/*        fprintf(stderr, "Error reading '%s': %s\n", filename, strerror(errno));*/
/*        exit(1); */
/*    }*/
/**/
/*    struct stat st;*/
/*    assert(fstat(fileno(file), &st) == 0);*/
/**/
/*    char * file_data = mem_alloc(st.st_size + 1);*/
/**/
/*    assert(file_data != NULL);*/
/*    assert(fread(file_data, 1, st.st_size, file) == (size_t) st.st_size);*/
/**/
/*    file_data[st.st_size] = 0;*/
/*    size_t bench_size = 0;*/
/*    {*/
/*        char * str = file_data;*/
/*        while(*str) {*/
/*            if(*str == '\n') */
/*                bench_size++;*/
/*            str++;*/
/*        }*/
/*    }*/
/**/
/*    data->file_data = file_data;*/
/*    data->size      = bench_size;*/
/*    data->values    = mem_alloc(sizeof(const char *) * data->size);*/
/*    data->insertion_order = mem_alloc(sizeof(size_t) * data->size);*/
/*    data->search_order    = mem_alloc(sizeof(size_t) * data->size);*/
/**/
/*    size_t i = 0;*/
/*    char * current = strtok(file_data, "\n");*/
/*    data->values[i++] = current;*/
/**/
/*    while((current = strtok(NULL, "\n")) != NULL)*/
/*        data->values[i++] = current;*/
/**/
/*    assert(i == data->size);*/
/**/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        data->insertion_order[i] = i;*/
/*        data->search_order[i]    = i;*/
/*    }*/
/**/
/*    // init random c:*/
/*    struct timeval time;*/
/*    gettimeofday(&time, 0);*/
/*    srand(time.tv_sec * 1000000 + time.tv_usec);*/
/**/
/*    shuffle_array(data->insertion_order, data->size);*/
/*    shuffle_array(data->search_order, data->size);*/
/*}*/
/**/
/*void bench_debug(const BenckMarkData * data) {*/
/*    puts("VALUES          =======================");*/
/*    for(size_t i = 0; i < data->size; i++) */
/*        printf("[%03zu] = '%s' ... %p\n", i, data->values[i], data->values[i]);*/
/**/
/*    puts("INSERTION-ORDER =======================");*/
/*    for(size_t i = 0; i < data->size; i++) */
/*        printf("%zu\n", data->insertion_order[i]);*/
/**/
/*    puts("SEARCH-ORDER    =======================");*/
/*    for(size_t i = 0; i < data->size; i++) */
/*        printf("%zu\n", data->search_order[i]);*/
/*}*/
/**/
/*typedef struct {*/
/*    const char * name;*/
/*    struct timeval start;*/
/*    struct timeval end;*/
/*} BenchTimer;*/
/**/
/*void timer_init(BenchTimer * timer, const char * name) {*/
/*    timer->name  = name;*/
/*    timer->start = (struct timeval) {0};*/
/*    timer->end   = (struct timeval) {0};*/
/*}*/
/**/
/*void timer_start(BenchTimer * timer) {*/
/*    gettimeofday(&timer->start, NULL);*/
/*}*/
/**/
/*void timer_stop(BenchTimer * timer) {*/
/*    gettimeofday(&timer->end, NULL);*/
/*}*/
/**/
/*void timer_report(BenchTimer * timer) {*/
/*    uint64_t elapsed = 1000 * 1000 * (timer->end.tv_sec - timer->start.tv_sec) */
/*        + timer->end.tv_usec - timer->start.tv_usec;*/
/**/
/*    uint64_t millis  = elapsed / 1000;*/
/*    uint64_t secs    = millis  / 1000;*/
/*    uint64_t minutes = secs / 60;*/
/**/
/*    secs   %= 60;*/
/*    millis %= 1000;*/
/**/
/*    printf("timer '%s' took %lum%lus%lums (%lu us)\n", timer->name, minutes, secs, millis, elapsed);*/
/*}*/
/**/
/*void bench_darray(const BenckMarkData * data) {*/
/*    BenchTimer insert, search;*/
/**/
/*    timer_init(&insert, "Darray insert");*/
/*    timer_init(&search, "Darray search");*/
/**/
/*    StrArray array;*/
/*    da_init(&array);*/
/**/
/*    timer_start(&insert);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->insertion_order[i];*/
/*        const char * str = data->values[idx];*/
/*        da_push(&array, &str);*/
/*    }*/
/*    timer_stop(&insert);*/
/**/
/*    timer_start(&search);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->search_order[i];*/
/*        assert(da_find_value(&array, data->values[idx]) == true);*/
/*    }*/
/*    timer_stop(&search);*/
/**/
/*    timer_report(&insert);*/
/*    timer_report(&search);*/
/**/
/*    da_destroy(&array);*/
/*}*/
/**/
/*LoxString * make_lox_str(const char * value) {*/
/*    return lox_str_take(value, strlen(value));*/
/*}*/
/**/
/*void bench_hash_map(BenckMarkData * data) {*/
/*    BenchTimer insert, search;*/
/**/
/*    LoxObject head = {0};*/
/**/
/*    timer_init(&insert, "HashMap insert");*/
/*    timer_init(&search, "HashMap search");*/
/**/
/*    HashMap map;*/
/*    map_init(&map);*/
/**/
/*    timer_start(&insert);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->insertion_order[i];*/
/*        LoxString *  key = make_lox_str(data->values[idx]);*/
/*        map_set(&map, key, BOOL_VAL(true));*/
/*    }*/
/*    timer_stop(&insert);*/
/**/
/*    timer_start(&search);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->search_order[i];*/
/*        LoxString * key = make_lox_str(data->values[idx]);*/
/*        assert(map_get(&map, key) != NULL);*/
/*    }*/
/*    timer_stop(&search);*/
/**/
/*    timer_report(&insert);*/
/*    timer_report(&search);*/
/**/
/*    map_destroy(&map);*/
/**/
/*    // free objects*/
/*    {*/
/*        LoxObject * current = head.next;*/
/*        while(current) {*/
/*            LoxObject * next = current->next;*/
/*            mem_dealloc(current);*/
/*            current = next;*/
/*        }*/
/*    }*/
/*}*/
/**/
/*void bench_sll(const BenckMarkData * data) {*/
/*    LinkedList list;*/
/*    ll_init(&list);*/
/**/
/*    BenchTimer insert, search;*/
/**/
/*    timer_init(&insert, "SingleLinkedList insert");*/
/*    timer_init(&search, "SingleLinkedList search");*/
/**/
/*    timer_start(&insert);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->insertion_order[i];*/
/*        ll_push(&list, data->values[idx]);*/
/*    }*/
/*    timer_stop(&insert);*/
/**/
/*    timer_start(&search);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->search_order[i];*/
/*        assert(ll_find_value(&list, data->values[idx]) == true);*/
/*    }*/
/*    timer_stop(&search);*/
/**/
/*    timer_report(&insert);*/
/*    timer_report(&search);*/
/**/
/*    ll_destroy(&list);*/
/*}*/
/**/
/*void bench_dll(const BenckMarkData * data) {*/
/*    LinkedList list;*/
/*    ll_init(&list);*/
/**/
/*    BenchTimer insert, search;*/
/**/
/*    timer_init(&insert, "DoublyLinkedList insert");*/
/*    timer_init(&search, "DoublyLinkedList search");*/
/**/
/*    timer_start(&insert);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->insertion_order[i];*/
/*        ll_push_d(&list, data->values[idx]);*/
/*    }*/
/*    timer_stop(&insert);*/
/**/
/*    timer_start(&search);*/
/*    for(size_t i = 0; i < data->size; i++) {*/
/*        size_t idx = data->search_order[i];*/
/*        assert(ll_find_value(&list, data->values[idx]) == true);*/
/*    }*/
/*    timer_stop(&search);*/
/**/
/*    timer_report(&insert);*/
/*    timer_report(&search);*/
/**/
/*    ll_destroy(&list);*/
/*}*/
/**/
/*void small_bench_mark(void) {*/
    /*LinkedList list;*/
    /*ll_init(&list);*/
    /**/
    /*ll_push_d(&list, "a");*/
    /*ll_push_d(&list, "b");*/
    /*ll_push_d(&list, "c");*/
    /*ll_push_d(&list, "d");*/
    /*ll_push_d(&list, "e");*/
/**/
    /*printf("ssl_find_value(&list, \"a\") = %s\n", ll_find_value(&list, "a") ? "true" : "false");*/
    /*printf("ssl_find_value(&list, \"b\") = %s\n", ll_find_value(&list, "b") ? "true" : "false");*/
    /*printf("ssl_find_value(&list, \"c\") = %s\n", ll_find_value(&list, "c") ? "true" : "false");*/
    /*printf("ssl_find_value(&list, \"d\") = %s\n", ll_find_value(&list, "d") ? "true" : "false");*/
    /*printf("ssl_find_value(&list, \"e\") = %s\n", ll_find_value(&list, "e") ? "true" : "false");*/
    /*printf("ssl_find_value(&list, \"j\") = %s\n", ll_find_value(&list, "j") ? "true" : "false");*/
    /*printf("ssl_find_value(&list, \"g\") = %s\n", ll_find_value(&list, "g") ? "true" : "false");*/
    /**/
    /*ll_debug(&list);*/
    /*exit(0);*/
/**/
/*    char * tests[] = {*/
/*        "bench/10-str.txt",*/
/*        "bench/1000-str.txt",*/
/*        "bench/10000-str.txt",*/
/*        "bench/50000-str.txt",*/
/*        "bench/100000-str.txt",*/
/*    };*/
/**/
/*    size_t tests_size = sizeof(tests) / sizeof(char *);*/
/**/
/*    for(size_t i = 0; i < tests_size; i++) {*/
/*        const char * filename = tests[i];*/
/*        printf("reading '%s' **************************************\n", filename);*/
/**/
/*        BenckMarkData data;*/
/*        bench_load_data(&data, filename);*/
/**/
/*        bench_darray(&data);*/
/*        bench_hash_map(&data);*/
/*        bench_sll(&data);*/
/*        bench_dll(&data);*/
/*    }*/
/**/
/*    exit(0);*/
/*}*/
