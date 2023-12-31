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

ParseNode** copy_children(ParseNode** children, size_t num_children, ParseNode* parent){
    ParseNode** copy = (ParseNode**)calloc(num_children, sizeof(ParseNode*));

    for (size_t i = 0; i < num_children; i++){
        // copy[i] = (ParseNode*)malloc(sizeof(ParseNode));
        copy[i] = copy_tree(children[i]);

        if (copy[i]->should_free_value){
            copy[i]->value.str = (char*)calloc(strlen(children[i]->value.str) + 1, sizeof(char));
            strcpy(copy[i]->value.str, children[i]->value.str);
        }

    }

    return copy;
}

ParseNode* copy_tree(ParseNode* node){
    ParseNode* copy = (ParseNode*)malloc(sizeof(ParseNode));
    *copy = *node;

    if (node->should_free_value){
        strcpy(copy->value.str, node->value.str);
    }

    if (node->_child_arr_size){
        copy->children = copy_children(node->children, node->child_count, copy);
    }

    return copy;
}

ParseNode* expand_repeat(ParseNode** to_repeat, size_t tr_size, size_t num_required, size_t num_optional, bool bounded, bool lazy, ParseNode* parent){
    ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
    ASSERT_NOT(!node, "Could not allocate node for repeat const!");

    node->type = EXPR;
    node->child_count = (tr_size * num_required) + (bounded ? num_optional : 1);
    node->_child_arr_size = node->child_count;
    node->children = (ParseNode**)calloc(node->_child_arr_size, sizeof(ParseNode*));
    node->should_free_value = 0;
    node->parent = parent;

    for (size_t i = 0; i < num_required * tr_size; i += tr_size){
        ParseNode** new_to_repeat = copy_children(to_repeat, tr_size, node);
        memcpy(&(node->children[i]), new_to_repeat, sizeof(ParseNode*) * tr_size);
        free(new_to_repeat);
    }

        
    if (!bounded){
        ParseNode* final = (ParseNode*)malloc(sizeof(ParseNode));
        final->type = ZERO_OR_MORE;
        final->should_free_value = false;
        final->parent = node;
        final->child_count = tr_size;
        final->_child_arr_size = tr_size;
        final->children = copy_children(to_repeat, tr_size, final);
        final->lazy = lazy;
        node->children[node->child_count - 1] = final;
    } else if (num_optional) {

        for (size_t i = num_required * tr_size; i < node->child_count; i++){
            ParseNode* zoo = (ParseNode*)malloc(sizeof(ParseNode));
            *zoo = (ParseNode){
                .type = ZERO_OR_ONE,
                .child_count = tr_size,
                ._child_arr_size = tr_size,
                .children = copy_children(to_repeat, tr_size, zoo),
                .lazy = lazy,
                .should_free_value = false,
            };
            node->children[i] = zoo;
        }
    }
    
    

    return node;

}

void simplify_tree(ParseNode* tree){

    if (!tree || !tree->children) return;

    /*
    What do

    Make array shorter
    + Remove empty char classes (they exist)
    + Merge literals (unless in alternations)

    Make array longer
    + Convert repetitions into quantifiers (e.g. a{1,2} -> aa{0,1}, a{2,} -> aaa*)
    + Convert one-or-more quantifiers into zero-or-more quantifiers
    + Expand alternations within alternations
    ? Split up alternations if possible

    In-place
    ? Convert alternations of single characters into character classes
    + Convert inverse character classes to just regular character classes
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
                if (tree->type == ALTERNATE) {
                    temp_children[temp_child_size++] = child;
                    break;
                }

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
                    free_parse_tree(tree->children[i]);
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
                
                temp_children[temp_child_size++] = expand_repeat(child->children, child->child_count, child->value.bounds[0], 
                                                        0, true, child->lazy, tree);
                free_parse_tree(child);

                break;
            }
            case REPEAT_BOUNDED: {
                
                if (child->value.bounds[0] != 0){
                    temp_children[temp_child_size++] = expand_repeat(child->children, child->child_count, child->value.bounds[0], 
                                                        child->value.bounds[1] - child->value.bounds[0],
                                                        true, child->lazy, tree);
                    free_parse_tree(child);
                } else {
                    temp_children[temp_child_size++] = child;
                }

                break;
            }
            case REPEAT_UNBOUNDED: {
                
                if (child->value.bounds[0] != 0){
                    temp_children[temp_child_size++] = expand_repeat(child->children, child->child_count, child->value.bounds[0], 
                                                        0, false, child->lazy, tree);
                    free_parse_tree(child);
                } else {
                    temp_children[temp_child_size++] = child;
                }

                break;
            }
            case ONE_OR_MORE: {
                ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
                *node = (ParseNode){
                    .type = EXPR,
                    .child_count = child->child_count + 1,
                    ._child_arr_size = child->child_count + 1,
                    .children = (ParseNode**)calloc(child->child_count + 1, sizeof(ParseNode*)),
                    .lazy = child->lazy,
                    .parent = tree,
                    .should_free_value = false
                };

                ParseNode** children_copy = copy_children(child->children, child->child_count, node);
                memcpy(&(node->children[0]), children_copy, child->child_count * sizeof(ParseNode*));
                free(children_copy);

                ParseNode* zom = (ParseNode*)malloc(sizeof(ParseNode));
                zom->type = ZERO_OR_MORE;
                zom->parent = node;
                zom->lazy = child->lazy;
                zom->should_free_value = 0;
                zom->child_count = child->child_count;
                zom->_child_arr_size = child->child_count;
                zom->children = copy_children(child->children, child->child_count, zom);
                node->children[child->child_count] = zom;

                temp_children[temp_child_size++] = node;
                free_parse_tree(child);

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
        if (temp_children[i]->type == EXPR && (tree->type != ALTERNATE || temp_children[i]->child_count == 1)
            || temp_children[i]->type == ALTERNATE && tree->type == ALTERNATE){
            new_child_size += temp_children[i]->child_count;
            has_exprs = true; 
        } else {
            new_child_size += 1;
        }
    }

    if (has_exprs){
        ParseNode** new_children = (ParseNode**)calloc(new_child_size, sizeof(ParseNode*));
        size_t new_child_pos = 0;

        for (size_t i = 0; i < temp_child_size; i++){
            if (temp_children[i]->type == EXPR && (tree->type != ALTERNATE || temp_children[i]->child_count == 1)
                || temp_children[i]->type == ALTERNATE && tree->type == ALTERNATE){
                
                memmove(&(new_children[new_child_pos]), temp_children[i]->children, sizeof(ParseNode*) * temp_children[i]->child_count);
                new_child_pos += temp_children[i]->child_count;
                free_tree_not_children(temp_children[i]);
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

        // Check if expanded expressions simplify any more
        simplify_tree(tree);
    } else {
        free(tree->children);
        tree->children = temp_children;
        tree->_child_arr_size = temp_child_size;
        tree->child_count = temp_child_size;
    }
    // print_parse_tree(tree);

}

size_t label_groups(ParseNode* tree){
    size_t group_num = 1;
    _label_groups(tree, &group_num);
    return group_num;
}

void _label_groups(ParseNode* tree, size_t* gnum){
    if (tree->type == GROUP){
        tree->value.group_num = *gnum;
        *gnum += 1;
    }

    for (size_t i = 0; i < tree->child_count; i++){
        _label_groups(tree->children[i], gnum);
    }
}