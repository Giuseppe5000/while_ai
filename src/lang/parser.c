#include "parser.h"
#include "../common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

AST_Node *create_node(enum Node_Type type) {
    AST_Node *node = xcalloc(1, sizeof(AST_Node));
    node->type = type;
    return node;
}

void parser_free_ast_node(AST_Node *node) {
    if (node != NULL) {

        // Leaf nodes
        if (node->type == NODE_NUM || node->type == NODE_VAR || node->type == NODE_BOOL_LITERAL) {
            free(node);
        }
        else {
            parser_free_ast_node(node->as.child.left);
            parser_free_ast_node(node->as.child.right);
            parser_free_ast_node(node->as.child.condition);
            free(node);
        }
    }
}

static void parser_print_ast_impl(const AST_Node *node, FILE *fp) {
    switch (node->type) {
    case NODE_NUM:
        fprintf(fp, " %ld", node->as.num);
        break;
    case NODE_VAR:
        fprintf(fp, " %.*s", (int)node->as.var.len, node->as.var.name);
        break;
    case NODE_BOOL_LITERAL:
        if (node->as.boolean) {
            fprintf(fp, " (true)");
        } else {
            fprintf(fp, " (false)");
        }
        break;
    case NODE_PLUS:
        fprintf(fp, " (+");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_MINUS:
        fprintf(fp, " (-");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_MULT:
        fprintf(fp, " (*");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_DIV:
        fprintf(fp, " (/");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_EQ:
        fprintf(fp, " (=");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_LEQ:
        fprintf(fp, " (<=");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_NOT:
        fprintf(fp, " (!");
        parser_print_ast_impl(node->as.child.left, fp);
        fprintf(fp, ")");
        break;
    case NODE_AND:
        fprintf(fp, " (&");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_ASSIGN:
        fprintf(fp, " (:=");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, ")");
        break;
    case NODE_SKIP:
        fprintf(fp, " (skip");
        fprintf(fp, ")");
        break;
    case NODE_SEQ:
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        break;
    case NODE_IF:
        fprintf(fp, " (if");
        parser_print_ast_impl(node->as.child.condition, fp);
        fprintf(fp, " (");
        parser_print_ast_impl(node->as.child.left, fp);
        parser_print_ast_impl(node->as.child.right, fp);
        fprintf(fp, "))");
        break;
    case NODE_WHILE:
        fprintf(fp, " (while");
        parser_print_ast_impl(node->as.child.condition, fp);
        fprintf(fp, " (");
        parser_print_ast_impl(node->as.child.left, fp);
        fprintf(fp, "))");
        break;
    default:
        assert(0 && "UNREACHABLE");
    }
}

void parser_print_ast(const AST_Node *node, FILE *fp) {
    parser_print_ast_impl(node, fp);
}

AST_Node *parser_copy_node(const AST_Node *node) {
    if (node != NULL) {

        // Leaf nodes
        if (node->type == NODE_NUM || node->type == NODE_VAR || node->type == NODE_BOOL_LITERAL) {
            AST_Node *node_copy = create_node(node->type);

            if (node->type == NODE_NUM) {
                node_copy->as.num = node->as.num;
            }
            else if (node->type == NODE_VAR) {
                node_copy->as.var.name = node->as.var.name;
                node_copy->as.var.len = node->as.var.len;
            }
            else if (node->type == NODE_BOOL_LITERAL) {
                node_copy->as.boolean = node->as.boolean;
            }

            return node_copy;
        }
        else {
            AST_Node *node_copy = create_node(node->type);

            node_copy->as.child.left = parser_copy_node(node->as.child.left);
            node_copy->as.child.right = parser_copy_node(node->as.child.right);
            node_copy->as.child.condition = parser_copy_node(node->as.child.condition);

            return node_copy;
        }
    }

    return NULL;
}

/* ============================= Recursive descent parser ============================= */
// https://en.wikipedia.org/wiki/Recursive_descent_parser

