#include <stddef.h>

typedef struct _repl {
    size_t start;
    size_t end;
    char* repl;
} Repl;

char* fread_all(const char* filename);
char* str_replaced(const char* s, size_t start, size_t end, const char* repl);
char* str_multi_replace(const char* s, Repl repls[], size_t count);
