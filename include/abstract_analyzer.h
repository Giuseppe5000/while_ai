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
    /* Where the result of the analysis will be put */
    FILE *fp;

    /*
    Number of steps to wait before applying the widening,
    if the value is SIZE_MAX then it is disabled,
    */
    size_t widening_delay;

    /* Number of descending steps (narrowing) */
    size_t descending_steps;
} While_Analyzer_Exec_Opt;

While_Analyzer *while_analyzer_init(const char *src_path, const While_Analyzer_Opt *opt);

void while_analyzer_exec(While_Analyzer *wa, const While_Analyzer_Exec_Opt *opt);

void while_analyzer_free(While_Analyzer *wa);

#endif /* WHILE_AI_ANALYZER_ */
