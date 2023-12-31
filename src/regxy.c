#include "regex/regex_parse.h"
#include "regex/regex_simplify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// gcc src/*.c src/*.h src/regex/*.c src/regex/*.h -ggdb && a.exe
// cl src/*.c src/regex/*.c /Zi && drmemory regxy.exe

#define PrintRange(s, start, end) {for (int _I = ((start >= 0) ? start : 0); _I < end && s[_I]; _I++) fputc(s[_I], stdout); fputc('\n', stdout);}

int main(void){

    ParseNode* tree = parse("(a(a)(a))(a)(aa(((a))))");
    print_parse_tree(tree);
    size_t groups = label_groups(tree);
    printf("Found %zu groups\n", groups);
    printf("-------------- After simplification ---------------\n");
    simplify_tree(tree);
    print_parse_tree(tree);
    free_parse_tree(tree);

    printf("done");
    return 0;
}