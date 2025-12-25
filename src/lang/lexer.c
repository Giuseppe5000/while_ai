#include "lexer.h"
#include "../common.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

struct Lexer {
    const char *src;
    const char *cursor;
};

/* List of language keywords with associated type */
typedef struct {
    const char *keyword;
    enum Token_Type type;
} Keyword_Token;

const Keyword_Token keywords[] = {
    { .keyword = "+"     , .type = TOKEN_PLUS    },
    { .keyword = "-"     , .type = TOKEN_MINUS   },
    { .keyword = "*"     , .type = TOKEN_MULT    },
    { .keyword = "/"     , .type = TOKEN_DIV     },
    { .keyword = "true"  , .type = TOKEN_TRUE    },
    { .keyword = "false" , .type = TOKEN_FALSE   },
    { .keyword = "="     , .type = TOKEN_EQ      },
    { .keyword = "<="    , .type = TOKEN_LEQ     },
    { .keyword = "!"     , .type = TOKEN_NOT     },
    { .keyword = "&"     , .type = TOKEN_AND     },
    { .keyword = ":="    , .type = TOKEN_ASSIGN  },
    { .keyword = "skip"  , .type = TOKEN_SKIP    },
    { .keyword = ";"     , .type = TOKEN_SEMICOL },
    { .keyword = "if"    , .type = TOKEN_IF      },
    { .keyword = "then"  , .type = TOKEN_THEN    },
    { .keyword = "else"  , .type = TOKEN_ELSE    },
    { .keyword = "fi"    , .type = TOKEN_FI      },
    { .keyword = "while" , .type = TOKEN_WHILE   },
    { .keyword = "do"    , .type = TOKEN_DO      },
    { .keyword = "done"  , .type = TOKEN_DONE    },
};

const size_t keywords_len = sizeof(keywords) / sizeof(keywords[0]);

Lexer *lex_init(const char *src) {
    Lexer *lex = xmalloc(sizeof(Lexer));
    lex->src = src;
    lex->cursor = src;
    return lex;
}

static void skip_space(Lexer *lex) {
    while (isspace(*lex->cursor)) {
        lex->cursor++;
    }
}

/*
Check if 'kt.keyword' is pointed by the lexer cursor.
True case => returns the token associated with that keyword and updates the cursor.
Otherwise => returns a token with EOF type.
*/
static Token check_keyword(Lexer *lex, Keyword_Token kt) {
    Token t = {0};

    if (strncmp(lex->cursor, kt.keyword, strlen(kt.keyword)) == 0) {

        /*
        A variable like 'whileCounter' will be tokenize as 'while' and 'Counter'.
        Here I should check that after the keyword there is no alphanumeric char.
        */
        if (isalnum(kt.keyword[0]) && isalnum(lex->cursor[strlen(kt.keyword)])) {
            return t;
        }

        t.type = kt.type;
        t.as.str.name = lex->cursor;
        t.as.str.len = strlen(kt.keyword);

        lex->cursor += t.as.str.len;
    }

    return t;
}

Token lex_next(Lexer *lex) {
    Token t = {0};

    skip_space(lex);

    /* EOF */
    if (*lex->cursor == '\0') {
        return t;
    }

    /* Parse numbers (integers) */
    if (isdigit(*lex->cursor)) {
        const char *start = lex->cursor;

        /* Skip the number */
        while (isdigit(*lex->cursor)) {
            lex->cursor++;
        }

        t.type = TOKEN_NUM;
        t.as.num = atoi(start);
        return t;
    }

    /* Keywords */
    for (size_t i = 0; i < keywords_len; ++i) {
        t = check_keyword(lex, keywords[i]);

        if (t.type != TOKEN_EOF) {
            return t;
        }
    }

    /* Variables (alphanum and start with alpha) */
    if (isalpha(*lex->cursor)) {
        const char *start = lex->cursor;

        while (isalnum(*lex->cursor)) {
            lex->cursor++;
        }

        t.type = TOKEN_VAR;
        t.as.str.name = start;
        t.as.str.len = lex->cursor - start;
        return t;
    }

    /*TODO: Better to signal a syntax error */
    assert(0 && "UNREACHABLE");
}

Token lex_peek(Lexer *lex) {
    /* Get token and then rewind the cursor */
    const char *cursor = lex->cursor;
    Token t = lex_next(lex);
    lex->cursor = cursor;

    return t;
}

void lex_free(Lexer *lex) {
    free(lex);
}
