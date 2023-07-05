#include "regex_parse.h"
#include "char_class.h"
#include "../strrepl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define STACK_SIZE 3

// Create parse tree

ParseNode new_alternate(ParseNode* left, ParseNode* right, ParseNode* parent){
    ParseNode** children = (ParseNode**)calloc(2, sizeof(ParseNode*));
    children[0] = left;
    children[1] = right;
    return (ParseNode){.type=ALTERNATE, .children=children, .child_count = 2, ._child_arr_size=2, .parent=parent,
                        .should_free_value=0};
}

ParseNode new_quantifier(RegexRules type, ParseNode* child, ParseNode* parent){
    ParseNode** children = (ParseNode**)calloc(1, sizeof(ParseNode*));
    children[0] = child;
    return (ParseNode){.type=type, .children=children, .child_count = 1, ._child_arr_size=1, .parent=parent,
                        .should_free_value=0};
}

ParseNode new_literal(char* str, ParseNode* parent){
    return (ParseNode){.type=LITERAL, .value=str, .child_count=0, ._child_arr_size=0, .parent=parent,
                        .should_free_value=1};
}

ParseNode new_wildcard(ParseNode* parent){
    return (ParseNode){.type=WILDCARD, .value=NULL, .child_count=0, ._child_arr_size=0, .parent=parent,
                        .should_free_value=0};
}

ParseNode new_char_class(uint64_t allowed_chars[4], ParseNode* parent, bool inverted){
    ParseNode node = (ParseNode){.type = inverted ? INV_CHAR_CLASS : CHAR_CLASS,
                        .child_count = 0,
                        ._child_arr_size = 0, 
                        .children = NULL,
                        .parent = parent,
                        .should_free_value = 0,
                        };
    
    memcpy(node.value.in_class, allowed_chars, sizeof(uint64_t) * 4);
    return node;
}

size_t parse_int(char** ptr){
    size_t out = 0;
    while ('0' <= **ptr && **ptr <= '9'){
        printf("Character parsing as int: %c\n", **ptr);
        out = out * 10 + **ptr - '0';
        (*ptr)++;
    }
    return out;
}

ParseNode* parse_repeat(char** ptr, ParseNode* child, ParseNode* parent){
    char* token_ptr = *ptr;
    
    ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
    token_ptr++;
    while (*token_ptr == ' ') token_ptr++;
    ASSERT_NOT(!('0' <= *token_ptr && *token_ptr <= '9'), 
                "Repeat lower bound must be a positive integer, but found the character '%c'", *token_ptr);

    node->value.bounds[0] = parse_int(&token_ptr);
    while (*token_ptr == ' ') token_ptr++;

    ASSERT_NOT(*token_ptr != ',' && *token_ptr != '}', 
                "Repeat lower bound must be a positive integer, but found the character '%c'", *token_ptr);
    if (*token_ptr == '}'){
        // Actually is REPEAT_CONST
        node->type = REPEAT_CONST;
    } else if (*token_ptr == ',' && *(token_ptr + 1) == '}'){
        node->type = REPEAT_UNBOUNDED;
        token_ptr++;
    } else {
        token_ptr++;
        node->type = REPEAT_BOUNDED;
        while (*token_ptr == ' ') token_ptr++;
        node->value.bounds[1] = parse_int(&token_ptr);
        while (*token_ptr == ' ') token_ptr++;
        ASSERT_NOT(*token_ptr != '}', \
                "Repeat upper bound must be a positive integer, but found the character '%c'", *token_ptr);
    }

    node->child_count = 1;
    node->_child_arr_size = 1;
    node->children = (ParseNode**)malloc(sizeof(ParseNode));
    node->children[0] = child;

    // Check lazy
    if (*(token_ptr + 1) == '?'){
        token_ptr++;
        node->lazy = true;
    } else {
        node->lazy = false;
    }

    node->should_free_value = 0;
    node->parent = parent;

    *ptr = token_ptr;

    return node;
}

