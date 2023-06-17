#define PCRE2_CODE_UNIT_WIDTH 8
#define LINK_SIZE 2

#include "strrepl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PrintRange(s, start, end) {for (int _I = ((start >= 0) ? start : 0); _I < end && s[_I]; _I++) fputc(s[_I], stdout); fputc('\n', stdout);}

int main(void){
    char* cont = fread_all("words_alpha.txt");
    char* r_cont = NULL;

    // // Test naive string replacement
    // srand(10);
    // for (int i = 0; i < 1000; i++){
    //     size_t len = strlen(cont);
    //     int rstart = rand() % len;
    //     int rend = rstart + ((rand() % (len - rstart)) % 2);
    //     // printf("(%d, %d) - Size: %d\n", rstart, rend, len);
    //     r_cont = str_replace(cont, rstart, rend, "");

    //     free(cont);
    //     cont = r_cont;
    // }

    // FILE* fp = fopen("output.txt", "w+");
    // fwrite(cont, sizeof(char), strlen(cont), fp);
    // fclose(fp);
    // free(fp);

    SChunk* sc = str2schunk(cont);
    int thing = 0;
    for (int i = 0; i < strlen(cont); i++){
        thing += get_char(sc, i);
        // thing += cont[i];
        thing %= 10;
    }
    printf("\n");
    free(sc);

    free(cont);
    return 0;
}