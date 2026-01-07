#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vars_push_unique(Variables *vars, String s) {
    // Check if s is already present in the array
    for (size_t i = 0; i < vars->count; ++i) {
        if (s.len == vars->var[i].len) {
            if (strncmp(vars->var[i].name, s.name, s.len) == 0) {
                return;
            }
        }
    }

    if (vars->count >= vars->capacity) {
        if (vars->capacity == 0) {
            vars->capacity = 32; // Inits with 32 elements
        } else {
            vars->capacity *= 2;
        }
        vars->var = xrealloc(vars->var, vars->capacity*sizeof(String));
    }
    vars->var[vars->count++] = s;
}

void constant_push_unique(Constants *c, int64_t constant) {
    // Check if s is already present in the array
    for (size_t i = 0; i < c->count; ++i) {
        if (c->data[i] == constant) {
            return;
        }
    }

    if (c->count >= c->capacity) {
        if (c->capacity == 0) {
            c->capacity = 32; // Inits with 32 elements
        } else {
            c->capacity *= 2;
        }
        c->data = xrealloc(c->data, c->capacity*sizeof(int64_t));
    }
    c->data[c->count++] = constant;
}

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "[ERROR]: OOM when allocating %zu bytes.\n", size);
        exit(1);
    }
    return ptr;
}

void *xcalloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
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
