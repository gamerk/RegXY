#ifndef REGEX_SIMPLIFY_H
#define REGEX_SIMPLIFY_H

#include "regex_parse.h"
#include <stddef.h>
#include <stdbool.h>

void normalize_char_class(ParseNode* node);
ParseNode** copy_children(ParseNode** children, size_t num_children, ParseNode* parent);
ParseNode* copy_tree(ParseNode* node);
ParseNode* expand_repeat(ParseNode** to_repeat, size_t tr_size, size_t num_required, size_t num_optional, bool bounded, bool lazy, ParseNode* parent);

void simplify_tree(ParseNode* tree);

size_t label_groups(ParseNode* tree);
void _label_groups(ParseNode* tree, size_t* gnum);

#endif /* REGEX_SIMPLIFY_H */
