#include "../include/abstract_analyzer.h"
#include "lang/cfg.h"
#include "lang/parser.h"
#include "common.h"
#include "abstract_domain.h"
#include "domain/abstract_interval_domain.h"
#include "domain/wrappers/abstract_interval_domain_wrap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct While_Analyzer {
    /* Control Flow Graph of the input program, contains the program points (the nodes) */
    CFG *cfg;

    /*
    Every element is the pointer to a state.

    So state[0] is the pointer to the state of the first program point
    (the first node of the cfg, cfg->nodes[0]),
    state[1] is the pointer to the state of the second program point, and so on...
    */
    Abstract_State **state;

    /* Abstract domain context */
    Abstract_Dom_Ctx *ctx;

    /* All variables present in the input program */
    Variables vars;

    /* Source code of the input program */
    char *src;

    /* Operations vtable */
    const Abstract_Dom_Ops *ops;
};

/* ================================ Vars dynamic array  =============================== */

static void vars_push(Variables *vars, String s) {

    /* Check if s is already present in the array */
    for (size_t i = 0; i < vars->count; ++i) {
        if (strncmp(vars->var[i].name, s.name, s.len) == 0) {
            return;
        }
    }

    if (vars->count >= vars->capacity) {
        if (vars->capacity == 0) {
            vars->capacity = 32; /* Inits with 32 elements */
        } else {
            vars->capacity *= 2;
        }
        vars->var = xrealloc(vars->var, vars->capacity*sizeof(String));
    }
    vars->var[vars->count++] = s;
}

static void set_vars(While_Analyzer *wa) {
    memset(&(wa->vars), 0, sizeof(Variables));

    for (size_t i = 0; i < wa->cfg->count; ++i) {
        CFG_Node node = wa->cfg->nodes[i];
        for (size_t j = 0; j < node.edge_count; ++j) {
            CFG_Edge edge = node.edges[j];

            if (edge.type == EDGE_ASSIGN) {
                const char *str = edge.as.assign->as.child.left->as.var.name;
                size_t len = edge.as.assign->as.child.left->as.var.len;
                String s = {
                    .name = str,
                    .len = len,
                };
                vars_push(&(wa->vars), s);
            }
        }
    }
}

/* ==================================================================================== */

/* ============================== Worklist dynamic array  ============================= */

/* The worklist is simply a queue (implemented ad linked list) */
typedef struct Program_Point_Node Program_Point_Node;
struct Program_Point_Node {
    Program_Point_Node *next;
    size_t id;
};

typedef struct {
    Program_Point_Node *head;
    Program_Point_Node *tail;
} Worklist;

static void worklist_init(Worklist *wl) {
    wl->head = NULL;
    wl->tail = NULL;
}

static void worklist_enqueue(Worklist *wl, size_t point_id) {
    Program_Point_Node *node = xmalloc(sizeof(*node));
    node->id = point_id;

    if (wl->head == NULL && wl->tail == NULL) {
        wl->head = node;
        node->next = NULL;
        wl->tail = node;
    }
    else {
        node->next = wl->head;
        wl->head = node;
    }
}

static size_t worklist_dequeue(Worklist *wl) {
    if (wl->tail == NULL) {
        fprintf(stderr, "[ERROR]: Trying to dequeue from empty queue.\n");
        exit(1);
    }
    else {
        size_t id = wl->tail->id;

        Program_Point_Node *to_free = wl->tail;

        if (wl->head == wl->tail) {
            /* There is only one node */
            wl->head = NULL;
            wl->tail = NULL;
        } else {
            /* Set the tail to the prev element */
            Program_Point_Node *p = wl->head;
            while (p->next != to_free) {
                p = p->next;
            }
            wl->tail = p;
        }

        free(to_free);
        return id;
    }
}

/* ==================================================================================== */

/* ======================== Parametric interval domain Int(m,n) ======================= */
static void while_analyzer_init_parametric_interval(While_Analyzer *wa, int64_t m, int64_t n) {
    /* Domain context setup */
    wa->ctx = abstract_interval_ctx_init(m, n, &(wa->vars));

    /* Alloc abstract states for all program points */
    wa->state = malloc(sizeof(Abstract_State *) * wa->cfg->count);

    for (size_t i = 0; i < wa->cfg->count; ++i) {
        wa->state[i] = (Abstract_State*) abstract_interval_state_init(wa->ctx);
    }

    /* Link all domain functions */
    wa->ops = &abstract_interval_ops;
}
/* ==================================================================================== */

