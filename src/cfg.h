#ifndef WHILE_AI_CFG_
#define WHILE_AI_CFG_

#include "lang/parser.h"

enum Edge_Type {
    EDGE_ASSIGN,
    EDGE_GUARD,
    EDGE_SKIP,
};

typedef struct CFG_Node CFG_Node;
typedef struct CFG_Edge CFG_Edge;

struct CFG_Edge {
    size_t src;
    size_t dst;
    enum Edge_Type type;
    union {
        AST_Node *assign;
        struct {
            AST_Node *condition;
            bool val;
        } guard;
    } as;
};

struct CFG_Node{
    size_t id;
    CFG_Edge *edges;   /* Array of edges that *starts* from this point */
    size_t edge_count;
};

typedef struct {
    size_t points_count;
    CFG_Node *points;
} CFG;

/* Construct the Control Flow Graph using the AST */
CFG *cfg_init(AST_Node *root);

void cfg_free(CFG *cfg);

#endif /* WHILE_AI_CFG_ */
