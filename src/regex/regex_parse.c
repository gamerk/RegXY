#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "regex_parse.h"
#include "../strrepl.h"

#define STACK_SIZE 50

// Create parse tree

ParseNode new_alternate(ParseNode* left, ParseNode* right, ParseNode* parent){
    ParseNode** children = (ParseNode**)calloc(2, sizeof(ParseNode*));
    children[0] = left;
    children[1] = right;
    return (ParseNode){.type=ALTERNATE, .children=children, .child_count = 2, ._child_arr_size=2, .parent=parent,
                        .should_free_value=0};
}

ParseNode new_zero_or_more(ParseNode* child, ParseNode* parent){
    ParseNode** children = (ParseNode**)calloc(1, sizeof(ParseNode*));
    children[0] = child;
    return (ParseNode){.type=ZERO_OR_MORE, .children=children, .child_count = 1, ._child_arr_size=1, .parent=parent,
                        .should_free_value=0};
}

ParseNode new_literal(char* str, ParseNode* parent){
    return (ParseNode){.type=LITERAL, .value=str, .child_count=0, ._child_arr_size=0, .parent=parent,
                        .should_free_value=1};
}

ParseNode* parse(char* regex){
    char* token_ptr = regex;

    RegexRules rules[STACK_SIZE];
    rules[0] = END;
    RegexRules* rules_top = &rules[0];

    ParseNode *tree = (ParseNode*)malloc(sizeof(ParseNode));
    *tree = (ParseNode){.type=EXPR, .children=(ParseNode**)calloc(10, sizeof(ParseNode*)), .child_count=0, ._child_arr_size=10};
    ParseNode* current = tree;
    char is_parsing = 1;

    while (is_parsing){

        if (current->child_count >= current->_child_arr_size - 1 && current->children){
            ParseNode** curr_child = (ParseNode**)realloc(current->children, current->_child_arr_size * 2 * sizeof(ParseNode*));
            if (!curr_child){
                fputs("Warning! Realloc failed to create larger array for children. Returning parse tree so far", stderr);
                return tree;
            }
            current->children = curr_child;
            current->_child_arr_size *= 2;
        }

        // Add rules to rule stack
        if (*token_ptr){
            switch(*token_ptr){
                case '*':
                    *(++rules_top) = ZERO_OR_MORE;
                    break;
                case '|':
                    *(++rules_top) = ALTERNATE;
                    break;
                case '(':
                    *(++rules_top) = GROUP;
                    break;
                case ')':
                    *(++rules_top) = GROUP_END;
                    break;
                default:
                    *(++rules_top) = LITERAL;
                    break;
            }
        }

        for (RegexRules* i = &rules[0]; i <= rules_top; i++){
            printf("%d ", *i);
        }
        if (*token_ptr) printf("With char %c", *token_ptr);
        printf("\n");

        ASSERT_NOT(rules_top >= rules + STACK_SIZE, "Stack overflow on rules", NULL);

        // Apply a rule from rule stack
        switch(*(rules_top--)){
            case END:
                is_parsing = 0;
                break;
            case LITERAL: {
                ParseNode* node = malloc(sizeof(ParseNode));
                char* s = (char*)calloc(2, sizeof(char));
                s[0] = *token_ptr;
                *node = new_literal(s, current);
                current->children[current->child_count++] = node;
                break;
            }
            case ZERO_OR_MORE: {
                ASSERT_NOT(current->child_count == 0, "Zero or more at postion %d must be preceded by another expression", token_ptr - regex);
                ParseNode* node = malloc(sizeof(ParseNode));
                *node = new_zero_or_more(current->children[current->child_count - 1], current);
                current->children[current->child_count - 1]->parent = node;
                current->children[current->child_count - 1] = node;
                break;
            }
            case ALTERNATE: {
                ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
                *node = new_alternate(current, (ParseNode*)malloc(sizeof(ParseNode)), current->parent);
                
                if (current->parent){
                    for (int i = 0; i < current->parent->child_count; i++){
                        if (current->parent->children[i] == current){
                            current->parent->children[i] = node;
                            printf("Replaced %i\n", i);
                            break;
                        }
                    }
                }

                current->parent = node;
                current = node->children[1];
                *current = (ParseNode){.type = EXPR, .parent=node, .child_count=0, ._child_arr_size=10, .children=(ParseNode**)calloc(10, sizeof(ParseNode*))};
                break;
            }
            case GROUP: {
                // Creates a group node with only one child: an expr node
                ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
                ParseNode** children = (ParseNode**)calloc(1, sizeof(ParseNode*));
                *node = (ParseNode){.type=GROUP, .children=children, .child_count = 1, 
                                    ._child_arr_size=1, .parent=current,
                                    .should_free_value=0};
                children[0] = (ParseNode*)malloc(sizeof(ParseNode));
                *children[0] = (ParseNode){.type=EXPR, .children=(ParseNode**)calloc(1, sizeof(ParseNode*)),
                                            .child_count=0, ._child_arr_size=1, .parent=node};
                current->children[current->child_count++] = node;
                current = node->children[0];

                break;
            }
            case GROUP_END: {
                while (current->type != GROUP || !current->parent){
                    current = current->parent;
                }
                ASSERT_NOT(!current->parent, "Unmatched parenthesis at position %zd\n", token_ptr - regex);
                current = current->parent;
                break;
            }
            default:
                ASSERT_NOT(1, "Unknown Regex Rule '%d'\n", *(rules_top + 1));
                break;
        }

        while (tree->parent){
            tree = tree->parent;
        }

        if (*token_ptr){
            token_ptr++;
        }

    }

    return tree;

}

void free_parse_tree(ParseNode* tree){
    if (tree->value.str && tree->should_free_value){
        free(tree->value.str);
    }
    for (int i = 0; i < tree->child_count; i++){
        free_parse_tree(tree->children[i]);
    }
    free(tree->children);
    free(tree);
}

void _print_parse_tree(ParseNode* tree, int indent){

    for (int i = 0; i < indent; i++){
        printf("\t");
    }

    switch (tree->type){
        case LITERAL:
            printf("Literal('%s')", tree->value.str);
            break;
        case ZERO_OR_MORE:
            printf("Zero_or_more");
            break;
        case ALTERNATE:
            printf("Alternate");
            break;
        case EXPR:
            printf("Expression");
            break;
        case GROUP:
            printf("Group");
            break;
        default:
            ASSERT_NOT(1, "Unknown Regex Rule '%d'\n", tree->type);
            break;
    }

    printf("\n");

    for (int i = 0; i < tree->child_count; i++){
        // for (int i = 0; i < indent; i++){
        //     printf("\t");
        // }
        // printf("Child #%d@%zx(%1$zd)\n", i, *tree->children[i]);
        _print_parse_tree(tree->children[i], indent + 1);
    }

    if (tree->child_count){
        for (int i = 0; i < indent; i++){
            printf("\t");
        }
        printf("End\n");
    }
}

void print_parse_tree(ParseNode* tree){
    _print_parse_tree(tree, 0);
}

