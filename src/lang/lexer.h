#ifndef WHILE_AI_LEXER_
#define WHILE_AI_LEXER_

#include "../common.h"
#include <stddef.h>
#include <stdint.h>

enum Token_Type {
    TOKEN_EOF,
    TOKEN_NUM,
    TOKEN_VAR,
    TOKEN_OPAR,
    TOKEN_CPAR,

    // Aexp
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,

    // Bexp
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_EQ,
    TOKEN_LEQ,
    TOKEN_NOT,
    TOKEN_AND,

    // Stm
    TOKEN_ASSIGN,
    TOKEN_SKIP,
    TOKEN_SEMICOL,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_FI,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_DONE,
};

typedef struct {
    enum Token_Type type;
    union {
        String str;
        int64_t num;
    } as;
} Token;

typedef struct Lexer Lexer;

// Construct a new lexer.
// 'src' must be a string buffer and must be null terminated.
Lexer *lex_init(const char *src);

// Get next token
Token lex_next(Lexer *lex);

// Get next token, without consuming it
Token lex_peek(Lexer *lex);

// Free the lexer
void lex_free(Lexer *lex);

#endif // WHILE_AI_LEXER_
