#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: OOM when allocating %zu bytes.", size);
        exit(1);
    }
    return ptr;
}
