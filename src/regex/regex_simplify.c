#include "regex_simplify.h"
#include "regex_parse.h"
#include "../strrepl.h"
#include "char_class.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define ASSERT_TYPE(node, t) {if (node->type != t) {fprintf(stderr, "Node must have a type of %s (%d), but got type %d\n", #t, t, node); exit(EXIT_FAILURE);}}

void normalize_char_class(ParseNode* node){
    ASSERT_TYPE(node, INV_CHAR_CLASS);
    node->type = CHAR_CLASS;
    for (int i = 0; i < 4; i++){
        node->value.in_class[i] = ~node->value.in_class[i];
    }
}

void simplify_tree(ParseNode* tree){

    if (!tree) return;

    /*
    What do

    Make array shorter
    + Remove empty char classes (they exist)
    + Merge literals

    Make array longer
    - Convert repetitions into quantifiers (e.g. a{1,2} -> aa?, a{2,} -> aaa*)
    - Convert one-or-more quantifiers into zero-or-more quantifiers
    - Split up alternations if possible

    In-place
    - Convert alternations of single characters into character classes
    - Convert inverse character classes to just regular character classes
    */


    ParseNode** temp_children = (ParseNode**)calloc(tree->child_count, sizeof(ParseNode*));
    size_t temp_child_size = 0;

    for (size_t i = 0; i < tree->child_count; i++){
        ParseNode* child = tree->children[i];
        switch(child->type){
            case INV_CHAR_CLASS:
                normalize_char_class(child);
                if (!is_empty(child->value.in_class)){
                    temp_children[temp_child_size++] = child;
                } else {
                    free_parse_tree(child);
                }
                break;
            case CHAR_CLASS:
                if (!is_empty(child->value.in_class)){
                    temp_children[temp_child_size++] = child;
                } else {
                    free_parse_tree(child);
                }
                break;
            case LITERAL: {
                size_t literal_length = 0;
                ASSERT_NOT(!tree->children[i], "Child does not exist\n");
                for (size_t index = i; index < tree->child_count && tree->children[index]->type == LITERAL; index++){
                    literal_length += strlen(tree->children[index]->value.str);
                }

                char* new_str = (char*)calloc(literal_length + 1, sizeof(char));
                size_t char_on = 0;
                size_t start = i;

                while (i < tree->child_count && tree->children[i]->type == LITERAL){
                    strcpy(&new_str[char_on], tree->children[i]->value.str);
                    char_on += strlen(tree->children[i]->value.str);
                    i++;
                }
                
                // Might be faster to use one of the pre-existing nodes instead but eh
                ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
                *node = new_literal(new_str, tree);

                temp_children[temp_child_size++] = node;

                i--;
                break;
            }
            case REPEAT_CONST: {
                ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
                ASSERT_NOT(!node, "Could not allocate node for repeat const!");
                ASSERT_NOT(!child, "child is null!");
                node->type = EXPR;
                node->child_count = child->child_count * child->value.bounds[0];
                node->_child_arr_size = node->child_count;
                node->children = (ParseNode**)calloc(node->_child_arr_size, sizeof(ParseNode*));
                node->should_free_value = 0;

                for (size_t i = 0; i < node->child_count; i += child->child_count){
                    memcpy(&(node->children[i]), child->children, sizeof(ParseNode*) * child->child_count);
                }

                // free(child->children);
                // free(child);
                temp_children[temp_child_size++] = node;

                break;
            }
            default:
                temp_children[temp_child_size++] = child;
                break;
        }

    }

    // Recurse through all children (left to right)
    size_t new_child_size = 0;
    bool has_exprs = false;
    for (size_t i = 0; i < temp_child_size; i++){
        simplify_tree(temp_children[i]);
        if (temp_children[i]->type == EXPR){
            new_child_size += temp_children[i]->child_count;
            has_exprs = true;
        } else {
            new_child_size += 1;
        }
    }

    ParseNode** new_children = (ParseNode**)calloc(new_child_size, sizeof(ParseNode*));
    size_t new_child_pos = 0;

    for (size_t i = 0; i < temp_child_size; i++){
        if (temp_children[i]->type == EXPR){
            memcpy(&(new_children[new_child_pos]), temp_children[i]->children, sizeof(ParseNode*) * temp_children[i]->child_count);
            new_child_pos += temp_children[i]->child_count;
        } else {
            new_children[new_child_pos] = temp_children[i];
            new_child_pos += 1;
        }
    }

    free(temp_children);
    free(tree->children);
    tree->children = new_children;
    tree->_child_arr_size = new_child_size;
    tree->child_count = new_child_size;

    if (has_exprs){
        // Check if expanded expressions simplify any more
        simplify_tree(tree);
    }

}