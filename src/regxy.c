#include "strrepl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PrintRange(s, start, end) {for (int _I = ((start >= 0) ? start : 0); _I < end && s[_I]; _I++) fputc(s[_I], stdout); fputc('\n', stdout);}

int main(void){

    char* cont = fread_all("testdata/words_alpha.txt");
    char* r_cont = NULL;

    // // // Test naive string replacement
    // srand(10);
    // // for (int i = 0; i < 1000; i++){
    // //     size_t len = strlen(cont);
    // //     int rstart = rand() % len;
    // //     int rend = rstart + ((rand() % (len - rstart)) % 2);
    // //     // printf("(%d, %d) - Size: %d\n", rstart, rend, len);
    // //     r_cont = str_replaced(cont, rstart, rend, "");

    // //     free(cont);
    // //     cont = r_cont;
    // // }

    Repl repls[10000];
    size_t num_repls = sizeof(repls)/sizeof(Repl);
    

    for (int i = 0; i < num_repls; i++){
        repls[i] = (Repl){
                        .start = strlen(cont) / num_repls * i,
                        .end = strlen(cont) / num_repls * i + (rand() % (strlen(cont) / num_repls) - 1) + 1,
                        .repl = "--------------------"};
        // printf("%d-%d -> '%s'\n", repls[i].start, repls[i].end, repls[i].repl);
    }

    r_cont = str_multi_replace(cont, repls, num_repls);

    // FILE* fp = fopen("output.txt.out", "w+");
    // fwrite(r_cont, sizeof(char), strlen(r_cont), fp);
    // fclose(fp);
    // free(fp);

    free(r_cont);
    free(cont);
    return 0;
}