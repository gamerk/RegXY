#include <stddef.h>

typedef enum RegexRules {
    LITERAL,
    ZERO_OR_MORE,
    ALTERNATE,
    EXPR,
    GROUP,
    GROUP_END,
    END
} RegexRules;

typedef union NodeValue {
    char* str;
    size_t bounds[2];
} NodeValue;

typedef struct ParseNode {
    RegexRules type;
    NodeValue value;
    struct ParseNode** children;
    size_t child_count;
    size_t _child_arr_size;
    struct ParseNode* parent;
    char should_free_value;
} ParseNode;

ParseNode new_alternate(ParseNode* left, ParseNode* right, ParseNode* parent);
ParseNode new_zero_or_more(ParseNode* child, ParseNode* parent);
ParseNode new_literal(char* str, ParseNode* parent);
ParseNode* parse(char* regex);
void free_parse_tree(ParseNode* tree);
void _print_parse_tree(ParseNode* tree, int indent);
void print_parse_tree(ParseNode* tree);
// char** tokenize(const char* s, int* length);