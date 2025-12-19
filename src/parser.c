#include "parser.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

struct Parser {
    Lexer *lex;
    AST_Node *root;
};

Parser *parser_init(const char *src) {
    Parser *parser = xmalloc(sizeof(Parser));
    parser->lex = lex_init(src);
    parser->root = NULL;

    return parser;
}

void parser_free(Parser *parser) {
    lex_free(parser->lex);
    /* TODO: free AST */
    free(parser);
}

static void parser_print_ast_impl(AST_Node *node, int indent) {
    if (node->type == NODE_NUM) {
        printf("%*s%d\n", indent, "", node->as.num);
    }
    else if (node->type == NODE_VAR) {
        printf("%*s%.*s\n", indent, "", (int)node->as.var.len, node->as.var.str);
    }
    else if (node->type == NODE_BOOL_LITERAL) {
        if (node->as.boolean) {
            printf("%*s(true)\n", indent, "");
        } else {
            printf("%*s(false)\n", indent, "");
        }
    }
    else {
        switch (node->type) {
        case NODE_NUM:
        case NODE_VAR:
        case NODE_BOOL_LITERAL:
            break;
        case NODE_PLUS:
            printf("%*s+\n", indent, "");
            break;
        case NODE_MINUS:
            printf("%*s-\n", indent, "");
            break;
        case NODE_MULT:
            printf("%*s*\n", indent, "");
            break;
        case NODE_DIV:
            printf("%*s/\n", indent, "");
            break;
        case NODE_EQ:
            printf("%*s=\n", indent, "");
            break;
        case NODE_LEQ:
            printf("%*s<=\n", indent, "");
            break;
        case NODE_NOT:
            printf("%*s!\n", indent, "");
            break;
        case NODE_AND:
            printf("%*s&\n", indent, "");
            break;
        case NODE_ASSIGN:
            printf("%*s:=\n", indent, "");
            break;
        case NODE_SKIP:
            printf("%*sskip\n", indent, "");
            break;
        case NODE_SEQ:
            printf("%*s;\n", indent, "");
            break;
        case NODE_IF:
            printf("%*sif\n", indent, "");
            parser_print_ast_impl(node->as.child.condition, indent + 2);
            break;
        case NODE_WHILE:
            printf("%*swhile\n", indent, "");
            parser_print_ast_impl(node->as.child.condition, indent + 2);
            break;
        }

        parser_print_ast_impl(node->as.child.left, indent + 2);
        if (node->type != NODE_WHILE) parser_print_ast_impl(node->as.child.right, indent + 2);
    }
}

void parser_print_ast(AST_Node *node) {
    parser_print_ast_impl(node, 0);
}


/* ============================= Recursive descent parser ============================= */
/* https://en.wikipedia.org/wiki/Recursive_descent_parser */

static AST_Node *parse_stmt(Parser *parser);
static AST_Node *parse_bexp(Parser *parser);

/* Alloc a zero initialized AST node */
static AST_Node *create_node(enum Node_Type type) {
    AST_Node *node = xmalloc(sizeof(AST_Node));
    memset(node, 0, sizeof(AST_Node));
    node->type = type;
    return node;
}

static void expect(Token t, enum Token_Type type) {
    if (t.type != type) {
        fprintf(stderr, "[ERROR]: Expected type %d but found %d while parsing.", type, t.type);
        exit(1);
    }
}

static AST_Node *parse_factor_aexp(Parser *parser) {
    Token t = lex_next(parser->lex);

    /* Numeral */
    if (t.type == TOKEN_NUM) {
        AST_Node *num_node = create_node(NODE_NUM);
        num_node->as.num = t.as.num;

        return num_node;
    }

    /* Variable (a) */
    if (t.type == TOKEN_VAR) {
        AST_Node *var_node = create_node(NODE_VAR);
        var_node->as.var.str = t.as.str.data;
        var_node->as.var.len = t.as.str.len;

        return var_node;
    }

    fprintf(stderr, "[ERROR]: Unexpected token while parsing aexp\n");
    exit(1);
}

static AST_Node *parse_term_aexp(Parser *parser) {
    AST_Node *left = parse_factor_aexp(parser);

    /* * and / precedence */
    Token t = lex_peek(parser->lex);
    while (t.type == TOKEN_MULT || t.type == TOKEN_DIV) {
        lex_next(parser->lex);

        AST_Node *node = NULL;
        if (t.type == TOKEN_MULT) {
            node = create_node(NODE_MULT);
        } else {
            node = create_node(NODE_DIV);
        }

        node->as.child.left = left;
        node->as.child.right = parse_factor_aexp(parser);
        left = node;

        t = lex_peek(parser->lex);
    }

    return left;
}

static AST_Node *parse_aexp(Parser *parser) {
    AST_Node *left = parse_term_aexp(parser);

    /* - and + precedence */
    Token t = lex_peek(parser->lex);
    while (t.type == TOKEN_PLUS || t.type == TOKEN_MINUS) {
        lex_next(parser->lex);

        AST_Node *node = NULL;
        if (t.type == TOKEN_PLUS) {
            node = create_node(NODE_PLUS);
        } else {
            node = create_node(NODE_MINUS);
        }

        node->as.child.left = left;
        node->as.child.right = parse_term_aexp(parser);
        left = node;

        t = lex_peek(parser->lex);
    }

    return left;
}

