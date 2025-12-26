#include "abstract_interval_domain.h"
#include "../common.h"
#include <stdlib.h>
#include <string.h>

#define INTERVAL_PLUS_INF INT64_MAX
#define INTERVAL_MIN_INF INT64_MIN

enum Interval_Type {
    INTERVAL_BOTTOM, /* Bottom element */
    INTERVAL_STD,    /* Standard interval (Top included) */
};

/*
Define the integer interval [a,b].
If a or b are INF, then their value is:
    INTERVAL_PLUS_INF for represent infinite.
    INTERVAL_MIN_INF for represent -infinite.
*/
struct Interval {
    enum Interval_Type type;
    int64_t a;
    int64_t b;
};

struct Abstract_Interval_Ctx {
    int64_t m;
    int64_t n;
    const String *vars;
    size_t var_count;
};

/* ================================== Interval ops ==================================== */

/*
Check if interval 'i1' is a less than or equal to interval 'i2'.
Returns true if i1 <= i2 (if i1 is contained in i2), false otherwise.
*/
static bool interval_leq(Interval i1, Interval i2) {
    if (i1.type == INTERVAL_BOTTOM && i2.type == INTERVAL_BOTTOM) {
        return true;
    }
    else if (i1.type == INTERVAL_BOTTOM) {
        return true;
    }
    else if (i2.type == INTERVAL_BOTTOM) {
        return false;
    }
    else {
        return i1.a >= i2.a && i1.b <= i2.b;
    }
}

/*
Create an interval beloging to the domain Int(m,n).

NOTE: if [a, b] does not belong to the domain Top will be returned.
*/
static Interval interval_create(const Abstract_Interval_Ctx *ctx, int64_t a, int64_t b) {
    Interval i = {0};
    i.type = INTERVAL_STD;
    i.a = a;
    i.b = b;

    /* Empty interval (Bottom) */
    if (a > b) {
        i.type = INTERVAL_BOTTOM;
        return i;
    }

    /* Top */
    if (a == INTERVAL_MIN_INF && b == INTERVAL_PLUS_INF) {
        return i;
    }

    /* { [k,k] | k ∈ Z } */
    if (a == b) {
        /* Edge case: (-INF, -INF) or (INF,INF) => return Top */
        if (a == INTERVAL_MIN_INF || a == INTERVAL_PLUS_INF) {
            i.type = INTERVAL_STD;
            i.a = INTERVAL_MIN_INF;
            i.b = INTERVAL_PLUS_INF;
        }
        return i;
    }

    /* { [a,b] | a < b, [a,b] ⊆ [m,n] } */
    if (a < b) {
        Interval i_mn = {
            .type = INTERVAL_STD,
            .a = ctx->m,
            .b = ctx->n,
        };

        if (interval_leq(i, i_mn)) {
            return i;
        }
    }

    /* { (-INF, k] | k ∈ [m, n] } */
    if (a == INTERVAL_MIN_INF && (b >= ctx->m && b <= ctx->n)) {
        return i;
    }

    /* { [k, +INF) | k ∈ [m, n] } */
    if (b == INTERVAL_PLUS_INF && (a >= ctx->m && a <= ctx->n)) {
        return i;
    }

    /* [a,b] is not in the domain, so return Top */
    i.type = INTERVAL_STD;
    i.a = INTERVAL_MIN_INF;
    i.b = INTERVAL_PLUS_INF;
    return i;
}


