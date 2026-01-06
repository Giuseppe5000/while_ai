#ifndef WHILE_AI_CFG_
#define WHILE_AI_CFG_

#include "parser.h"
#include <stdio.h>

enum Edge_Type {
    EDGE_ASSIGN,
    EDGE_GUARD,
    EDGE_SKIP,
};

typedef struct CFG_Node CFG_Node;
typedef struct CFG_Edge CFG_Edge;

struct CFG_Edge {
    size_t src; // Node src id
    size_t dst; // Node dst id
    enum Edge_Type type;
    union {
        AST_Node *skip;
        AST_Node *assign;
        AST_Node *condition;
    } as; // TODO: union is not needed actually
};

struct CFG_Node{
    size_t id;

    // Array of edges that *starts* from this point.
    //
    // The size is fixed because one node can have at maximum 2 edges in output.
    // This is true only for the While Language, because it does not have like
    // switch case and similar.
    //
    // Note: When we are on a while loop/if stmt node then
    //       the first edge is the true condition,
    //       and the second is the false condition.
    CFG_Edge edges[2];
    size_t edge_count;

    // Array of nodes that have an edge coming here
    size_t *preds;
    size_t preds_count;

    bool is_while;
};

typedef struct {
    size_t count;
    CFG_Node *nodes;
} CFG;

// Construct and returns the CFG
CFG *cfg_get(AST_Node *root);

// Prints to 'fp' the Graphviz representation of the CFG
void cfg_print_graphviz(CFG *cfg, FILE *fp);

// Free the CFG
void cfg_free(CFG *cfg);

#endif // WHILE_AI_CFG_
