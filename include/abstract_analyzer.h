#ifndef WHILE_AI_ANALYZER_
#define WHILE_AI_ANALYZER_

#include <stdio.h>
#include <stdint.h>

typedef struct While_Analyzer While_Analyzer;

enum Abstract_Dom_Type {
    WHILE_ANALYZER_PARAMETRIC_INTERVAL,
};

typedef struct {
    enum Abstract_Dom_Type type;
    union {
        struct {
            int64_t m;
            int64_t n;
        } parametric_interval;
    } as;
} While_Analyzer_Opt;

typedef struct {
    // Number of steps to wait before applying the widening,
    // if the value is SIZE_MAX then it is disabled.
    size_t widening_delay;

    // Number of descending steps (narrowing)
    size_t descending_steps;

    // Initial abstract state conf file path for the entry point (each domain has its own representation)
    const char *init_state_path;
} While_Analyzer_Exec_Opt;

// Inits the analyzer structure based on the specific domain configuration
While_Analyzer *while_analyzer_init(const char *src_path, const While_Analyzer_Opt *opt);

// Free the analyzer structure
void while_analyzer_free(While_Analyzer *wa);

// Execute the analysis
void while_analyzer_exec(While_Analyzer *wa, const While_Analyzer_Exec_Opt *opt);

// Dump the abstract states of every program point through 'fp'
void while_analyzer_states_dump(const While_Analyzer *wa, FILE *fp);

// Dump the Control Flow Graph through 'fp', uses Graphviz format
void while_analyzer_cfg_dump(const While_Analyzer *wa, FILE *fp);

#endif // WHILE_AI_ANALYZER_