/* Returns the union of intervals 'a' and 'b' */
static Interval interval_union(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    /* Bottom handling */
    if (i1.type == INTERVAL_BOTTOM) {
        return i2;
    }
    else if (i2.type == INTERVAL_BOTTOM) {
        return i1;
    }
    /* Top handling */
    else if (i1.type == INTERVAL_STD && i1.a == INTERVAL_MIN_INF && i1.b == INTERVAL_PLUS_INF) {
        return i1;
    }
    else if (i2.type == INTERVAL_STD && i2.a == INTERVAL_MIN_INF && i2.b == INTERVAL_PLUS_INF) {
        return i2;
    }
    /* Other cases */
    else {
        int64_t min_a = i1.a >= i2.a ? i2.a : i1.a;
        int64_t max_b = i1.b >= i2.b ? i1.b : i2.b;

        /*
        [Edge case: i1/i2 = [k,k] with k < m or k > n]

        First we check if both i1 and i2 are like [k,k]
        and then we check only for i1/i2.
        */
        if (i1.a == i1.b && i2.a == i2.b) {
            /* Both i1 and i2 are like [k,k], we need to distinguish 3 cases */

            /*
            Case i1,i2 < m.
            Returning (-INF, m] which is the smallest valid interval that contains both i1,i2.
            */
            if (i1.a < ctx->m && i2.a < ctx->m) {
                return interval_create(ctx, INTERVAL_MIN_INF, ctx->m);
            }

            /*
            Case i1,i2 > n.
            Returning [n, +INF) which is the smallest valid interval that contains both i1,i2.
            */
            if (i1.a > ctx->n && i2.a > ctx->n) {
                return interval_create(ctx, ctx->n, INTERVAL_PLUS_INF);
            }

            /* Case i1<m and i2>n or viceversa, so we can only return Top */
            if ((i1.a < ctx->m && i2.a > ctx->n) || (i1.a > ctx->n && i2.a < ctx->m)) {
                return interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF);
            }

            /*
            If I am here, then one of i1/i2 has k in [m,n], so it is not an edge case,
            so we continue with the single cases on i1 and i2.
            This is the motivation of using 'if' and not 'else if'.
            */
        }
        if (i1.a == i1.b) {
            /*
            In this case i1 is like [k,k] and i2 can be anything but not [k,k].
            So for including i1 we must take the min/max and if k<m then the min if -INF
            otherwise max is +INF, so the resulting interval will be a correct over-approximation.
            */
            if (i1.a < ctx->m) {
                min_a = INTERVAL_MIN_INF;
            }
            else if (i1.a > ctx->n) {
                max_b = INTERVAL_PLUS_INF;
            }
        }
        if (i2.a == i2.b) {
            /* Same reasoning but for i2 */
            if (i2.a < ctx->m) {
                min_a = INTERVAL_MIN_INF;
            }
            else if (i2.a > ctx->n) {
                max_b = INTERVAL_PLUS_INF;
            }
        }

        return interval_create(ctx, min_a, max_b);
    }
}

/* Returns the intersection of intervals 'a' and 'b' */
// static Interval interval_intersect(const Abstract_Int_State *s, Interval a, Interval b);

/* Arithmetic operations on intervals */
// static Interval interval_plus(const Abstract_Int_State *s, Interval a, Interval b);
// static Interval interval_minus(const Abstract_Int_State *s, Interval a, Interval b);
// static Interval interval_mult(const Abstract_Int_State *s, Interval a, Interval b);
// static Interval interval_div(const Abstract_Int_State *s, Interval a, Interval b);

/* Widening and Narrowing operators */
// static Interval interval_widening(const Abstract_Int_State *s, Interval a, Interval b);
// static Interval interval_narrowing(const Abstract_Int_State *s, Interval a, Interval b);

/* ==================================================================================== */

Abstract_Interval_Ctx *abstract_interval_ctx_init(int64_t m, int64_t n, const String *vars, size_t var_count) {
    Abstract_Interval_Ctx *ctx = xmalloc(sizeof(Abstract_Interval_Ctx));

    /* Setting the props */
    ctx->m = m;
    ctx->n = n;
    ctx->vars = vars;
    ctx->var_count = var_count;

    return ctx;
}

void abstract_interval_ctx_free(Abstract_Interval_Ctx *ctx) {
    free(ctx);
}

Interval *abstract_interval_state_init(const Abstract_Interval_Ctx *ctx) {
    Interval *s = xmalloc(sizeof(Interval) * ctx->var_count);

    /* By default set to bottom*/
    abstract_interval_state_set_bottom(ctx, s);

    return s;
}

void abstract_interval_state_free(Interval *s) {
    free(s);
}

void abstract_interval_state_set_bottom(const Abstract_Interval_Ctx *ctx, Interval *s) {
    /* Since BOTTOM enum value = 0, all the intervals will be bottom */
    memset(s, 0, sizeof(Interval) * ctx->var_count);
}

void abstract_interval_state_set_top(const Abstract_Interval_Ctx *ctx, Interval *s) {
    /* Set every interval to TOP */
    for (size_t i = 0; i < ctx->var_count; ++i) {
        s[i].type = INTERVAL_STD;
        s[i].a = INTERVAL_MIN_INF;
        s[i].b = INTERVAL_PLUS_INF;
    }
}

Interval *abstract_interval_state_exec_command(const Abstract_Interval_Ctx *ctx, const Interval *s, const AST_Node *command) {}

bool abstract_interval_state_leq(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    bool result = true;

    /* s1 <= s2 if all elements of s1 are <= all elements of s2 */
    for (size_t i = 0; i < ctx->var_count; ++i) {
        result = result && interval_leq(s1[i], s2[i]);
    }

    return result;
}

Interval *abstract_interval_state_union(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    Interval *res = abstract_interval_state_init(ctx);

    for (size_t i = 0; i < ctx->var_count; ++i) {
        res[i] = interval_union(ctx, s1[i], s2[i]);
    }

    return res;
}

Interval *abstract_interval_state_widening(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {}

Interval *abstract_interval_state_narrowing(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {}
