#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "strrepl.h"


char* fread_all(const char* filename){
    FILE* fp = fopen(filename, "r");
    if (!fp){
        fputs("Could not open file!\n", stderr);
        exit(EXIT_FAILURE);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* content = (char*)calloc(fsize + 1, sizeof(char));
    if (!content){
        fclose(fp);
        free(fp);
        fputs("Could not allocate content!\n", stderr);
        exit(EXIT_FAILURE);
        return NULL;
    }

    fread(content, sizeof(char), fsize, fp);

    fclose(fp);
    free(fp);

    return content;

}

char* str_replace(const char* s, size_t start, size_t end, const char* repl){
    if (start > end){
        fputs("Start is after end of string replacement\n", stderr);
        exit(EXIT_FAILURE);
        return NULL;
    }
    size_t new_size = strlen(s) + (strlen(repl) - (end - start));
    char* new_s = calloc(new_size + 1, sizeof(char));
    
    // Copy start of s to new_s
    strncpy(new_s, s, start);
    
    // Copy repl to new_s
    strcpy(new_s + start, repl);

    // Copy end of s to new_s
    strncpy(new_s + start + strlen(repl), s + end, strlen(s) - end);

    return new_s;
}