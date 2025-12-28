#include "abstract_interval_domain.h"
#include "../common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
    const Variables *vars;
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

NOTE: if [a, b] does not belong to the domain a correct over-approximation will be returned.
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

    /* === [a,b] is not in the domain, looking for a correct over-approximation === */

    /*
    [a,b] < m => (-INF, m]

    Checking if m <= n, because if m > n then the constant
    propagation domain does not have intervals like (-INF, m].
    */
    if (b < ctx->m && ctx->m <= ctx->n) {
        i.a = INTERVAL_MIN_INF;
        i.b = ctx->m;
        return i;
    }

    /* [a,b] > n so [n, +INF) (Same check for constant prop. here) */
    else if (a > ctx->n && ctx->m <= ctx->n) {
        i.a = ctx->n;
        i.b = INTERVAL_PLUS_INF;
        return i;
    }

    /* [a,b] with 'm <= b <= n' and a < m => (-INF, b] */
    if (a < ctx->m && (b >= ctx->m && b <= ctx->n)) {
        i.a = INTERVAL_MIN_INF;
        return i;
    }

    /* [a,b] with 'm <= a <= n' and b > n => [a, +INF) */
    if (b > ctx->n && (a >= ctx->m && a <= ctx->n)) {
        i.b = INTERVAL_PLUS_INF;
        return i;
    }

    /* Otherwise return Top */
    i.a = INTERVAL_MIN_INF;
    i.b = INTERVAL_PLUS_INF;
    return i;
}


/* Returns the union of intervals 'a' and 'b' */
static Interval interval_union(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    /* Bottom handling */
    if (i1.type == INTERVAL_BOTTOM) return i2;
    if (i2.type == INTERVAL_BOTTOM) return i1;

    int64_t min_a = i1.a >= i2.a ? i2.a : i1.a;
    int64_t max_b = i1.b >= i2.b ? i1.b : i2.b;

    /* If [min_a, max_b] is not in the domain, the function will choose a correct over-approximation */
    return interval_create(ctx, min_a, max_b);
}

/* Returns the intersection of intervals 'a' and 'b' */
// static Interval interval_intersect(const Abstract_Int_State *s, Interval a, Interval b);

/* Arithmetic operations on intervals */

/* Returns true if a+b will produce an integer overflow */
static bool integer_plus_overflow_check(int64_t a, int64_t b) {
    if (b > 0) {
        /* If a + b > INT64_MAX there is an overflow */
        return a > (INT64_MAX - b);
    } else if (b < 0) {
        /* If a + b < INT64_MIN there is an overflow */
        return a < (INT64_MIN - b);
    }
    return false;
}

static Interval interval_plus(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    /* Bottom handling */
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    int64_t a;
    int64_t b;

    /* Rule: [i1.a,i1.b] - [i2.a,i2.b] = [i1.a + i2.a, i1.b + i2.b] */

    /* Compute a (checking for overflows) */
    if (i1.a == INTERVAL_MIN_INF || i2.a == INTERVAL_MIN_INF) {
        a = INTERVAL_MIN_INF;
    }
    else {
        if (integer_plus_overflow_check(i1.a, i2.a)) {
            assert(0 && "Overflow detected in interval_plus function!"); /* TODO: What to do with an overflow? */
        } else {
            a = i1.a + i2.a;
        }
    }

    /* Compute b (checking for overflows) */
    if (i1.b == INTERVAL_PLUS_INF || i2.b == INTERVAL_PLUS_INF) {
        b = INTERVAL_PLUS_INF;
    }
    else {
        if (integer_plus_overflow_check(i1.b, i2.b)) {
            assert(0 && "Overflow detected in interval_plus function!"); /* TODO: What to do with an overflow? */
        } else {
            b = i1.b + i2.b;
        }
    }

    return interval_create(ctx, a, b);
}

static Interval interval_minus(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    /* Bottom handling */
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    int64_t a;
    int64_t b;

    /* Rule: [i1.a,i1.b] - [i2.a,i2.b] = [i1.a - i2.b, i1.b - i2.a] */

    /* Compute a (checking for overflows) */
    if (i1.a == INTERVAL_MIN_INF || i2.b == INTERVAL_PLUS_INF) {
        a = INTERVAL_MIN_INF;
    }
    else {
        if (integer_plus_overflow_check(i1.a, -i2.b)) {
            assert(0 && "Overflow detected in interval_plus function!"); /* TODO: What to do with an overflow? */
        } else {
            a = i1.a - i2.b;
        }
    }

    /* Compute b (checking for overflows) */
    if (i1.b == INTERVAL_PLUS_INF || i2.a == INTERVAL_MIN_INF) {
        b = INTERVAL_PLUS_INF;
    }
    else {
        if (integer_plus_overflow_check(i1.b, -i2.a)) {
            assert(0 && "Overflow detected in interval_plus function!"); /* TODO: What to do with an overflow? */
        } else {
            b = i1.b - i2.a;
        }
    }

    return interval_create(ctx, a, b);
}

// static Interval interval_mult(const Abstract_Int_State *s, Interval a, Interval b);
// static Interval interval_div(const Abstract_Int_State *s, Interval a, Interval b);

