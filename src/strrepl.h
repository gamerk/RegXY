#ifndef STRREPL_H
#define STRREPL_H
#include <stddef.h>

#define ASSERT_NOT(cond, fmt, ...) if(cond) {fprintf(stderr, fmt , ## __VA_ARGS__); exit(EXIT_FAILURE);}

typedef struct _repl {
    size_t start;
    size_t end;
    char* repl;
} Repl;

char* fread_all(const char* filename);
char* str_replaced(const char* s, size_t start, size_t end, const char* repl);
char* str_multi_replace(const char* s, Repl repls[], size_t count, int is_sorted);


#endif /* STRREPL_H */
