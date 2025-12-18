#include "lexer.h"
#include "utils.h"

struct Lexer {
    const char *src;
    const char *cursor;
};

Lexer *lex_init(const char *src) {
    Lexer *lex = xmalloc(sizeof(Lexer));
    lex->src = src;
    lex->cursor = src;
    return lex;
}

Token lex_next(void) {
    /* TODO */
}

void lex_free(Lexer *lex) {
    /* TODO */
}