/* Widening and Narrowing operators */
// static Interval interval_widening(const Abstract_Int_State *s, Interval a, Interval b);
// static Interval interval_narrowing(const Abstract_Int_State *s, Interval a, Interval b);

/* ==================================================================================== */

Abstract_Interval_Ctx *abstract_interval_ctx_init(int64_t m, int64_t n, const Variables *vars) {
    Abstract_Interval_Ctx *ctx = xmalloc(sizeof(Abstract_Interval_Ctx));

    /* Setting the props */
    ctx->m = m;
    ctx->n = n;
    ctx->vars = vars;

    return ctx;
}

void abstract_interval_ctx_free(Abstract_Interval_Ctx *ctx) {
    free(ctx);
}

Interval *abstract_interval_state_init(const Abstract_Interval_Ctx *ctx) {
    Interval *s = xmalloc(sizeof(Interval) * ctx->vars->count);

    /* By default set to bottom*/
    abstract_interval_state_set_bottom(ctx, s);

    return s;
}

void abstract_interval_state_free(Interval *s) {
    free(s);
}

void abstract_interval_state_set_bottom(const Abstract_Interval_Ctx *ctx, Interval *s) {
    /* Since BOTTOM enum value = 0, all the intervals will be bottom */
    memset(s, 0, sizeof(Interval) * ctx->vars->count);
}

void abstract_interval_state_set_top(const Abstract_Interval_Ctx *ctx, Interval *s) {
    /* Set every interval to TOP */
    for (size_t i = 0; i < ctx->vars->count; ++i) {
        s[i].type = INTERVAL_STD;
        s[i].a = INTERVAL_MIN_INF;
        s[i].b = INTERVAL_PLUS_INF;
    }
}

bool abstract_interval_state_leq(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    bool result = true;

    /* s1 <= s2 if all elements of s1 are <= all elements of s2 */
    for (size_t i = 0; i < ctx->vars->count; ++i) {
        result = result && interval_leq(s1[i], s2[i]);
    }

    return result;
}

Interval *abstract_interval_state_union(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    Interval *res = abstract_interval_state_init(ctx);

    for (size_t i = 0; i < ctx->vars->count; ++i) {
        res[i] = interval_union(ctx, s1[i], s2[i]);
    }

    return res;
}

Interval *abstract_interval_state_widening(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {}

Interval *abstract_interval_state_narrowing(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {}

/* ================================ Commands execution ================================ */

/* Returns a new heap allocated state with the same elements of 's' */
static Interval *clone_state(const Abstract_Interval_Ctx *ctx, const Interval *s) {
    Interval *res = abstract_interval_state_init(ctx);
    memcpy(res, s, sizeof(Interval) * ctx->vars->count);
    return res;
}

static Interval exec_aexpr(const Abstract_Interval_Ctx *ctx, const Interval *s, const AST_Node *node) {
    switch (node->type) {
    case NODE_NUM:
        {
            int64_t num = node->as.num;
            return interval_create(ctx, num, num);
        }

    case NODE_VAR:
        {
            /* Get the assigned variable */
            String var = node->as.var;

            /* Get the interval for that variable */
            size_t var_index = 0;
            for (size_t i = 0; i < ctx->vars->count; ++i) {
                if (strncmp(var.name, ctx->vars->var[i].name, var.len) == 0) {
                    var_index = i;
                }
            }
            return s[var_index];
        }

    case NODE_PLUS:
        {
            Interval i1 = exec_aexpr(ctx, s, node->as.child.left);
            Interval i2 = exec_aexpr(ctx, s, node->as.child.right);
            return interval_plus(ctx, i1, i2);
        }
    case NODE_MINUS:
        {
            Interval i1 = exec_aexpr(ctx, s, node->as.child.left);
            Interval i2 = exec_aexpr(ctx, s, node->as.child.right);
            return interval_minus(ctx, i1, i2);
        }
    case NODE_MULT:
    case NODE_DIV:
        assert(0 && "TODO");
    default:
        assert(0 && "UNREACHABLE");
    }
}

static Interval *abstract_interval_state_exec_assign(const Abstract_Interval_Ctx *ctx, const Interval *s, const AST_Node *assign) {

    /* Get the assigned variable */
    String var = assign->as.child.left->as.var;

    /* Get the interval for that variable */
    size_t var_index = 0;
    for (size_t i = 0; i < ctx->vars->count; ++i) {
        if (strncmp(var.name, ctx->vars->var[i].name, var.len) == 0) {
            var_index = i;
        }
    }

    /* Compute the right expression of assign node */
    Interval aexpr_res = exec_aexpr(ctx, s, assign->as.child.right);

    /* Create the new state and return */
    Interval *res = clone_state(ctx, s);
    res[var_index] = aexpr_res;
    return res;
}

Interval *abstract_interval_state_exec_command(const Abstract_Interval_Ctx *ctx, const Interval *s, const AST_Node *command) {

    Interval *res = NULL;

    switch (command->type) {
    case NODE_ASSIGN:
        res = abstract_interval_state_exec_assign(ctx, s, command);
        break;
    case NODE_IF:
    case NODE_WHILE:
        /* TODO */
        break;
    case NODE_SKIP:
        res = clone_state(ctx, s);
        break;
    default:
        assert(0 && "UNREACHABLE");
    }

    return res;
}

/* ==================================================================================== */
