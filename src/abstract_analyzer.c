#include "../include/abstract_analyzer.h"
#include "lang/cfg.h"
#include "lang/parser.h"
#include "common.h"
#include "domain/abstract_interval_domain.h"

#include <stdio.h>
#include <stdlib.h>

/*
TODO: Wrap every point of the CFG with an abstract state.
P0 is initialized with top and other points to bottom.

Then apply the worklist algorithm and return the fixpoint.
*/

typedef void Abstract_State;
typedef void Abstract_Dom_Ctx;

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
    String *vars;
    size_t var_count;

    /* Source code of the input program */
    char *src;

    /* Functions needed for the analysis, dynamically setted to the chosen domain */
    struct {
        void (*ctx_free)(Abstract_Dom_Ctx *ctx);
        void (*state_free)(Abstract_State *s);

        Abstract_State *(*exec_command) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s, const AST_Node *command);
        bool (*abstract_state_leq) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
        Abstract_State *(*U) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2); /* Union */
        Abstract_State *(*widening) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
        Abstract_State *(*narrowing) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
    } func;
};


/* =================== Parametric interval domain Int(m,n) wrappers =================== */
void abstract_interval_state_free_wrap(Abstract_State *s) {
    abstract_interval_state_free((Interval *) s);
}

void abstract_interval_ctx_free_wrap(Abstract_Dom_Ctx *ctx) {
    abstract_interval_ctx_free((Abstract_Interval_Ctx *)ctx);
}

/* TODO: continue */
/* ==================================================================================== */


/* Default init for all types of domain */
static While_Analyzer *while_analyzer_init(const char *src_path) {
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

    /* Get var count in the input program */
    wa->var_count = 0;
    for (size_t i = 0; i < wa->cfg->count; ++i) {
        CFG_Node node = wa->cfg->nodes[i];
        for (size_t j = 0; j < node.edge_count; ++j) {
            CFG_Edge edge = node.edges[j];

            if (edge.type == EDGE_ASSIGN) {
                wa->var_count++;
            }
        }
    }

    /* Alloc and link variable names/len to wa->vars */
    wa->vars = xmalloc(sizeof(String) * wa->var_count);

    size_t loop_count = 0;
    for (size_t i = 0; i < wa->cfg->count; ++i) {
        CFG_Node node = wa->cfg->nodes[i];
        for (size_t j = 0; j < node.edge_count; ++j) {
            CFG_Edge edge = node.edges[j];

            if (edge.type == EDGE_ASSIGN) {
                const char *str = edge.as.assign->as.child.left->as.var.name;
                size_t len = edge.as.assign->as.child.left->as.var.len;
                wa->vars[loop_count].name = str;
                wa->vars[loop_count].len = len;
                loop_count++;
            }
        }
    }

    /* Print vars */
    for (size_t i = 0; i < wa->var_count; ++i) {
        printf("(var) %.*s\n", (int) wa->vars[i].len, wa->vars[i].name);
    }

    /* CFG graphviz */
    cfg_print_graphviz(wa->cfg);

    return wa;
}

While_Analyzer *while_analyzer_init_parametric_interval(const char *src_path, int64_t m, int64_t n) {
    While_Analyzer *wa = while_analyzer_init(src_path);

    /* Domain context setup */
    wa->ctx = abstract_interval_ctx_init(m, n, wa->vars, wa->var_count);

    /* Alloc abstract states for all program points */
    wa->state = malloc(sizeof(Abstract_State *) * wa->cfg->count);

    for (size_t i = 0; i < wa->cfg->count; ++i) {
        wa->state[i] = (Abstract_State*) abstract_interval_state_init(wa->ctx);
    }

    /* TODO: link all domain functions */
    wa->func.state_free = abstract_interval_state_free_wrap;
    wa->func.ctx_free = abstract_interval_ctx_free_wrap;
    // wa->func.abstract_state_leq = abstract_int_state_leq_wrap;

    return wa;
}

void while_analyzer_free(While_Analyzer *wa) {
    /* Free abstract states */
    for (size_t i = 0; i < wa->cfg->count; ++i) {
        wa->func.state_free(wa->state[i]);
    }
    free(wa->state);
    free(wa->ctx);
    free(wa->src);
    cfg_free(wa->cfg);
    free(wa->vars);
    free(wa);
}