static AST_Node *parse_stmt(Lexer *lex);
static AST_Node *parse_bexp(Lexer *lex);
static AST_Node *parse_aexp(Lexer *lex);

static void expect(Token t, enum Token_Type type) {
    if (t.type != type) {
        fprintf(stderr, "[ERROR]: Expected type %d but found %d while parsing.", type, t.type);
        exit(1);
    }
}

static AST_Node *parse_factor_aexp(Lexer *lex) {
    Token t = lex_next(lex);

    // OPAR
    if (t.type == TOKEN_OPAR) {
        AST_Node *node = parse_aexp(lex);
        expect(lex_next(lex), TOKEN_CPAR);
        return node;
    }

    // Numeral
    if (t.type == TOKEN_NUM) {
        AST_Node *num_node = create_node(NODE_NUM);
        num_node->as.num = t.as.num;

        return num_node;
    }

    // Variable
    if (t.type == TOKEN_VAR) {
        AST_Node *var_node = create_node(NODE_VAR);
        var_node->as.var.name = t.as.str.name;
        var_node->as.var.len = t.as.str.len;

        return var_node;
    }

    fprintf(stderr, "[ERROR]: Unexpected token of type %d while parsing aexp\n", t.type);
    exit(1);
}

static AST_Node *parse_term_aexp(Lexer *lex) {
    AST_Node *left = parse_factor_aexp(lex);

    // '*' and '/' precedence
    Token t = lex_peek(lex);
    while (t.type == TOKEN_MULT || t.type == TOKEN_DIV) {
        lex_next(lex);

        AST_Node *node = NULL;
        if (t.type == TOKEN_MULT) {
            node = create_node(NODE_MULT);
        } else {
            node = create_node(NODE_DIV);
        }

        node->as.child.left = left;
        node->as.child.right = parse_factor_aexp(lex);
        left = node;

        t = lex_peek(lex);
    }

    return left;
}

static AST_Node *parse_aexp(Lexer *lex) {
    AST_Node *left = parse_term_aexp(lex);

    // '-' and '+' precedence
    Token t = lex_peek(lex);
    while (t.type == TOKEN_PLUS || t.type == TOKEN_MINUS) {
        lex_next(lex);

        AST_Node *node = NULL;
        if (t.type == TOKEN_PLUS) {
            node = create_node(NODE_PLUS);
        } else {
            node = create_node(NODE_MINUS);
        }

        node->as.child.left = left;
        node->as.child.right = parse_term_aexp(lex);
        left = node;

        t = lex_peek(lex);
    }

    return left;
}

static AST_Node *parse_atom_bexp(Lexer *lex) {
    Token t = lex_peek(lex);

    // OPAR
    // if (t.type == TOKEN_OPAR) {
    //     lex_next(lex);

    //     // TODO: Check if parse_bexp is correct
    //     // Otherwise it must be an aexp
    //     //
    //     // For doing this I must return an error code (like NULL) in parse_bexp
    //     AST_Node *node = parse_bexp(lex);
    //     expect(lex_next(lex), TOKEN_CPAR);
    //     return node;
    // }

    // True
    if (t.type == TOKEN_TRUE) {
        lex_next(lex);
        AST_Node *bool_lit_node = create_node(NODE_BOOL_LITERAL);
        bool_lit_node->as.boolean = true;
        return bool_lit_node;
    }

    // False
    if (t.type == TOKEN_FALSE) {
        lex_next(lex);
        AST_Node *bool_lit_node = create_node(NODE_BOOL_LITERAL);
        bool_lit_node->as.boolean = false;
        return bool_lit_node;
    }

    // Not
    if (t.type == TOKEN_NOT) {
        lex_next(lex);
        AST_Node *not_node = create_node(NODE_NOT);
        not_node->as.child.left = parse_bexp(lex);
        return not_node;
    }

    // It can be EQ or LEQ
    AST_Node *left = parse_aexp(lex);
    t = lex_next(lex);

    // EQ
    if (t.type == TOKEN_EQ) {
        AST_Node *eq_node = create_node(NODE_EQ);
        eq_node->as.child.left = left;
        eq_node->as.child.right = parse_aexp(lex);
        return eq_node;
    }

    // LEQ
    if (t.type == TOKEN_LEQ) {
        AST_Node *leq_node = create_node(NODE_LEQ);
        leq_node->as.child.left = left;
        leq_node->as.child.right = parse_aexp(lex);
        return leq_node;
    }

    fprintf(stderr, "[ERROR]: Unexpected token of type %d while parsing bexp\n", t.type);
    exit(1);
}

