#include "include/abstract_analyzer.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *src_path = "example.while";

    While_Analyzer_Opt opt = {
        .type = WHILE_ANALYZER_PARAMETRIC_INTERVAL,
        .widening_delay = -1,
        .descending_steps = 0,
        .as = {
            .parametric_interval = {
                .m = -100,
                .n = 100,
            },
        },
    };

    While_Analyzer *wa = while_analyzer_init(src_path, &opt);
    while_analyzer_exec(wa);
    while_analyzer_free(wa);
}
