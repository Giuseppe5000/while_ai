#ifndef WHILE_AI_ABSTRACT_DOMAIN_
#define WHILE_AI_ABSTRACT_DOMAIN_

#include "lang/parser.h"
#include <stdio.h>

typedef void Abstract_State;
typedef void Abstract_Dom_Ctx;

// Operations in the current abstract domain
typedef struct {
    void (*ctx_free) (Abstract_Dom_Ctx *ctx);
    void (*state_free) (Abstract_State *s);
    void (*state_set_bottom) (const Abstract_Dom_Ctx *ctx, Abstract_State *s);
    void (*state_set_top) (const Abstract_Dom_Ctx *ctx, Abstract_State *s);
    void (*state_print) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s, FILE *fp);
    Abstract_State *(*exec_command) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s, const AST_Node *command);
    bool (*state_leq) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
    Abstract_State *(*union_) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
    Abstract_State *(*widening) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
    Abstract_State *(*narrowing) (const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2);
} Abstract_Dom_Ops;


#endif // WHILE_AI_ABSTRACT_DOMAIN_
