#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: OOM when allocating %zu bytes.\n", size);
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "[ERROR]: OOM when allocating %zu bytes.\n", size);
        exit(1);
    }
    return p;
}
