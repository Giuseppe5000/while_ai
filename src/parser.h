#ifndef WHILE_AI_PARSER_
#define WHILE_AI_PARSER_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
Syntactic Categories for While language.

numerals:
    n ∈ Num (integers)

variables:
    x ∈ Var

arithmetic expressions:
    a ∈ Aexp
    a ::= n | x | a1 + a2 | a1 - a2 | a1 * a2 | a1 / a2

boolean expressions:
    b ∈ Bexp
    b ::= true | false | a1 = a2 | a1 <= a2 | !b | b1 & b2

statements:
    S ∈ Stm
    S ::= x := a | skip | S1;S2 | if b then S1 else S2 | while b do S
*/

enum Node_type {
    /* Leaf nodes */
    NODE_NUM,
    NODE_VAR,
    NODE_BOOL_LITERAL,

    /* Arithmetic expr */
    NODE_PLUS,
    NODE_MINUS,
    NODE_MULT,
    NODE_DIV,

    /* Boolean expr */
    NODE_EQ,
    NODE_LEQ,
    NODE_NOT,
    NODE_AND,

    /* Statements */
    NODE_ASSIGN,
    NODE_SKIP,
    NODE_SEQ,
    NODE_IF,
    NODE_WHILE,
};

typedef struct AST_Node AST_Node;

struct AST_Node {
    enum Node_type type;
    union {
        /* Inner node attributes */
        struct {
            AST_Node *left;
            AST_Node *right;
            AST_Node *condition; /* Used for while and if condition */
        } children;

        /* Leaf node attributes */
        struct {
            const char *str;
            size_t len;
        } var;
        int32_t num;
        bool boolean;
    } as;
};

/*
Parse the program according to the grammar.
Returns the root node of the AST.
*/
AST_node *parser_init(Lexer *lex);

/* Free the AST */
void parser_free(AST_node *root);

#endif /* WHILE_AI_PARSER_ */
