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

    printf("\n");
    while_analyzer_cfg_dump(wa, stdout);
    printf("\n");

    While_Analyzer_Exec_Opt exec_opt = {
        .widening_delay = SIZE_MAX,
        .descending_steps = 0,
    };
    while_analyzer_exec(wa, &exec_opt);
    while_analyzer_states_dump(wa, stdout);
    while_analyzer_free(wa);

    // printf("[Constant propagation domain]\n");

    /* Constant propagation */
    opt = (While_Analyzer_Opt) {
        .type = WHILE_ANALYZER_PARAMETRIC_INTERVAL,
        .as = {
            .parametric_interval = {
                .m = 1,
                .n = -1,
            },
        },
    };

    wa = while_analyzer_init(src_path, &opt);
    while_analyzer_exec(wa, &exec_opt);
    // while_analyzer_states_dump(wa, stdout);
    while_analyzer_free(wa);
}
