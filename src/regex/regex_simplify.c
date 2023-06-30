#include "regex_simplify.h"
#include "regex_parse.h"
#include "../strrepl.h"
#include <stdio.h>
#include <stdlib.h>

#define ASSERT_TYPE(node, t) {if (node->type != t) {fprintf(stderr, "Node must have a type of %s (%d), but got type %d\n", #t, t, node); exit(EXIT_FAILURE);}}

void normalize_char_class(ParseNode* node){
    ASSERT_TYPE(node, INV_CHAR_CLASS);
    node->type = CHAR_CLASS;
    for (int i = 0; i < 4; i++){
        node->value.in_class[i] = ~node->value.in_class[i];
    }
}

void simplify_tree(ParseNode* tree){
    // Simplify tree
    switch(tree->type){
        case INV_CHAR_CLASS:
            normalize_char_class(tree);
            break;
    }
    // Simplify children of tree (as a whole)
    // Recurse through all children (left to right)
    for (int i = 0; i < tree->child_count; i++){
        simplify_tree(tree->children[i]);
    }

    // TODO: Realloc child arrays to save space?
}