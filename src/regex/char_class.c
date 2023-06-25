#include "char_class.h"
#include "regex_parse.h"
#include "../strrepl.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void add_char(uint64_t (*allowed_chars)[4], char c){
    (*allowed_chars)[(uint8_t)c >> 6] |= 1ULL << abs(c & 0b00111111);
}

void remove_char(uint64_t (*allowed_chars)[4], char c){
    (*allowed_chars)[(uint8_t)c >> 6] &= ~(1ULL << abs(c & 0b00111111));
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
    
    ASSERT_NOT(start >= end, "Start must be before end, but start = '%c'(%1$d) and end = '%c'(%2$d)", start, end);
    
    uint32_t index = ((uint8_t)start) >> 6;
    printf("Index for %c-%c: %d\n", start, end, index);
    start &= 0b00111111;
    end &= 0b00111111;
    uint64_t u = ~(uint64_t)0;
    (*allowed_chars)[index] |= ((u >> start) << start) & ~((u >> (end + 1)) << (end + 1));

}

void add_char_class(uint64_t (*dest)[4], uint64_t src[4]){
    *dest[0] |= src[0];
    *dest[1] |= src[1];
    *dest[2] |= src[2];
    *dest[3] |= src[3];
}

bool contains_char(uint64_t allowed_chars[4], char c){
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
        if (*(*ptr + 1) == '-'){
            char start = **ptr;
            char end = *(*ptr + 2);
            add_char_range(&(node->value.in_class), start, end);
            *ptr += 2;
        } else {
            add_char(&(node->value.in_class), **ptr);
        }
    }

    ASSERT_NOT(**ptr == '\0', "Unclosed char class\n");

    (*ptr)++;

    return node;
}