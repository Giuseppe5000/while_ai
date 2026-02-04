#include "cfg.h"
#include "../common.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static CFG_Node build_node(size_t id) {
    CFG_Node node = {
        .id = id,
        .edge_count = 0,
        .preds = NULL,
        .preds_count = 0,
        .is_while = false,
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
        counter++;
        counter = count_nodes(node->as.child.left, counter);
        counter = count_nodes(node->as.child.right, counter);
        break;
    case NODE_WHILE:
        // If there is a while as first stmt then we add an extra node (skip).
        // Here we just add this skip node to the count.
        if (counter == 1) {
           counter++;
        }
        counter++;
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
        // Grow the dynamic array
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
/* //////////////////////////////////////////////////////////////////////////////////// */


// Links all the elements in the stack of the predecessors to the current node.
static void wire_predecessors(CFG *cfg, size_t cur_node, Pred_Stack *preds) {

    // Copy the predecessors in the current node struct.
    //
    // This function can be called multiple times wiring the same node, so for collecting
    // all the preds we do a realloc every time (the first time will be a simple malloc).
    size_t cur_node_preds_count = cfg->nodes[cur_node].preds_count; // Prev preds count
    cfg->nodes[cur_node].preds_count += preds->count;
    cfg->nodes[cur_node].preds = xrealloc(cfg->nodes[cur_node].preds, sizeof(size_t)*cfg->nodes[cur_node].preds_count);
    memcpy(cfg->nodes[cur_node].preds + cur_node_preds_count, preds->data, sizeof(size_t)*preds->count);

    // Linking the edges
    while (preds->count != 0) {
        size_t pred = pred_stack_pop(preds);
        size_t prev_node_edge_count = cfg->nodes[pred].edge_count;
        if (prev_node_edge_count == 2) {
            assert(0 && "UNREACHABLE");
        }

        cfg->nodes[pred].edges[prev_node_edge_count].dst = cur_node;
        cfg->nodes[pred].edge_count++;
    }
}

// Build the CFG using the AST node.
//
// The contruction follows the struct of the AST, exploring recusively the tree.
//
// For each stmt node it saves the information of the current node (src, type and structures)
// into the edge that will point to the next node (even if we don't know the next, in fact 'dst' will be -1).
//
// Then when we are on the successor node we can link to the predecessor using the 'wire_predecessors' utility.
// For tracing all the predecessors (that can be arbitrary) we use the 'preds' stack.
static void build_cfg_impl(CFG *cfg, AST_Node *node, size_t *counter, Pred_Stack *preds) {
    switch (node->type) {
    case NODE_SKIP:
    case NODE_ASSIGN:
        // Create the assign node
        cfg->nodes[*counter] = build_node(*counter);
        cfg->nodes[*counter].edges[0].src = *counter;
        cfg->nodes[*counter].edges[0].dst = -1;
        enum Edge_Type type = node->type == NODE_ASSIGN ? EDGE_ASSIGN : EDGE_SKIP;
        cfg->nodes[*counter].edges[0].type = type;
        if (node->type == NODE_ASSIGN) {
            cfg->nodes[*counter].edges[0].command = parser_copy_node(node);
        } else {
            cfg->nodes[*counter].edges[0].command = parser_copy_node(node);
        }

        wire_predecessors(cfg, *counter, preds);

        pred_stack_push(preds, *counter);
        *counter += 1;
        break;
    case NODE_SEQ:
        build_cfg_impl(cfg, node->as.child.left, counter, preds);
        build_cfg_impl(cfg, node->as.child.right, counter, preds);
        break;
    case NODE_IF:
        // If node
        cfg->nodes[*counter] = build_node(*counter);

        cfg->nodes[*counter].edges[0].src = *counter;
        cfg->nodes[*counter].edges[0].dst = -1;
        cfg->nodes[*counter].edges[0].type = EDGE_GUARD;
        cfg->nodes[*counter].edges[0].command = parser_copy_node(node->as.child.condition);

        cfg->nodes[*counter].edges[1].src = *counter;
        cfg->nodes[*counter].edges[1].dst = -1;
        cfg->nodes[*counter].edges[1].type = EDGE_GUARD;

        // Negate the condition node for getting the false case
        AST_Node *false_cond = xmalloc(sizeof(AST_Node));
        false_cond->type = NODE_NOT;
        false_cond->as.child.left = parser_copy_node(node->as.child.condition);
        cfg->nodes[*counter].edges[1].command = false_cond;


        wire_predecessors(cfg, *counter, preds);

        const size_t if_cond = *counter;
        pred_stack_push(preds, *counter);
        *counter += 1;

        // Build the two branch, saving the predecessors
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

        // Add the predecessors of the if stmt
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

        // If there is while as first stmt then we add an extra node (skip).
        // This method handle a specific situation, when we have as first instruction
        // a while loop and the init state is different from Top.
        //
        // In that case the state of the while could be refined,
        // but the algorithm just skip and left unchanged the first node as a 'rule'.
        //
        // So as a fix we add an extra node that plays the role of the first node,
        // letting the while become the second node.
        if (*counter == 0) {
            AST_Node *skip = xmalloc(sizeof(AST_Node));
            skip->type = NODE_SKIP;
            build_cfg_impl(cfg, skip, counter, preds);
            free(skip);
        }

        // Loop invariant Node
        cfg->nodes[*counter] = build_node(*counter);
        cfg->nodes[*counter].is_while = true;

        cfg->nodes[*counter].edges[0].src = *counter;
        cfg->nodes[*counter].edges[0].dst = -1;
        cfg->nodes[*counter].edges[0].type = EDGE_GUARD;
        cfg->nodes[*counter].edges[0].command = parser_copy_node(node->as.child.condition);

        cfg->nodes[*counter].edges[1].src = *counter;
        cfg->nodes[*counter].edges[1].dst = -1;
        cfg->nodes[*counter].edges[1].type = EDGE_GUARD;

        // Negate the condition node for getting the exit condition
        AST_Node *exit_cond = xmalloc(sizeof(AST_Node));
        exit_cond->type = NODE_NOT;
        exit_cond->as.child.left = parser_copy_node(node->as.child.condition);
        cfg->nodes[*counter].edges[1].command = exit_cond;

        wire_predecessors(cfg, *counter, preds);

        const size_t loop_inv = *counter;
        pred_stack_push(preds, *counter);
        *counter += 1;

        build_cfg_impl(cfg, node->as.child.left, counter, preds);

        wire_predecessors(cfg, loop_inv, preds);

        // The predecessor of a statement after the while is the loop invariant
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

    cfg->nodes[counter] = build_node(counter);
    wire_predecessors(cfg, counter, &preds);

    free(preds.data);
}

void cfg_print_graphviz(CFG *cfg, FILE *fp) {
    fprintf(fp, "digraph G {\n");
    fprintf(fp, "\tnode [shape=circle]\n\n");
    for (size_t i = 0; i < cfg->count; ++i) {
        CFG_Node node = cfg->nodes[i];
        for (size_t j = 0; j < node.edge_count; ++j) {
            fprintf(fp, "\tP%zu -> P%zu", node.edges[j].src, node.edges[j].dst);
            switch (node.edges[j].type) {
            case EDGE_ASSIGN:
                fprintf(fp, " [label=\"");
                parser_print_ast(node.edges[j].command, fp);
                fprintf(fp, "\"]\n");
                break;
            case EDGE_GUARD:
                fprintf(fp, " [label=\"");
                parser_print_ast(node.edges[j].command, fp);
                fprintf(fp, "\"]\n");
                break;
            case EDGE_SKIP:
                fprintf(fp, " [label=\"(skip)\"]\n");
                break;
            }
        }
    }
    fprintf(fp, "}\n");
}

CFG *cfg_get(AST_Node *root) {
    CFG *cfg = xmalloc(sizeof(CFG));
    cfg->count = count_nodes(root, 1);
    cfg->nodes = xmalloc(sizeof(CFG_Node)*(cfg->count));
    build_cfg(cfg, root);

    return cfg;
}

void cfg_free(CFG *cfg) {
    // Free the AST nodes in the edges
    for (size_t i = 0; i < cfg->count; ++i) {
        CFG_Node node = cfg->nodes[i];
        for (size_t j = 0; j < node.edge_count; ++j) {
            CFG_Edge edge = node.edges[j];
            if (edge.type == EDGE_ASSIGN) {
                parser_free_ast_node(edge.command);
            }
            else if (edge.type == EDGE_GUARD) {
                parser_free_ast_node(edge.command);
            }
            else if (edge.type == EDGE_SKIP) {
                parser_free_ast_node(edge.command);
            }
        }

        // Predecessors array free
        if (node.preds != NULL) {
            free(node.preds);
        }
    }

    free(cfg->nodes);
    free(cfg);
}
