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

// -------------------- String chunking --------------------

SChunk* new_schunk(SChunk* next, char* str){
    /**
     * @brief Creates a new SChunk struct pointer
     * 
     * @param next Pointer to the next SChunk
     * @param str String to use in SChunk. Truncated to CHUNK_SIZE bytes.
     * 
     * @returns SChunk struct pointer
     */

    if (!str){
        fputs("String of SChunk cannot be NULL\n", stderr);
        exit(EXIT_FAILURE);
        return NULL;
    }
    size_t slen = strlen(str);
    SChunk* sc = (SChunk*)malloc(sizeof(SChunk));
    if (!sc){
        fputs("Could not allocate SChunk!\n", stderr);
        exit(EXIT_FAILURE);
        return NULL;
    }

    sc->next = next;
    sc->length = slen > CHUNK_SIZE ? CHUNK_SIZE : slen;
    strncpy(sc->str, str, CHUNK_SIZE);
    return sc;
}

SChunk* str2schunk(char* s){
    SChunk* root = new_schunk(NULL, s);
    SChunk* curr = root;

    size_t slen = strlen(s);
    for (int i = CHUNK_SIZE; i < slen; i += CHUNK_SIZE){
        curr->next = new_schunk(NULL, s + i);
        curr = curr->next;
    }

    return root;
}

char get_char(SChunk* sc, size_t index){

    if (!sc){
        fputs("Cannot fetch index from a NULL SChunk\n", stderr);
        exit(EXIT_FAILURE);
        return 0;
    } else if (index < 0){
        fputs("Index cannot be less than 0\n", stderr);
        exit(EXIT_FAILURE);
        return 0;
    }

    static SChunk* gc_curr = NULL;
    static size_t total_len = 0;

    if (!gc_curr || index <= total_len){
        gc_curr = sc;
        total_len = 0;
    }


    do {
        if (index < (total_len + gc_curr->length)){
            return gc_curr->str[index - total_len];
        }
        total_len += gc_curr->length;
        gc_curr = gc_curr->next;
    } while (gc_curr);

    fputs("Index is out of bounds\n", stderr);
    exit(EXIT_FAILURE);
    return 0;
}