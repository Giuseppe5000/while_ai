#include "cfg.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct CFG_Ctx {
    CFG *cfg;
    Parser *parser;
};

static CFG_Node build_node(size_t id) {
    CFG_Node node = {
        .id = id,
        .edge_count = 0,
    };

    return node;
}

static size_t count_nodes(AST_Node *node, size_t counter) {
    switch (node->type) {
    case NODE_ASSIGN:
    case NODE_SKIP:
        counter++;
        break;
    case NODE_SEQ:
        counter = count_nodes(node->as.child.left, counter);
        counter = count_nodes(node->as.child.right, counter);
        break;
    case NODE_IF:
        counter += 2;
        counter = count_nodes(node->as.child.left, counter);
        counter = count_nodes(node->as.child.right, counter);
        break;
    case NODE_WHILE:
        counter += 1;
        counter = count_nodes(node->as.child.left, counter);
        break;
    default:
        break;
    }
    return counter;
}

/* ================================ Predecessor stack ================================= */
typedef struct {
    size_t *data;
    size_t count;
    size_t capacity;
} Pred_Stack;

void pred_stack_push(Pred_Stack *s, size_t pred) {
    if (s->count >= s->capacity) {
        /* Grow the dynamic array */
        if (s->capacity == 0) {
            s->capacity = 8;
        } else {
            s->capacity *= 2;
        }
        s->data = xrealloc(s->data, s->capacity*sizeof(size_t));
    }
    s->data[s->count++] = pred;
}

size_t pred_stack_pop(Pred_Stack *s) {
    if (s->count > 0) {
        s->count--;
        return s->data[s->count];
    }
    fprintf(stderr, "[ERROR]: Trying to pop from empty stack\n");
    exit(1);
}
/* ==================================================================================== */

/*
TODO: The predecessor MUST be a stack (i can use a dynamic arrat), because
in the language a node can have an arbitrary number of predecessors. */
static void build_cfg_impl(CFG *cfg, AST_Node *node, size_t *counter, Pred_Stack *preds) {
    switch (node->type) {
    case NODE_SKIP:
    case NODE_ASSIGN:
        /* Create the assign node */
        cfg->nodes[*counter] = build_node(*counter);
        cfg->nodes[*counter].edges[0].src = *counter;
        cfg->nodes[*counter].edges[0].dst = -1;
        enum Edge_Type type = node->type == NODE_ASSIGN ? EDGE_ASSIGN : EDGE_SKIP;
        cfg->nodes[*counter].edges[0].type = type;
        cfg->nodes[*counter].edges[0].as.assign = node;

        /* If there is a predecessor node we need to wire an edge to this new node */
        while (preds->count != 0) {
            size_t pred = pred_stack_pop(preds);
            size_t prev_node_edge_count = cfg->nodes[pred].edge_count;
            if (prev_node_edge_count == 2) {
                assert(0 && "UNREACHABLE");
            }

            cfg->nodes[pred].edges[prev_node_edge_count].dst = *counter;
            cfg->nodes[pred].edge_count++;
        }

        pred_stack_push(preds, *counter);
        *counter += 1;
        break;
    case NODE_SEQ:
        build_cfg_impl(cfg, node->as.child.left, counter, preds);
        build_cfg_impl(cfg, node->as.child.right, counter, preds);
        break;
    case NODE_IF:
        /* If node */
        cfg->nodes[*counter] = build_node(*counter);

        cfg->nodes[*counter].edges[0].src = *counter;
        cfg->nodes[*counter].edges[0].dst = -1;
        cfg->nodes[*counter].edges[0].type = EDGE_GUARD;
        cfg->nodes[*counter].edges[0].as.guard.condition = node->as.child.condition;
        cfg->nodes[*counter].edges[0].as.guard.val = true;

        cfg->nodes[*counter].edges[1].src = *counter;
        cfg->nodes[*counter].edges[1].dst = -1;
        cfg->nodes[*counter].edges[1].type = EDGE_GUARD;
        cfg->nodes[*counter].edges[1].as.guard.condition = node->as.child.condition;
        cfg->nodes[*counter].edges[1].as.guard.val = false;

        /* If there is a predecessor node we need to wire an edge to this new node */
        while (preds->count != 0) {
            size_t pred = pred_stack_pop(preds);
            size_t prev_node_edge_count = cfg->nodes[pred].edge_count;
            if (prev_node_edge_count == 2) {
                assert(0 && "UNREACHABLE");
            }

            cfg->nodes[pred].edges[prev_node_edge_count].dst = *counter;
            cfg->nodes[pred].edge_count++;
        }

        const size_t if_cond = *counter;
        pred_stack_push(preds, *counter);
        *counter += 1;

        /* Build the two branch, saving the predecessors */

        build_cfg_impl(cfg, node->as.child.left, counter, preds);
        Pred_Stack preds_then_branch = {0};
        while (preds->count != 0) {
            pred_stack_push(&preds_then_branch, pred_stack_pop(preds));
        }
        pred_stack_push(preds, if_cond);
        build_cfg_impl(cfg, node->as.child.right, counter, preds);
        Pred_Stack preds_else_branch = {0};
        while (preds->count != 0) {
            pred_stack_push(&preds_else_branch, pred_stack_pop(preds));
        }

        /* Add the predecessors of the if stmt */
        while (preds_then_branch.count != 0) {
            pred_stack_push(preds, pred_stack_pop(&preds_then_branch));
        }

        while (preds_else_branch.count != 0) {
            pred_stack_push(preds, pred_stack_pop(&preds_else_branch));
        }

        free(preds_then_branch.data);
        free(preds_else_branch.data);
        break;
    case NODE_WHILE:
        /* Loop invariant Node */
        cfg->nodes[*counter] = build_node(*counter);

        cfg->nodes[*counter].edges[0].src = *counter;
        cfg->nodes[*counter].edges[0].dst = -1;
        cfg->nodes[*counter].edges[0].type = EDGE_GUARD;
        cfg->nodes[*counter].edges[0].as.guard.condition = node->as.child.condition;
        cfg->nodes[*counter].edges[0].as.guard.val = true;

        cfg->nodes[*counter].edges[1].src = *counter;
        cfg->nodes[*counter].edges[1].dst = -1;
        cfg->nodes[*counter].edges[1].type = EDGE_GUARD;
        cfg->nodes[*counter].edges[1].as.guard.condition = node->as.child.condition;
        cfg->nodes[*counter].edges[1].as.guard.val = false;

        /* If there is a predecessor node we need to wire an edge to this new node */
        while (preds->count != 0) {
            size_t pred = pred_stack_pop(preds);
            size_t prev_node_edge_count = cfg->nodes[pred].edge_count;
            if (prev_node_edge_count == 2) {
                assert(0 && "UNREACHABLE");
            }

            cfg->nodes[pred].edges[prev_node_edge_count].dst = *counter;
            cfg->nodes[pred].edge_count++;
        }

        const size_t loop_inv = *counter;
        pred_stack_push(preds, *counter);
        *counter += 1;

        build_cfg_impl(cfg, node->as.child.left, counter, preds);

        /* Wire the last added node (the predecessor) to the loop invariant node */
        while (preds->count != 0) {
            size_t pred = pred_stack_pop(preds);
            size_t prev_node_edge_count = cfg->nodes[pred].edge_count;
            if (prev_node_edge_count == 2) {
                assert(0 && "UNREACHABLE");
            }

            cfg->nodes[pred].edges[prev_node_edge_count].dst = loop_inv;
            cfg->nodes[pred].edge_count++;
        }

        /* The predecessor of a statement after the while is the loop invariant */
        pred_stack_push(preds, loop_inv);
        break;
    default:
        break;
    }
}