/* Default init for all types of domain */
While_Analyzer *while_analyzer_init(const char *src_path, const While_Analyzer_Opt *opt) {
    /* Open source file */
    FILE *fp = fopen(src_path, "r");

    if (fp == NULL) {
        fprintf(stderr, "[ERROR]: File %s not found.\n", src_path);
        exit(1);
    }

    /* Getting file size */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* Copy the text in a buffer */
    char *src = xmalloc((file_size + 1)*sizeof(char));
    fread(src, file_size, 1, fp);
    src[file_size] = '\0';
    fclose(fp);

    /* Init analyzer */
    While_Analyzer *wa = xmalloc(sizeof(While_Analyzer));
    wa->src = src;

    /* Lexer */
    Lexer *lex = lex_init(src);

    /* AST */
    AST_Node *ast = parser_parse(lex);
    lex_free(lex);

    /* Get CFG */
    wa->cfg = cfg_get(ast);
    parser_free_ast_node(ast);

    /* Set the vars present in the input program in wa->vars */
    set_vars(wa);

    /* Domain specific init */
    switch (opt->type) {
    case WHILE_ANALYZER_PARAMETRIC_INTERVAL:
        {
            int64_t m = opt->as.parametric_interval.m;
            int64_t n = opt->as.parametric_interval.n;
            while_analyzer_init_parametric_interval(wa, m, n);
            break;
        }
    default:
        assert(0 && "UNREACHABLE");
    }

    return wa;
}

void while_analyzer_exec(While_Analyzer *wa, const While_Analyzer_Exec_Opt *opt) {

    /* Init the abstract states (Top for P0 and Bottom the others) */
    wa->ops->state_set_top(wa->ctx, wa->state[0]);
    for (size_t i = 1; i < wa->cfg->count; ++i) {
        wa->ops->state_set_bottom(wa->ctx, wa->state[i]);
    }

    /* === Worklist algorithm === */
    Worklist wl = {0};
    worklist_init(&wl);

    /* Add all the program points to the worklist */
    for (size_t i = 0; i < wa->cfg->count; ++i) {
            worklist_enqueue(&wl, i);
    }

    while(wl.tail != NULL) {
        size_t id = worklist_dequeue(&wl);
        CFG_Node node = wa->cfg->nodes[id];

        if (id != 0) {
            Abstract_State **states = xmalloc(sizeof(Abstract_State *) * node.preds_count);

            /* Apply the abstract transfer function for each predecessor */
            for (size_t i = 0; i < node.preds_count; ++i) {
                size_t pred = node.preds[i];
                CFG_Edge edge;

                if (wa->cfg->nodes[pred].edges[0].dst == id) {
                    edge = wa->cfg->nodes[pred].edges[0];
                } else {
                    edge = wa->cfg->nodes[pred].edges[1];
                }

                switch (edge.type) {
                case EDGE_ASSIGN:
                    states[i] = wa->ops->exec_command(wa->ctx, wa->state[pred], edge.as.assign);
                    break;
                case EDGE_GUARD:
                    states[i] = wa->ops->exec_command(wa->ctx, wa->state[pred], edge.as.guard.condition);
                    break;
                case EDGE_SKIP:
                    states[i] = wa->ops->exec_command(wa->ctx, wa->state[pred], edge.as.skip);
                    break;
                }
            }

            /* Union of the results */
            Abstract_State *acc = wa->ops->union_(wa->ctx, states[0], states[0]);
            Abstract_State *prev_acc = NULL;
            for (size_t i = 1; i < node.preds_count; ++i) {
                prev_acc = acc;
                acc = wa->ops->union_(wa->ctx, acc, states[i]);
                wa->ops->state_free(prev_acc);
            }

            /* If state changed signal the node dependencies */
            bool state_changed = !(wa->ops->state_leq(wa->ctx, wa->state[id], acc) && wa->ops->state_leq(wa->ctx, acc, wa->state[id]));
            if (state_changed) {
                wa->ops->state_free(wa->state[id]);
                wa->state[id] = acc;

                for (size_t i = 0; i < node.edge_count; ++i) {
                    size_t dep = node.edges[i].dst;
                    worklist_enqueue(&wl, dep);
                }
            } else {
                wa->ops->state_free(acc);
            }

            /* States free */
            for (size_t i = 0; i < node.preds_count; ++i) {
                wa->ops->state_free(states[i]);
            }
            free(states);
        }
    }

    for (size_t i = 0; i < wa->cfg->count; ++i) {
        printf("[P%zu]\n", i);
        wa->ops->state_print(wa->ctx, wa->state[i], opt->fp);
    }
}

void while_analyzer_print_cfg(const While_Analyzer *wa, FILE *fp) {
    cfg_print_graphviz(wa->cfg, fp);
}

void while_analyzer_free(While_Analyzer *wa) {

    /* Free abstract states */
    for (size_t i = 0; i < wa->cfg->count; ++i) {
        wa->ops->state_free(wa->state[i]);
    }
    free(wa->state);
    wa->ops->ctx_free(wa->ctx);
    free(wa->src);
    cfg_free(wa->cfg);
    free(wa->vars.var);
    free(wa);
}

