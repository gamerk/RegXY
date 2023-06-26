#ifndef CHAR_CLASS_H
#define CHAR_CLASS_H

#include "regex_parse.h"

ParseNode* parse_escaped(char** ptr, ParseNode* parent);
ParseNode* parse_char_class(char** ptr, ParseNode* parent, bool inverted);
ParseNode* parse_escaped(char** ptr, ParseNode* parent);
void add_char(uint64_t (*allowed_chars)[4], char c);
void remove_char(uint64_t (*allowed_chars)[4], char c);
void add_char_range(uint64_t (*allowed_chars)[4], char start, char end);
void add_char_class(uint64_t (*dest)[4], uint64_t src[4]);
void add_node(uint64_t (*allowed_chars)[4], ParseNode* node);
bool contains_char(uint64_t allowed_chars[4], char c);
#endif
