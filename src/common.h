#ifndef WHILE_AI_UTILS_
#define WHILE_AI_UTILS_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *name;
    size_t len;
} String;

typedef struct {
    String *var;
    size_t count;
    size_t capacity;
} Variables;

typedef struct {
    int64_t *data;
    size_t count;
    size_t capacity;
} Constants;

/* Push 's' into the array if not already inside */
void vars_push_unique(Variables *vars, String s);

/* Push 'constat' into the array if not already inside */
void constant_push_unique(Constants *c, int64_t constant);

/* Same as originals but exits on OOM */
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);

#endif /* WHILE_AI_UTILS_ */
