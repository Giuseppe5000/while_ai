#ifndef WHILE_AI_UTILS_
#define WHILE_AI_UTILS_

#include <stddef.h>

/* Same as originals but exits on OOM */
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

#endif /* WHILE_AI_UTILS_ */
