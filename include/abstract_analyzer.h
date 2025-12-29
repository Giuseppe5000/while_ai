#ifndef WHILE_AI_ANALYZER_
#define WHILE_AI_ANALYZER_

#include <stdint.h>
#include <stddef.h>

typedef struct While_Analyzer While_Analyzer;

enum Abstract_Dom_Type {
    WHILE_ANALYZER_PARAMETRIC_INTERVAL,
};

typedef struct {
    enum Abstract_Dom_Type type;

    /* Generic opts */
    int widening_delay;      /* Number of steps to wait before applying the widening, disabled if < 0 */
    size_t descending_steps; /* Number of descending steps (narrowing) */

    /* Domain specific opts */
    union {
        struct {
            int64_t m;
            int64_t n;
        } parametric_interval;
    } as;
} While_Analyzer_Opt;

While_Analyzer *while_analyzer_init(const char *src_path, const While_Analyzer_Opt *opt);

void while_analyzer_exec(While_Analyzer *wa);

void while_analyzer_free(While_Analyzer *wa);

#endif /* WHILE_AI_ANALYZER_ */