ParseNode* parse(char* regex){
    char* token_ptr = regex;

    RegexRules rules[STACK_SIZE];
    rules[0] = END;
    RegexRules* rules_top = &rules[0];

    ParseNode *tree = (ParseNode*)malloc(sizeof(ParseNode));
    *tree = (ParseNode){.type=EXPR, .children=(ParseNode**)calloc(10, sizeof(ParseNode*)), .child_count=0, ._child_arr_size=10};
    ParseNode* current = tree;
    bool is_parsing = true;
    int paren_balance = 0;

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
                case ESCAPE_CHR:
                    *(++rules_top) = ESCAPE;
                    break;
                case '*':
                    *(++rules_top) = ZERO_OR_MORE;
                    break;
                case '+':
                    *(++rules_top) = ONE_OR_MORE;
                    break;
                case '?':
                    *(++rules_top) = ZERO_OR_ONE;
                    break;
                case '|':
                    *(++rules_top) = ALTERNATE;
                    break;
                case '(':
                    paren_balance++;
                    *(++rules_top) = GROUP;
                    break;
                case ')':
                    paren_balance--;
                    *(++rules_top) = GROUP_END;
                    break;
                case '{':
                    // Using REPEAT_CONST as a placeholder for all repeat types
                    *(++rules_top) = REPEAT_CONST;
                    break;
                case '[':
                    if (*(token_ptr + 1) == '^'){
                        *(++rules_top) = INV_CHAR_CLASS;
                        token_ptr++;
                    } else {
                        *(++rules_top) = CHAR_CLASS;
                    }
                    token_ptr++;
                    break;
                case '.':
                    *(++rules_top) = WILDCARD;
                    break;
                default:
                    *(++rules_top) = LITERAL;
                    break;
            }
        }
        // // For debugging rule stack
        // for (RegexRules* i = &rules[0]; i <= rules_top; i++){
        //     printf("%d ", *i);
        // }
        // if (*token_ptr) printf("With char %c", *token_ptr);
        // printf("\n");

        ASSERT_NOT(rules_top >= rules + STACK_SIZE, "Stack overflow on rules");

        // Apply a rule from rule stack
        switch(*(rules_top--)){
            case END:
                is_parsing = false;
                break;
            case LITERAL: {
                ParseNode* node = malloc(sizeof(ParseNode));
                char* s = (char*)calloc(2, sizeof(char));
                s[0] = *token_ptr;
                *node = new_literal(s, current);
                current->children[current->child_count++] = node;
                break;
            }
            case ESCAPE: {
                token_ptr++;
                current->children[current->child_count++] = parse_escaped(&token_ptr, current);
                break;
            }
            case WILDCARD: {
                ParseNode* node = malloc(sizeof(ParseNode));
                *node = new_wildcard(current);
                current->children[current->child_count++] = node;
                break;
            }
            case CHAR_CLASS: {
                current->children[current->child_count++] = parse_char_class(&token_ptr, current, false);
                break;
            }
            case INV_CHAR_CLASS: {
                current->children[current->child_count++] = parse_char_class(&token_ptr, current, true);
                break;
            }
            case ZERO_OR_ONE:
            case ONE_OR_MORE:
            case ZERO_OR_MORE: {
                ASSERT_NOT(current->child_count == 0, "Quantifier at postion %lld must be preceded by another expression", token_ptr - regex);
                ParseNode* node = malloc(sizeof(ParseNode));
                *node = new_quantifier(*(rules + 1), current->children[current->child_count - 1], current);
                if (*(token_ptr + 1) == '?'){
                    token_ptr++;
                    node->lazy = true;
                } else {
                    node->lazy = false;
                }
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
            case REPEAT_CONST: {
                ParseNode* node = parse_repeat(&token_ptr, current->children[current->child_count - 1], current);
                current->children[current->child_count - 1]->parent = node;
                current->children[current->child_count - 1] = node;
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

    ASSERT_NOT(paren_balance, "Parentheses are not balanced in expression");

    return tree;

}

void free_parse_tree(ParseNode* tree){
    if (!tree){
        fprintf(stderr, "Warning: Trying to free NULL tree\n");
        return;
    }
    if (tree->children){
        for (int i = 0; i < tree->child_count; i++){
            if (!tree->children[i]){
                fprintf(stderr, "Warning: Trying to free NULL child\n");
                continue;
            }
            free_parse_tree(tree->children[i]);
        }
        free(tree->children);
    }
    if (tree->should_free_value && tree->value.str){
        free(tree->value.str);
    }
    free(tree);
}

void free_tree_not_children(ParseNode* tree){
    if (!tree){
        fprintf(stderr, "Warning: Trying to free NULL tree\n");
        return;
    }
    if (tree->should_free_value && tree->value.str){
        free(tree->value.str);
    }
    if (tree->children){
        free(tree->children);
    }
    free(tree);
}

void _print_parse_tree(ParseNode* tree, int indent){

    for (int i = 0; i < indent; i++){
        printf("\t");
    }

    switch (tree->type){
        case WILDCARD:
            printf("Wildcard");
            break;
        case LITERAL:
            printf("Literal('%s')", tree->value.str);
            break;
        case ZERO_OR_MORE:
            printf("Zero or more (lazy = %d)", tree->lazy);
            break;
        case ZERO_OR_ONE:
            printf("Zero or one (lazy = %d)", tree->lazy);
            break;
        case ONE_OR_MORE:
            printf("One or more (lazy = %d)", tree->lazy);
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
        case REPEAT_CONST:
            printf("Const Repeat {%zd} (lazy = %d)", tree->value.bounds[0], tree->lazy);
            break;
        case REPEAT_BOUNDED:
            printf("Bounded Repeat {%zd, %zd} (lazy = %d)", tree->value.bounds[0], tree->value.bounds[1], tree->lazy);
            break;
        case REPEAT_UNBOUNDED:
            printf("Unbounded Repeat {%zd, } (lazy = %d)", tree->value.bounds[0], tree->lazy);
            break;
        case INV_CHAR_CLASS:
        case CHAR_CLASS: {
            if (tree->type == INV_CHAR_CLASS){
                printf("Inverted ");
            }
            printf("Character Class: [");

            for (int i = 0; i < 256; i++){
                if (!contains_char(tree->value.in_class, (char)i)) continue;
                if (32 <= i && i < 127){
                    printf("%c", (char)i);
                } else if (i == 10){
                    printf("\\n");
                } else {
                    printf("\\x%02X", i);
                }
            }
            
            printf("]");
            break;
        }
        default:
            ASSERT_NOT(1, "Unknown Regex Rule '%d'\n", tree->type);
            break;
    }

    printf(" (@%zX)\n", tree);

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

