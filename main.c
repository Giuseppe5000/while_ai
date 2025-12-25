#include "include/abstract_analyzer.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *src_path = "example.while";
    int64_t m = -100;
    int64_t n = 100;
    While_Analyzer *wa = while_analyzer_init_parametric_interval(src_path, m, n);
    while_analyzer_exec(wa);
    while_analyzer_free(wa);
}
