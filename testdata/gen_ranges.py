from os.path import getsize
from random import randrange
from textwrap import dedent

n = 10

if __name__ == "__main__":
    size = getsize("testdata/words_alpha.txt")
    
    max_range = size // n
    
    structs = []
    for i in range(n):
        structs.append(f"(Repl){{ .start={max_range * i}, .end={max_range * i + randrange(1, max_range)}, .repl=\"{'-' * randrange(0, 20)}\"  }}")
    
    repl_init = f"Repl repls[{n}] = {{ {','.join(structs)} }};"
    
    full_text = dedent(
        f"""
        #include "src/strrepl.h"
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        
        int main(void){{

            char* cont = fread_all("testdata/words_alpha.txt");
            char* r_cont = NULL;
            
            {repl_init}
            r_cont = str_multi_replace(cont, repls, {n}, 1);
            
            FILE* fp = fopen("output.txt.out", "w+");
            fwrite(r_cont, sizeof(char), strlen(r_cont), fp);
            fclose(fp);

            free(r_cont);
            free(cont);
            printf("done");
            return 0;
        }}
        
        """)
    
    test = open("repl_test.c", "w+")
    test.write(full_text)
    test.close()