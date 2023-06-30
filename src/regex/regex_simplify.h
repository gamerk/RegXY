#ifndef REGEX_SIMPLIFY_H
#define REGEX_SIMPLIFY_H

#include "regex_parse.h"

void normalize_char_class(ParseNode* node);

void simplify_tree(ParseNode* tree);

#endif /* REGEX_SIMPLIFY_H */
