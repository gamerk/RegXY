
#include "src/strrepl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void){

    char* cont = fread_all("testdata/words_alpha.txt");
    char* r_cont = NULL;

    Repl repls[10] = { (Repl){ .start=0, .end=370842, .repl="-----"  },(Repl){ .start=386481, .end=474025, .repl="--------------"  },(Repl){ .start=772962, .end=802137, .repl="------------"  },(Repl){ .start=1159443, .end=1379163, .repl="----"  },(Repl){ .start=1545924, .end=1795052, .repl="---------"  },(Repl){ .start=1932405, .end=1951456, .repl="-----------------"  },(Repl){ .start=2318886, .end=2495446, .repl="-----"  },(Repl){ .start=2705367, .end=2994790, .repl="------------"  },(Repl){ .start=3091848, .end=3331195, .repl="------"  },(Repl){ .start=3478329, .end=3536063, .repl="------------------"  } };
    r_cont = str_multi_replace(cont, repls, 10, 1);

    FILE* fp = fopen("output.txt.out", "w+");
    fwrite(r_cont, sizeof(char), strlen(r_cont), fp);
    fclose(fp);
    free(fp);

    free(r_cont);
    free(cont);
    printf("done");
    return 0;
}

