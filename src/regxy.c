#include "strrepl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PrintRange(s, start, end) {for (int _I = ((start >= 0) ? start : 0); _I < end && s[_I]; _I++) fputc(s[_I], stdout); fputc('\n', stdout);}

int main(void){

    char* cont = fread_all("testdata/words_alpha.txt");
    char* r_cont = NULL;

    clock_t start = clock();
    Repl repls[10000];
    size_t num_repls = sizeof(repls)/sizeof(Repl);

    printf("Time: %d or %d\n", (clock() - start), (clock() - start)/CLOCKS_PER_SEC);
    
    for (int i = 0; i < num_repls; i++){
        repls[i] = (Repl){
                        .start = strlen(cont) / num_repls * i,
                        .end = strlen(cont) / num_repls * i + (rand() % (strlen(cont) / num_repls) - 1) + 1,
                        .repl = "--------------------"};
    }

    r_cont = str_multi_replace(cont, repls, num_repls, 1);


    FILE* fp = fopen("output.txt.out", "w+");
    fwrite(r_cont, sizeof(char), strlen(r_cont), fp);
    fclose(fp);
    free(fp);

    free(r_cont);
    free(cont);
    printf("done");
    return 0;
}