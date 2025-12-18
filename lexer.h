#ifndef WHILE_AI_LEXER_
#define WHILE_AI_LEXER_

#include <stddef.h>
#include <stdint.h>

/*
Syntactic Categories for While language.

numerals:
    n ∈ Num

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

enum Token_Type {
    TOKEN_EOF,
    TOKEN_NUM,
    TOKEN_VAR,

    /* Aexp */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,

    /* Bexp */
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_EQ,
    TOKEN_LEQ,
    TOKEN_NOT,
    TOKEN_AND,

    /* Stm */
    TOKEN_ASSIGN,
    TOKEN_SKIP,
    TOKEN_SEMICOL,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
};

typedef struct {
    enum Token_Type type;
    union {
        struct {
            const char *str;
            size_t len;
        };
        int32_t num;
    };
} Token;

typedef struct Lexer Lexer;

/* Construct a new lexer */
Lexer *lex_init(const char *src);

/* Get next token */
Token lex_next(void);

/* Free the lexer */
void lex_free(Lexer *lex);

#endif /* WHILE_AI_LEXER_ */
