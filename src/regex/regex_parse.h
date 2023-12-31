#ifndef REGEX_PARSE_H
#define REGEX_PARSE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ESCAPE_CHR '/'

typedef enum RegexRules {
    LITERAL = 1,
    ZERO_OR_MORE,
    ONE_OR_MORE,
    ZERO_OR_ONE,
    ALTERNATE,
    EXPR,
    GROUP,
    GROUP_END,
    REPEAT_CONST,
    REPEAT_BOUNDED,
    REPEAT_UNBOUNDED,
    CHAR_CLASS,
    INV_CHAR_CLASS,
    WILDCARD,
    ESCAPE,
    END
} RegexRules;

typedef union NodeValue {
    char* str;
    size_t bounds[2];
    uint64_t in_class[4];
    size_t group_num;
} NodeValue;

typedef struct ParseNode {
    RegexRules type;
    NodeValue value;
    struct ParseNode** children;
    size_t child_count;
    size_t _child_arr_size;
    struct ParseNode* parent;
    char should_free_value;
    bool lazy;
} ParseNode;

ParseNode new_alternate(ParseNode* left, ParseNode* right, ParseNode* parent);
ParseNode new_quantifier(RegexRules type, ParseNode* child, ParseNode* parent);
ParseNode new_literal(char* str, ParseNode* parent);
ParseNode new_char_class(uint64_t allowed_chars[4], ParseNode* parent, bool inverted);
ParseNode new_wildcard(ParseNode* parent);

ParseNode* parse(char* regex);
void free_parse_tree(ParseNode* tree);
void free_tree_not_children(ParseNode* tree);
void _print_parse_tree(ParseNode* tree, int indent);
void print_parse_tree(ParseNode* tree);
size_t parse_int(char** ptr);


#endif /* REGEX_PARSE_H */
