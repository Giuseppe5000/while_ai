#ifndef WHILE_AI_PARSER_
#define WHILE_AI_PARSER_

#include "lexer.h"
#include <stdio.h>
#include <stdbool.h>

// Syntactic Categories for While language.
//
// numerals:
//     n ∈ Num (integers)
//
// variables:
//     x ∈ Var
//
// arithmetic expressions:
//     a ∈ Aexp
//     a ::= n | x | a1 + a2 | a1 - a2 | a1 * a2 | a1 / a2
//
// boolean expressions:
//     b ∈ Bexp
//     b ::= true | false | a1 = a2 | a1 <= a2 | !b | b1 & b2
//
// statements:
//     S ∈ Stm
//     S ::= x := a | skip | S1;S2 | if b then S1 else S2 fi | while b do S done


enum Node_Type {
    // Leaf nodes
    NODE_NUM,
    NODE_VAR,
    NODE_BOOL_LITERAL,

    // Arithmetic expr
    NODE_PLUS,
    NODE_MINUS,
    NODE_MULT,
    NODE_DIV,

    // Boolean expr
    NODE_EQ,
    NODE_LEQ,
    NODE_NOT,
    NODE_AND,

    // Statements
    NODE_ASSIGN,
    NODE_SKIP,
    NODE_SEQ,
    NODE_IF,
    NODE_WHILE,
};

typedef struct AST_Node AST_Node;

struct AST_Node {
    enum Node_Type type;
    union {
        // Inner node attributes
        struct {
            AST_Node *left;
            AST_Node *right;
            AST_Node *condition; // Used for while and if condition
        } child;

        // Leaf node attributes
        String var;
        int64_t num;
        bool boolean;
    } as;
};

// Parse the program according to the grammar.
// Returns the root node of the AST.
AST_Node *parser_parse(Lexer *lex);

// Prints the ast through 'fp' (as S-expression)
void parser_print_ast(const AST_Node *node, FILE *fp);

// Returns an dynamic allocated copy of the tree in 'node'
AST_Node *parser_copy_node(const AST_Node *node);

// Free the AST node subtree
void parser_free_ast_node(AST_Node *node);

#endif // WHILE_AI_PARSER_
