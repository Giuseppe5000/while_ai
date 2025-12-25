#ifndef WHILE_AI_UTILS_
#define WHILE_AI_UTILS_

#include <stddef.h>

typedef struct {
    const char *name;
    size_t len;
} String;

/* Same as originals but exits on OOM */
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

#endif /* WHILE_AI_UTILS_ */
