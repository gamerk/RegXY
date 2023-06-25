#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "strrepl.h"

char* fread_all(const char* filename){
    FILE* fp = fopen(filename, "r");
    ASSERT_NOT(!fp, "Could not open file '%s'", filename);

    fseek(fp, 0L, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* content = (char*)calloc(fsize + 1, sizeof(char));
    if (!content){
        fclose(fp);
        free(fp);
        ASSERT_NOT(1, "Could not allocate content in fread_all");
    }

    fread(content, sizeof(char), fsize, fp);

    fclose(fp);

    return content;

}

char* str_replaced(const char* s, size_t start, size_t end, const char* repl){

    ASSERT_NOT(start > end, "Cannot replace range from %d to %d: start must be less than end", start, end);

    size_t new_size = strlen(s) + (strlen(repl) - (end - start));
    char* new_s = calloc(new_size + 1, sizeof(char));
    
    // Copy start of s to new_s
    strncpy(new_s, s, start);
    
    // Copy repl to new_s
    strcpy(new_s + start, repl);

    // Copy end of s to new_s
    strncpy(new_s + start + strlen(repl), s + end, strlen(s) - end);

    return new_s;
}

char* str_multi_replace(const char* s, Repl repls[], size_t count, int is_sorted){
    // Ranges cannot overlap
    
    ASSERT_NOT(!s, "Cannot multi-replace on an empty string");

    // Calculate final size and offset range indecies
    int offset = 0;
    Repl** ordered = (Repl**)calloc(count, sizeof(Repl*));


    if (!is_sorted){
        size_t min_start = SIZE_MAX;
        for (int i = 0; i < count; i++){
            if (repls[i].start < min_start){
                min_start = repls[i].start;
                ordered[0] = &repls[i];
            }
        }

        for (int j = 0; j < count - 1; j++){

            // Calculate new offset
            offset += strlen(ordered[j]->repl) - (ordered[j]->end - ordered[j]->start);

            min_start = SIZE_MAX;
            for (int i = 0; i < count; i++){

                if (&repls[i] == ordered[j]) continue;

                ASSERT_NOT(repls[i].start < ordered[j]->end && repls[i].end > ordered[j]->start, 
                        "Replacements cannot overlap: (%d-%d -> '%s') and (%d-%d -> '%s')", 
                        repls[i].start, repls[i].end, repls[i].repl,
                        ordered[j]->start, ordered[j]->end, ordered[j]->repl);

                if (ordered[j]->start < repls[i].start && repls[i].start < min_start){
                    min_start = repls[i].start;
                    ordered[j + 1] = &repls[i];
                }
            }
        }
    } else {
        for (int i = 0; i < count; i++){
            offset += strlen(repls[i].repl) - (repls[i].end - repls[i].start);
            ordered[i] = &repls[i];
        }
    }


    // New size is size of s + offset
    char* new_s = (char*)calloc(strlen(s) + offset, sizeof(char));
    ASSERT_NOT(!new_s, "New string could not be allocated in str_multi_replace!");
    size_t new_s_pos = 0;
    size_t s_pos = 0;

    for (int r = 0; r < count; r++){
        strncpy(new_s + new_s_pos, s + s_pos, ordered[r]->start - s_pos);
        new_s_pos += ordered[r]->start - s_pos;
        strcpy(new_s + new_s_pos, ordered[r]->repl);
        new_s_pos += strlen(ordered[r]->repl);
        s_pos = ordered[r]->end;
    }

    strncpy(new_s + new_s_pos, s + s_pos, strlen(s) - s_pos);

    free(ordered);

    return new_s;

}