static AST_Node *parse_atom_bexp(Parser *parser) {
    Token t = lex_peek(parser->lex);

    /* True */
    if (t.type == TOKEN_TRUE) {
        lex_next(parser->lex);
        AST_Node *bool_lit_node = create_node(NODE_BOOL_LITERAL);
        bool_lit_node->as.boolean = true;
        return bool_lit_node;
    }

    /* False */
    if (t.type == TOKEN_FALSE) {
        lex_next(parser->lex);
        AST_Node *bool_lit_node = create_node(NODE_BOOL_LITERAL);
        bool_lit_node->as.boolean = false;
        return bool_lit_node;
    }

    /* Not */
    if (t.type == TOKEN_NOT) {
        lex_next(parser->lex);
        AST_Node *not_node = create_node(NODE_NOT);
        not_node->as.child.left = parse_bexp(parser);
        return not_node;
    }

    /* It can be EQ or LEQ */
    AST_Node *left = parse_aexp(parser);
    t = lex_next(parser->lex);

    /* EQ */
    if (t.type == TOKEN_EQ) {
        AST_Node *eq_node = create_node(NODE_EQ);
        eq_node->as.child.left = left;
        eq_node->as.child.right = parse_aexp(parser);
        return eq_node;
    }

    /* LEQ */
    if (t.type == TOKEN_LEQ) {
        AST_Node *leq_node = create_node(NODE_LEQ);
        leq_node->as.child.left = left;
        leq_node->as.child.right = parse_aexp(parser);
        return leq_node;
    }

    fprintf(stderr, "[ERROR]: Unexpected token while parsing bexp\n");
    exit(1);
}

static AST_Node *parse_bexp(Parser *parser) {
    AST_Node *left = parse_atom_bexp(parser);

    Token t = lex_peek(parser->lex);
    while (t.type == TOKEN_AND) {
        lex_next(parser->lex);

        AST_Node *and_node = create_node(NODE_AND);
        and_node->as.child.left = left;
        and_node->as.child.right = parse_atom_bexp(parser);
        left = and_node;
    }

    return left;
}

static AST_Node *parse_atom_stmt(Parser *parser) {
    Token t = lex_next(parser->lex);

    /* Assignment */
    if (t.type == TOKEN_VAR) {

        /* Variable (a) */
        AST_Node *var_node = create_node(NODE_VAR);
        var_node->as.var.str = t.as.str.data;
        var_node->as.var.len = t.as.str.len;

        /* Assing symbol (:=) */
        t = lex_next(parser->lex);
        expect(t, TOKEN_ASSIGN);

        /* Aexp */
        AST_Node *assign_node = create_node(NODE_ASSIGN);
        assign_node->as.child.left = var_node;
        assign_node->as.child.right = parse_aexp(parser);

        return assign_node;
    }

    /* Skip */
    if (t.type == TOKEN_SKIP) {
        AST_Node *skip_node = create_node(NODE_SKIP);
        return skip_node;
    }

    /* If stmt */
    if (t.type == TOKEN_IF) {
        AST_Node *if_node = create_node(NODE_IF);

        /* Condition (b) */
        if_node->as.child.condition = parse_bexp(parser);

        /* Then symbol */
        t = lex_next(parser->lex);
        expect(t, TOKEN_THEN);

        /* S1 */
        if_node->as.child.left = parse_stmt(parser);

        /* Else symbol */
        t = lex_next(parser->lex);
        expect(t, TOKEN_ELSE);

        /* S2 */
        if_node->as.child.right = parse_stmt(parser);

        /* Fi symbol */
        t = lex_next(parser->lex);
        expect(t, TOKEN_FI);

        return if_node;
    }

    /* While stmt */
    if (t.type == TOKEN_WHILE) {
        AST_Node *while_node = create_node(NODE_WHILE);

        /* Condition (b) */
        while_node->as.child.condition = parse_bexp(parser);

        /* Do symbol */
        t = lex_next(parser->lex);
        expect(t, TOKEN_DO);

        /* S */
        while_node->as.child.left = parse_stmt(parser);

        /* Done symbol */
        t = lex_next(parser->lex);
        expect(t, TOKEN_DONE);

        return while_node;
    }

    fprintf(stderr, "[ERROR]: Unexpected token while parsing stmt\n");
    exit(1);
}

/* Parse the sequence statements */
static AST_Node *parse_stmt(Parser *parser) {
    AST_Node *left = parse_atom_stmt(parser);

    Token t = lex_peek(parser->lex);
    if (t.type == TOKEN_SEMICOL) {
        lex_next(parser->lex);
        AST_Node *node = create_node(NODE_SEQ);
        node->as.child.left = left;
        node->as.child.right = parse_stmt(parser);
        return node;
    }

    return left;
}

AST_Node *parser_parse(Parser *parser) {
    parser->root = parse_stmt(parser);
    return parser->root;
}

/* ==================================================================================== */
