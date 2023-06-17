#include <stddef.h>
#define CHUNK_SIZE 100000

typedef struct schunk {
    struct schunk* next;
    int length;
    char str[CHUNK_SIZE];
} SChunk;

char* fread_all(const char* filename);
char* str_replace(const char* s, size_t start, size_t end, const char* repl);

SChunk* new_schunk(SChunk* next, char* str);
SChunk* str2schunk(char* s);
char get_char(SChunk* sc, size_t index);