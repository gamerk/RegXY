#include "char_class.h"
#include "regex_parse.h"
#include "../strrepl.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void add_char(uint64_t (*allowed_chars)[4], char c){
    *allowed_chars[c >> 6] |= 1ULL << abs(c & 0b00111111);
}

void remove_char(uint64_t (*allowed_chars)[4], char c){
    *allowed_chars[c >> 6] &= ~(1ULL << abs(c & 0b00111111));
}

void add_char_range(uint64_t (*allowed_chars)[4], char start, char end){
    ASSERT_NOT(!('0' <= start && start <= '9' || 'a' <= start && start <= 'z' || 'A' <= start && start <= 'Z'),
                "Start of range must be in 0-9, a-z, or A-Z, not '%c'\n", start);
    ASSERT_NOT(!('0' <= end && end <= '9' || 'a' <= end && end <= 'z' || 'A' <= end && end <= 'Z'),
                "End of range must be in 0-9, a-z, or A-Z, not '%c'\n", end);

    ASSERT_NOT(!('0' <= start && start <= '9' && '0' <= end && end <= '9'
                || 'a' <= start && start <= 'z' && 'a' <= end && end <= 'z'
                || 'A' <= start && start <= 'Z' && 'A' <= end && end <= 'Z'),
                "Start and end or range must both be in 0-9, a-z, or A-Z, but got range '%c' to '%c'",
                start, end);
    
    start &= 0b00111111;
    end &= 0b00111111;
    uint64_t u = ~0;
    *allowed_chars[start >> 6] = ((u >> start) << start) & ~((u >> end) << end);

}

void add_char_class(uint64_t (*dest)[4], uint64_t src[4]){
    *dest[0] |= src[0];
    *dest[1] |= src[1];
    *dest[2] |= src[2];
    *dest[3] |= src[3];
}

bool contains_char(uint64_t allowed_chars[4], char c){
    // printf("allowed_chars[%d](=%zX) & (1 << %d)(=%zX)\n", c >> 6, allowed_chars[c >> 6], abs(c & 0b00111111), (1ULL << abs(c & 0b00111111)));
    return (bool)(allowed_chars[((uint8_t)c) >> 6] & (1ULL << abs(c & 0b00111111)));
}

ParseNode* parse_escaped(char** ptr, ParseNode* parent){
    // Assuming we start after the escape character
    ParseNode* out = (ParseNode*) malloc(sizeof(ParseNode));

    switch(**ptr){
        case 'n':
            *out = new_literal("\n", parent);
            break;
        case 't':
            *out = new_literal("\t", parent);
            break;
        default:
            char* s = (char*)calloc(2, sizeof(char));
            s[0] = **ptr;
            s[1] = 0;
            *out = new_literal(s, parent);
            break;
    }

    *ptr++;
}

ParseNode* parse_char_class(char** ptr, ParseNode* parent, bool inverted){
    // Assuming we start after the '[' or '[^'

    ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
    *node = new_char_class((uint64_t[]){0,0,0,0}, parent, inverted);
    
    for (; **ptr != ']' && **ptr != '\0'; (*ptr)++){
        printf("From pointer: %s\n", *ptr);
        printf("Adding char '%c'(%1$d)\n", **ptr);
        add_char(&(node->value.in_class), **ptr);
    }

    ASSERT_NOT(**ptr == '\0', "Unclosed char class\n");

    *ptr++;

    return node;
}