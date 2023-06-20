#include <stddef.h>

typedef enum RegexRules {
    LITERAL,
    ZERO_OR_MORE,
    ALTERNATE,
    JOIN,
    EXPR,
    END
} RegexRules;

typedef struct ParseNode {
    RegexRules type;
    char* str;
    struct ParseNode** children;
    size_t child_count;
    size_t _child_arr_size;
    struct ParseNode* parent;
} ParseNode;

ParseNode new_alternate(ParseNode* left, ParseNode* right, ParseNode* parent);
ParseNode new_zero_or_more(ParseNode* child, ParseNode* parent);
ParseNode new_literal(char* str, ParseNode* parent);
ParseNode parse(char* regex);
void _print_parse_tree(ParseNode tree, int indent);
void print_parse_tree(ParseNode tree);
// char** tokenize(const char* s, int* length);