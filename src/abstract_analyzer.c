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
    // Control Flow Graph of the input program, contains the program points (the nodes)
    CFG *cfg;

    // Every element is the pointer to a state.
    // So state[0] is the pointer to the state of the first program point
    // (the first node of the cfg, cfg->nodes[0]),
    // state[1] is the pointer to the state of the second program point, and so on...
    Abstract_State **state;

    // Abstract domain context
    Abstract_Dom_Ctx *ctx;

    // Source code of the input program
    char *src;

    // Operations vtable
    const Abstract_Dom_Ops *ops;
};

/* ====================================== Utils ====================================== */

static char *read_file(const char *src_path) {
    // Open source file
    FILE *fp = fopen(src_path, "r");

    if (fp == NULL) {
        fprintf(stderr, "[ERROR]: File %s not found.\n", src_path);
        exit(1);
    }

    // Getting file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Copy the text in a buffer
    char *src = xmalloc((file_size + 1)*sizeof(char));
    fread(src, file_size, 1, fp);
    src[file_size] = '\0';
    fclose(fp);

    return src;
}

/* /////////////////////////////////////////////////////////////////////////////////// */

/* ============================== Variables collection =============================== */

static void vars_collect(While_Analyzer *wa, Variables *vars) {
    Lexer *lex = lex_init(wa->src);

    Token t = lex_next(lex);
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_VAR) {
            vars_push_unique(vars, t.as.str);
        }
        t = lex_next(lex);
    }

    lex_free(lex);
}

/* /////////////////////////////////////////////////////////////////////////////////// */


/* ================================ Constant collection =============================== */

static int int64_compare(const void *a, const void *b) {
    const int64_t *a_int = (const int64_t *) a;
    const int64_t *b_int = (const int64_t *) b;

    if (*b_int < *a_int) {
        return 1;
    } else if (*b_int > *a_int) {
        return -1;
    } else {
        return 0;
    }
}

static void constant_collect(const char *src_path, Constants *constants, size_t vars_count) {

    // Collect constants in the source file
    char *src = read_file(src_path);
    Lexer *lex = lex_init(src);

    Token t = lex_next(lex);
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_NUM) {
            constant_push_unique(constants, t.as.num);
        }
        t = lex_next(lex);
    }

    lex_free(lex);
    free(src);

    // Using constant propagation domain for getting other constants
    While_Analyzer_Opt opt = {
        .type = WHILE_ANALYZER_PARAMETRIC_INTERVAL,
        .as = {
            .parametric_interval = {
                .m = 1,
                .n = -1,
            },
        },
    };

    While_Analyzer_Exec_Opt exec_opt = {
        .widening_delay = SIZE_MAX,
        .descending_steps = 0,
    };

    While_Analyzer *constant_dom = while_analyzer_init(src_path, &opt);
    while_analyzer_exec(constant_dom, &exec_opt);

    for (size_t state = 0; state < constant_dom->cfg->count; ++state) {
        for (size_t j = 0; j < vars_count; ++j) {
            Interval i = ((Interval *)constant_dom->state[state])[j];
            if (i.type != INTERVAL_BOTTOM && i.a != INTERVAL_MIN_INF) {
                constant_push_unique(constants, i.a);
            }
        }
    }

    while_analyzer_free(constant_dom);
}

/* /////////////////////////////////////////////////////////////////////////////////// */

/* ================================== Worklist queue ================================== */

// The worklist is simply a queue (implemented ad linked list)
typedef struct Program_Point_Node Program_Point_Node;
struct Program_Point_Node {
    Program_Point_Node *next;
    Program_Point_Node *prev;
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
        wl->tail = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else {
        node->next = wl->head;
        node->prev = NULL;
        wl->head->prev = node;
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
            // There is only one node
            wl->head = NULL;
            wl->tail = NULL;
        } else {
            // Set the tail to the prev element (there are almost 2 nodes)
            wl->tail = to_free->prev;
            wl->tail->next = NULL;
        }

        free(to_free);
        return id;
    }
}
/* /////////////////////////////////////////////////////////////////////////////////// */


