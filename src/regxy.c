#include "regex/regex_parse.h"
#include "regex/regex_simplify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// gcc src/*.c src/*.h src/regex/*.c src/regex/*.h -ggdb && a.exe
// cl src/*.c src/regex/*.c /Zi && regxy.exe

#define PrintRange(s, start, end) {for (int _I = ((start >= 0) ? start : 0); _I < end && s[_I]; _I++) fputc(s[_I], stdout); fputc('\n', stdout);}

int main(void){

    ParseNode* tree = parse("[^/D/d]a{2,}?bcd+");
    print_parse_tree(tree);
    simplify_tree(tree);
    printf("-------------- After simplification ---------------\n");
    print_parse_tree(tree);
    free_parse_tree(tree);

    printf("done");
    return 0;
}