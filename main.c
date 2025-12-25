#include "include/abstract_analyzer.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *src_path = "example.while";
    While_Analyzer *wa = while_analyzer_init_parametric_interval(src_path);
    while_analyzer_free(wa);
}
