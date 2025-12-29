#include "include/abstract_analyzer.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *src_path = "example.while";

    While_Analyzer_Opt opt = {
        .type = WHILE_ANALYZER_PARAMETRIC_INTERVAL,
        .as = {
            .parametric_interval = {
                .m = -100,
                .n = 100,
            },
        },
    };
    While_Analyzer *wa = while_analyzer_init(src_path, &opt);

    While_Analyzer_Exec_Opt exec_opt = {
        .fp = stdout,
        .widening_delay = SIZE_MAX,
        .descending_steps = 0,
    };
    while_analyzer_exec(wa, &exec_opt);

    while_analyzer_free(wa);
}
