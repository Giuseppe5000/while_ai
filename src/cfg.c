#include "cfg.h"
#include <stdbool.h>
#include <stdio.h>

CFG *cfg_init(AST_Node *root) {

    AST_Node *node = root;

    switch (node->type) {
    case NODE_ASSIGN:
        printf("(l) %.*s := X (l)\n", (int)node->as.child.left->as.var.len, node->as.child.left->as.var.str);
        break;
    case NODE_SKIP:
        printf("(l) skip (l)\n");
        break;
    case NODE_SEQ:
        printf("(l)\n");
        parser_print_ast(node->as.child.left);
        printf("(l)\n");
        parser_print_ast(node->as.child.right);
        printf("(l)\n");
        break;
    case NODE_IF:
        printf("(l) if b then (l)\n");
        parser_print_ast(node->as.child.left);
        printf("(l) else (l)\n");
        parser_print_ast(node->as.child.right);
        printf("(l) fi (l)\n");
        break;
    case NODE_WHILE:
        printf("(l) while (l) b do\n");
        printf("(l)\n");
        parser_print_ast(node->as.child.left);
        printf("(l) done (l)\n");
        break;
    default:
        break;
    }

    return NULL;
}