/* ======================== Parametric interval domain Int(m,n) ======================= */
static void while_analyzer_init_parametric_interval(While_Analyzer *wa, const char *src_path, int64_t m, int64_t n) {

    // Collect variables in the source
    Variables vars = {0};
    vars_collect(wa, &vars);

    // Dynamic array of (sorted) constants, by default with -INF and +INF as widening threshold
    Constants c = {0};
    constant_push_unique(&c, INTERVAL_MIN_INF);
    constant_push_unique(&c, INTERVAL_PLUS_INF);

    if (m <= n) {
        constant_collect(src_path, &c, vars.count);
    }

    qsort(c.data, c.count, sizeof(int64_t), int64_compare);

    // Domain context setup
    wa->ctx = abstract_interval_ctx_init(m, n, vars, c);

    // Alloc abstract states for all program points
    wa->state = malloc(sizeof(Abstract_State *) * wa->cfg->count);

    for (size_t i = 0; i < wa->cfg->count; ++i) {
        wa->state[i] = (Abstract_State*) abstract_interval_state_init(wa->ctx);
    }

    // Link all domain functions
    wa->ops = &abstract_interval_ops;
}

/* /////////////////////////////////////////////////////////////////////////////////// */


// Default init for all types of domain
While_Analyzer *while_analyzer_init(const char *src_path, const While_Analyzer_Opt *opt) {

    // Init analyzer
    While_Analyzer *wa = xmalloc(sizeof(While_Analyzer));
    wa->src = read_file(src_path);

    // Lexer
    Lexer *lex = lex_init(wa->src);

    // AST
    AST_Node *ast = parser_parse(lex);
    lex_free(lex);

    // Get CFG
    wa->cfg = cfg_get(ast);
    parser_free_ast_node(ast);

    // Domain specific init
    switch (opt->type) {
    case WHILE_ANALYZER_PARAMETRIC_INTERVAL:
        {
            int64_t m = opt->as.parametric_interval.m;
            int64_t n = opt->as.parametric_interval.n;
            while_analyzer_init_parametric_interval(wa, src_path, m, n);
            break;
        }
    default:
        assert(0 && "UNREACHABLE");
    }

    return wa;
}

void while_analyzer_exec(While_Analyzer *wa, const While_Analyzer_Exec_Opt *opt) {

    // Init the abstract states (Top for P0 and Bottom the others)
    wa->ops->state_set_top(wa->ctx, wa->state[0]);
    for (size_t i = 1; i < wa->cfg->count; ++i) {
        wa->ops->state_set_bottom(wa->ctx, wa->state[i]);
    }

    // Create a counter for each point (needed for opt->widening_delay)
    size_t *step_count = xcalloc(wa->cfg->count, sizeof(size_t));

    // === Worklist algorithm ===
    Worklist wl = {0};
    worklist_init(&wl);

    // Add all the program points to the worklist
    for (size_t i = 0; i < wa->cfg->count; ++i) {
            worklist_enqueue(&wl, i);
    }

    while(wl.tail != NULL) {
        size_t id = worklist_dequeue(&wl);
        CFG_Node node = wa->cfg->nodes[id];
        step_count[id]++;

        if (id != 0) {
            Abstract_State **states = xmalloc(sizeof(Abstract_State *) * node.preds_count);

            // Apply the abstract transfer function for each predecessor
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

            // Union of the results
            Abstract_State *acc = wa->ops->union_(wa->ctx, states[0], states[0]);
            Abstract_State *prev_acc = NULL;
            for (size_t i = 1; i < node.preds_count; ++i) {
                prev_acc = acc;
                acc = wa->ops->union_(wa->ctx, acc, states[i]);
                wa->ops->state_free(prev_acc);
            }

            // Apply widening if we are on a widening point
            if (node.is_while && step_count[id] > opt->widening_delay) {
                Abstract_State *prev_acc = acc;
                acc = wa->ops->widening(wa->ctx, wa->state[id], acc);
                wa->ops->state_free(prev_acc);
            }

            // If state changed signal the node dependencies
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

            // States free
            for (size_t i = 0; i < node.preds_count; ++i) {
                wa->ops->state_free(states[i]);
            }
            free(states);
        }
    }
    free(step_count);
}

void while_analyzer_states_dump(const While_Analyzer *wa, FILE *fp) {
    for (size_t i = 0; i < wa->cfg->count; ++i) {
        fprintf(fp, "[P%zu]\n", i);
        wa->ops->state_print(wa->ctx, wa->state[i], fp);
    }
}

void while_analyzer_cfg_dump(const While_Analyzer *wa, FILE *fp) {
    cfg_print_graphviz(wa->cfg, fp);
}

void while_analyzer_free(While_Analyzer *wa) {

    // Free abstract states
    for (size_t i = 0; i < wa->cfg->count; ++i) {
        wa->ops->state_free(wa->state[i]);
    }
    free(wa->state);
    wa->ops->ctx_free(wa->ctx);
    free(wa->src);
    cfg_free(wa->cfg);
    free(wa);
}