static void build_cfg(CFG *cfg, AST_Node *root) {
    size_t counter = 0;
    Pred_Stack preds = {0};
    build_cfg_impl(cfg, root, &counter, &preds);

    /* Wire to the last node */
    while (preds.count != 0) {
        size_t pred = pred_stack_pop(&preds);
        size_t prev_node_edge_count = cfg->nodes[pred].edge_count;
        if (prev_node_edge_count == 2) {
            assert(0 && "UNREACHABLE");
        }

        cfg->nodes[pred].edges[prev_node_edge_count].dst = counter;
        cfg->nodes[pred].edge_count++;
    }

    free(preds.data);
}

void cfg_print_graphviz(CFG *cfg) {
    printf("digraph G {\n");
    printf("\tnode [shape=circle]\n\n");
    for (size_t i = 0; i < cfg->count; ++i) {
        CFG_Node node = cfg->nodes[i];
        for (size_t j = 0; j < node.edge_count; ++j) {
            printf("\tP%zu -> P%zu", node.edges[j].src, node.edges[j].dst);
            switch (node.edges[j].type) {
            case EDGE_ASSIGN:
                printf(" [label=\"");

                /*
                Simply get the pointer to the var name
                and continue printing until the end cond.
                This mehod uses the assumption that the vars are pointing
                to the original source file and that the source is zero ended.
                */
                const char *var = node.edges[j].as.assign->as.child.left->as.var.str;
                while (
                *var != ';' && *var != '\0' &&
                strncmp(var, "else", 4) != 0 &&
                strncmp(var, "fi", 2) != 0 &&
                strncmp(var, "done", 4) != 0
                ) {
                    if (*var != '\n') {
                        printf("%c", *var);
                    }
                    var++;
                }

                printf("\"]\n");
                break;
            case EDGE_GUARD:
                printf(" [label=\"%s\"]\n", node.edges[j].as.guard.val ? "T" : "F");
                break;
            case EDGE_SKIP:
                printf(" [label=\"skip\"]\n");
                break;
            }
        }
    }
    printf("}\n");
}

CFG_Ctx *cfg_init(const char *src) {
    CFG_Ctx *ctx = xmalloc(sizeof(CFG_Ctx));

    /* Parser init */
    ctx->parser = parser_init(src);

    /* CFG init */
    ctx->cfg = NULL;

    return ctx;
}

CFG *cfg_get(CFG_Ctx *ctx) {
    if (ctx->cfg == NULL) {
        /* Alloc and build */
        AST_Node *root = parser_parse(ctx->parser);
        ctx->cfg = xmalloc(sizeof(CFG));
        ctx->cfg->count = count_nodes(root, 1);
        ctx->cfg->nodes = xmalloc(sizeof(CFG_Node)*(ctx->cfg->count));
        build_cfg(ctx->cfg, root);
        cfg_print_graphviz(ctx->cfg);
    }

    return ctx->cfg;
}

void cfg_free(CFG_Ctx *ctx) {
    parser_free(ctx->parser);
    if (ctx->cfg != NULL) {
        free(ctx->cfg->nodes);
        free(ctx->cfg);
    }
    free(ctx);
}