static AST_Node *parse_bexp(Lexer *lex) {
    AST_Node *left = parse_atom_bexp(lex);

    Token t = lex_peek(lex);
    while (t.type == TOKEN_AND) {
        lex_next(lex);

        AST_Node *and_node = create_node(NODE_AND);
        and_node->as.child.left = left;
        and_node->as.child.right = parse_atom_bexp(lex);
        left = and_node;

        t = lex_peek(lex);
    }

    return left;
}

static AST_Node *parse_atom_stmt(Lexer *lex) {
    Token t = lex_next(lex);

    // Assignment
    if (t.type == TOKEN_VAR) {

        // Variable (a)
        AST_Node *var_node = create_node(NODE_VAR);
        var_node->as.var.name = t.as.str.name;
        var_node->as.var.len = t.as.str.len;

        // Assing symbol (:=)
        t = lex_next(lex);
        expect(t, TOKEN_ASSIGN);

        // Aexp
        AST_Node *assign_node = create_node(NODE_ASSIGN);
        assign_node->as.child.left = var_node;
        assign_node->as.child.right = parse_aexp(lex);

        return assign_node;
    }

    // Skip
    if (t.type == TOKEN_SKIP) {
        AST_Node *skip_node = create_node(NODE_SKIP);
        return skip_node;
    }

    // If stmt
    if (t.type == TOKEN_IF) {
        AST_Node *if_node = create_node(NODE_IF);

        // Condition (b)
        if_node->as.child.condition = parse_bexp(lex);

        // Then symbol
        t = lex_next(lex);
        expect(t, TOKEN_THEN);

        // S1
        if_node->as.child.left = parse_stmt(lex);

        // Else symbol
        t = lex_next(lex);
        expect(t, TOKEN_ELSE);

        // S2
        if_node->as.child.right = parse_stmt(lex);

        // Fi symbol
        t = lex_next(lex);
        expect(t, TOKEN_FI);

        return if_node;
    }

    // While stmt
    if (t.type == TOKEN_WHILE) {
        AST_Node *while_node = create_node(NODE_WHILE);

        // Condition (b)
        while_node->as.child.condition = parse_bexp(lex);

        // Do symbol
        t = lex_next(lex);
        expect(t, TOKEN_DO);

        // S
        while_node->as.child.left = parse_stmt(lex);

        // Done symbol
        t = lex_next(lex);
        expect(t, TOKEN_DONE);

        return while_node;
    }

    fprintf(stderr, "[ERROR]: Unexpected token while parsing stmt\n");
    exit(1);
}

// Parse the sequence statements
static AST_Node *parse_stmt(Lexer *lex) {
    AST_Node *left = parse_atom_stmt(lex);

    Token t = lex_peek(lex);
    if (t.type == TOKEN_SEMICOL) {
        lex_next(lex);
        AST_Node *node = create_node(NODE_SEQ);
        node->as.child.left = left;
        node->as.child.right = parse_stmt(lex);
        return node;
    }

    return left;
}

AST_Node *parser_parse(Lexer *lex) {
    AST_Node *root = parse_stmt(lex);

    // Check if we reached the end
    Token t = lex_peek(lex);
    if (t.type != TOKEN_EOF) {
        fprintf(stderr, "[ERROR]: Expected end of file, but found token of type %d\n", t.type);
        exit(1);
    }

    return root;
}

/* /////////////////////////////////////////////////////////////////////////////////// */
