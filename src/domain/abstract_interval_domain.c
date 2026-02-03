#include "abstract_interval_domain.h"
#include "../common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

struct Abstract_Interval_Ctx {
    int64_t m;
    int64_t n;
    Variables vars;
    // Threshold points for widening, this array is sorted and contains always -INF and +INF
    Constants widening_points;
};

/* ================================== Interval ops ==================================== */

// Check if interval 'i1' is a less than or equal to interval 'i2'.
// Returns true if i1 <= i2 (if i1 is contained in i2), false otherwise.
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

// Create an interval beloging to the domain Int(m,n).
//
// NOTE: if [a, b] does not belong to the domain a correct over-approximation will be returned.
static Interval interval_create(const Abstract_Interval_Ctx *ctx, int64_t a, int64_t b) {
    Interval i = {0};
    i.type = INTERVAL_STD;
    i.a = a;
    i.b = b;

    // Empty interval (Bottom)
    if (a > b) {
        i.type = INTERVAL_BOTTOM;
        return i;
    }

    // Top
    if (a == INTERVAL_MIN_INF && b == INTERVAL_PLUS_INF) {
        return i;
    }

    // { [k,k] | k ∈ Z }
    if (a == b) {
        // Edge case: (-INF, -INF) or (INF,INF) => return Top
        if (a == INTERVAL_MIN_INF || a == INTERVAL_PLUS_INF) {
            i.a = INTERVAL_MIN_INF;
            i.b = INTERVAL_PLUS_INF;
        }
        return i;
    }

    // { [a,b] | a < b, [a,b] ⊆ [m,n] }
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

    // { (-INF, k] | k ∈ [m, n] }
    if (a == INTERVAL_MIN_INF && (b >= ctx->m && b <= ctx->n)) {
        return i;
    }

    // { [k, +INF) | k ∈ [m, n] }
    if (b == INTERVAL_PLUS_INF && (a >= ctx->m && a <= ctx->n)) {
        return i;
    }

    // === [a,b] is not in the domain, looking for a correct over-approximation ===

    // [a,b] < m => (-INF, m]
    //
    // Checking if m <= n, because if m > n then the constant
    // propagation domain does not have intervals like (-INF, m].
    if (b < ctx->m && ctx->m <= ctx->n) {
        i.a = INTERVAL_MIN_INF;
        i.b = ctx->m;
        return i;
    }

    // [a,b] > n so [n, +INF) (Same check for constant prop. here)
    else if (a > ctx->n && ctx->m <= ctx->n) {
        i.a = ctx->n;
        i.b = INTERVAL_PLUS_INF;
        return i;
    }

    // [a,b] with 'm <= b <= n' and a < m => (-INF, b]
    if (a < ctx->m && (b >= ctx->m && b <= ctx->n)) {
        i.a = INTERVAL_MIN_INF;
        return i;
    }

    // [a,b] with 'm <= a <= n' and b > n => [a, +INF)
    if (b > ctx->n && (a >= ctx->m && a <= ctx->n)) {
        i.b = INTERVAL_PLUS_INF;
        return i;
    }

    // Otherwise return Top
    i.a = INTERVAL_MIN_INF;
    i.b = INTERVAL_PLUS_INF;
    return i;
}


// Returns the union of intervals 'a' and 'b'
static Interval interval_union(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i2;
    if (i2.type == INTERVAL_BOTTOM) return i1;

    int64_t min_a = i1.a >= i2.a ? i2.a : i1.a;
    int64_t max_b = i1.b >= i2.b ? i1.b : i2.b;

    // If [min_a, max_b] is not in the domain, the function will choose a correct over-approximation
    return interval_create(ctx, min_a, max_b);
}

// Returns the intersection of intervals 'a' and 'b'
static Interval interval_intersect(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    int64_t max_a = i1.a >= i2.a ? i1.a : i2.a;
    int64_t min_b = i1.b >= i2.b ? i2.b : i1.b;

    return interval_create(ctx, max_a, min_b);
}

// Addition checking overflow and INF
static int64_t safe_plus(int64_t a, int64_t b) {

    // Here there aren't cases like a = +INF and b = -INF because it can't happen by contruction
    if (a == INTERVAL_PLUS_INF || b == INTERVAL_PLUS_INF) return INTERVAL_PLUS_INF;
    if (a == INTERVAL_MIN_INF || b == INTERVAL_MIN_INF) return INTERVAL_MIN_INF;

    // Check overflow: a + b > +INF
    if (b > 0 && a > INTERVAL_PLUS_INF - b) return INTERVAL_PLUS_INF;

    // Check underflow: a + b < -INF
    if (b < 0 && a < INTERVAL_MIN_INF - b) return INTERVAL_MIN_INF;

    return a + b;
}

// Subtraction checking overflow and INF
static int64_t safe_minus(int64_t a, int64_t b) {

    // Here there aren't cases like a = +INF and b = +INF because it can't happen by contruction
    if (a == INTERVAL_PLUS_INF || b == INTERVAL_MIN_INF) return INTERVAL_PLUS_INF;
    if (a == INTERVAL_MIN_INF || b == INTERVAL_PLUS_INF) return INTERVAL_MIN_INF;

    // Check overflow: a - b > +INF
    if (b < 0 && a > INTERVAL_PLUS_INF + b) return INTERVAL_PLUS_INF;

    // Check underflow: a - b < -INF
    if (b > 0 && a < INTERVAL_MIN_INF + b) return INTERVAL_MIN_INF;

    return a - b;
}

// Multiplication checking overflow and INF
static int64_t safe_mult(int64_t a, int64_t b) {

    // Zero handling
    if (a == 0 || b == 0) return 0;

    // INF handling, all possible cases because a and b can be anything
    if (a == INTERVAL_MIN_INF && b > 0) return INTERVAL_MIN_INF;
    if (a == INTERVAL_MIN_INF && b < 0) return INTERVAL_PLUS_INF;
    if (a == INTERVAL_PLUS_INF && b > 0) return INTERVAL_PLUS_INF;
    if (a == INTERVAL_PLUS_INF && b < 0) return INTERVAL_MIN_INF;

    if (b == INTERVAL_MIN_INF && a > 0) return INTERVAL_MIN_INF;
    if (b == INTERVAL_MIN_INF && a < 0) return INTERVAL_PLUS_INF;
    if (b == INTERVAL_PLUS_INF && a > 0) return INTERVAL_PLUS_INF;
    if (b == INTERVAL_PLUS_INF && a < 0) return INTERVAL_MIN_INF;

    // Check overflow: a * b > +INF <=> a < +INF / b -- (a and b positive)
    if (a > 0 && b > 0 && a > INTERVAL_PLUS_INF / b) return INTERVAL_PLUS_INF;

    // Check overflow: a * b < -INF <=> b < -INF / a -- (a positive, b negative)
    if (a > 0 && b < 0 && b < INTERVAL_MIN_INF / a) return INTERVAL_MIN_INF;

    // Check overflow: a * b < -INF <=> a < -INF / b -- (a negative, b positive)
    if (a < 0 && b > 0 && a < INTERVAL_MIN_INF / b) return INTERVAL_MIN_INF;

    // Check overflow: a * b > +INF <=> -a > -(+INF / b) <=> a < +INF / b -- (a negative, b negative)
    if (a < 0 && b < 0 && a < INTERVAL_PLUS_INF / b) return INTERVAL_PLUS_INF;

    return a * b;
}

// Division checking overflow and INF, b != 0
static int64_t safe_div(int64_t a, int64_t b) {

    // INF handling, all possible cases because a and b can be anything
    if (a == INTERVAL_MIN_INF && b > 0) return INTERVAL_MIN_INF;
    if (a == INTERVAL_MIN_INF && b < 0) return INTERVAL_PLUS_INF;
    if (a == INTERVAL_PLUS_INF && b > 0) return INTERVAL_PLUS_INF;
    if (a == INTERVAL_PLUS_INF && b < 0) return INTERVAL_MIN_INF;

    if (b == INTERVAL_MIN_INF && a > 0) return INTERVAL_MIN_INF;
    if (b == INTERVAL_MIN_INF && a < 0) return INTERVAL_PLUS_INF;
    if (b == INTERVAL_PLUS_INF && a > 0) return INTERVAL_PLUS_INF;
    if (b == INTERVAL_PLUS_INF && a < 0) return INTERVAL_MIN_INF;

    // With integer division overflow can happen with INTERVAL_MIN_INF / -1.
    // This case is handled in the above if stmt.

    return a / b;
}

static int64_t min4(int64_t a, int64_t b, int64_t c, int64_t d) {
    int64_t min = a < b ? a : b;
    min = min < c ? min : c;
    min = min < d ? min : d;
    return min;
}

static int64_t max4(int64_t a, int64_t b, int64_t c, int64_t d) {
    int64_t max = a > b ? a : b;
    max = max > c ? max : c;
    max = max > d ? max : d;
    return max;
}

static Interval interval_plus(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    // Rule: [i1.a,i1.b] +# [i2.a,i2.b] = [i1.a + i2.a, i1.b + i2.b]
    int64_t a = safe_plus(i1.a, i2.a);
    int64_t b = safe_plus(i1.b, i2.b);

    return interval_create(ctx, a, b);
}

static Interval interval_minus(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    // Rule: [i1.a,i1.b] -# [i2.a,i2.b] = [i1.a - i2.b, i1.b - i2.a]
    int64_t a = safe_minus(i1.a, i2.b);
    int64_t b = safe_minus(i1.b, i2.a);

    return interval_create(ctx, a, b);
}

static Interval interval_mult(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    // Rule: [i1.a,i1.b] *# [i2.a,i2.b] = [x,y]
    // where: x = min(i1.a*i2.a, i1.a*i2.b, i1.b*i2.a, i1.b*i2.b)
    //        y = max(i1.a*i2.a, i1.a*i2.b, i1.b*i2.a, i1.b*i2.b)
    //
    // Setting i1.a = a, i1.b = b, i2.a = c, i2.b = d:
    //        x = min(ac, ad, bc, bd)
    //        y = max(ac, ad, bc, bd)
    int64_t ac = safe_mult(i1.a, i2.a);
    int64_t ad = safe_mult(i1.a, i2.b);
    int64_t bc = safe_mult(i1.b, i2.a);
    int64_t bd = safe_mult(i1.b, i2.b);

    int64_t a = min4(ac, ad, bc, bd);
    int64_t b = max4(ac, ad, bc, bd);

    return interval_create(ctx, a, b);
}

static Interval interval_div(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i1;
    if (i2.type == INTERVAL_BOTTOM) return i2;

    // Setting i1.a = a, i1.b = b, i2.a = c, i2.b = d.
    //
    // Rule: [a,b] /# [c,d] =
    //       [min(a/c,a/d), max(b/c,b/d)] -- if c >= 1
    //       [min(b/c,b/d), max(a/c,a/d)] -- if d <= -1
    //       ([a,b] /# ([c,d] Intersect [1,+INF))) Union ([a,b] /# ([c,d] Intersect (-INF,1])) -- otherwise
    int64_t a;
    int64_t b;

    if (i2.a >= 1) {
        int64_t ac = safe_div(i1.a, i2.a);
        int64_t ad = safe_div(i1.a, i2.b);
        int64_t bc = safe_div(i1.b, i2.a);
        int64_t bd = safe_div(i1.b, i2.b);

        a = ac >= ad ? ad : ac;
        b = bc >= bd ? bc : bd;
        return interval_create(ctx, a, b);
    }
    else if (i2.b <= -1) {
        int64_t ac = safe_div(i1.a, i2.a);
        int64_t ad = safe_div(i1.a, i2.b);
        int64_t bc = safe_div(i1.b, i2.a);
        int64_t bd = safe_div(i1.b, i2.b);

        a = bc >= bd ? bd : bc;
        b = ac >= ad ? ac : ad;
        return interval_create(ctx, a, b);
    }
    else {

        // Here we manually do the two following intersection instead of
        // using interval_intersect because we don't want to over-approximate
        // the result (if the result is not in the domain).
        //
        // Otherwise in some cases this function will not terminate.
        // For example this occurs when i2 = Top and we are on the
        // constant propagation domain.

        // Intersect [1,+INF) with i2
        Interval pos = {
            .type = INTERVAL_STD,
            .a = 1,
            .b = INTERVAL_PLUS_INF,
        };
        pos.a = i2.a >= pos.a ? i2.a : pos.a;
        pos.b = i2.b >= pos.b ? pos.b : i2.b;
        if (pos.a > pos.b) {
            pos.type = INTERVAL_BOTTOM;
        }

        // Intersect (-INF,-1] with i2
        Interval neg = {
            .type = INTERVAL_STD,
            .a = INTERVAL_MIN_INF,
            .b = -1,
        };
        neg.a = i2.a >= neg.a ? i2.a : neg.a;
        neg.b = i2.b >= neg.b ? neg.b : i2.b;
        if (neg.a > neg.b) {
            neg.type = INTERVAL_BOTTOM;
        }

        Interval positive_part = interval_div(ctx, i1, pos);
        Interval negative_part = interval_div(ctx, i1, neg);

        return interval_union(ctx, positive_part, negative_part);
    }
}

// Widening operator (using thresholds)
static Interval interval_widening(const Abstract_Interval_Ctx *ctx, Interval i1, Interval i2) {

    // Bottom handling
    if (i1.type == INTERVAL_BOTTOM) return i2;
    if (i2.type == INTERVAL_BOTTOM) return i1;

    // Rule: [i1.a,i1.b] ▽ [i2.a,i2.b] = [x,y]
    // where: if i1.a <= i2.a then x = i1.a else x = max{k ∈ ctx->widening_points | k <= i2.a}.
    //        if i1.b >= i2.b then y = i1.b else y = min{k ∈ ctx->widening_points | k >= i2.b}.

    // If i2.a is -INF then the else branch will not set x, so we set it here in advance to -INF.
    // Same for y.
    int64_t x = INTERVAL_MIN_INF;
    int64_t y = INTERVAL_PLUS_INF;

    if (i1.a <= i2.a) {
        x = i1.a;
    }
    else {
        for (size_t i = 0; i < ctx->widening_points.count; ++i) {
            if (ctx->widening_points.data[i] > i2.a) {
                x = ctx->widening_points.data[i-1];
                break;
            }
        }
    }

    if (i1.b >= i2.b) {
        y = i1.b;
    }
    else {
        for (int i = ctx->widening_points.count - 1; i >= 0; --i) {
            if (ctx->widening_points.data[i] < i2.b) {
                y = ctx->widening_points.data[i+1];
                break;
            }
        }
    }

    return interval_create(ctx, x, y);
}

/* //////////////////////////////////////////////////////////////////////////////////// */

/* ============================== Interval backward ops =============================== */

typedef struct {
    Interval a;
    Interval b;
} Interval_Tuple;

static Interval_Tuple interval_backward_plus(const Abstract_Interval_Ctx *ctx, Interval x, Interval y, Interval r) {
    Interval_Tuple t = {0};

    t.a = interval_intersect(ctx, x, interval_minus(ctx, r, y));
    t.b = interval_intersect(ctx, y, interval_minus(ctx, r, x));

    return t;
}

static Interval_Tuple interval_backward_minus(const Abstract_Interval_Ctx *ctx, Interval x, Interval y, Interval r) {
    Interval_Tuple t = {0};

    t.a = interval_intersect(ctx, x, interval_plus(ctx, r, y));
    t.b = interval_intersect(ctx, y, interval_minus(ctx, x, r));

    return t;
}

static Interval_Tuple interval_backward_mult(const Abstract_Interval_Ctx *ctx, Interval x, Interval y, Interval r) {
    Interval_Tuple t = {0};

    t.a = interval_intersect(ctx, x, interval_div(ctx, r, y));
    t.b = interval_intersect(ctx, y, interval_div(ctx, r, x));

    return t;
}

static Interval_Tuple interval_backward_div(const Abstract_Interval_Ctx *ctx, Interval x, Interval y, Interval r) {
    Interval_Tuple t = {0};
    Interval s = interval_plus(ctx, r, interval_create(ctx, -1, 1));

    t.a = interval_intersect(ctx, x, interval_mult(ctx, s, y));
    t.b = interval_intersect(ctx, y, interval_union(ctx, interval_div(ctx, x, s), interval_create(ctx, 0, 0)));

    return t;
}

/* //////////////////////////////////////////////////////////////////////////////////// */

Abstract_Interval_Ctx *abstract_interval_ctx_init(int64_t m, int64_t n, Variables vars, Constants c) {
    Abstract_Interval_Ctx *ctx = xmalloc(sizeof(Abstract_Interval_Ctx));

    // Setting the props
    ctx->m = m;
    ctx->n = n;
    ctx->vars = vars;
    ctx->widening_points = c;

    return ctx;
}

void abstract_interval_ctx_free(Abstract_Interval_Ctx *ctx) {
    free(ctx->vars.var);
    free(ctx->widening_points.data);
    free(ctx);
}

Interval *abstract_interval_state_init(const Abstract_Interval_Ctx *ctx) {
    Interval *s = xmalloc(sizeof(Interval) * ctx->vars.count);

    // By default set to bottom
    abstract_interval_state_set_bottom(ctx, s);

    return s;
}

void abstract_interval_state_free(Interval *s) {
    free(s);
}

void abstract_interval_state_set_bottom(const Abstract_Interval_Ctx *ctx, Interval *s) {
    // Since BOTTOM enum value = 0, all the intervals will be bottom
    memset(s, 0, sizeof(Interval) * ctx->vars.count);
}

void abstract_interval_state_set_top(const Abstract_Interval_Ctx *ctx, Interval *s) {
    // Set every interval to TOP
    for (size_t i = 0; i < ctx->vars.count; ++i) {
        s[i].type = INTERVAL_STD;
        s[i].a = INTERVAL_MIN_INF;
        s[i].b = INTERVAL_PLUS_INF;
    }
}

void abstract_interval_state_set_from_config(const Abstract_Interval_Ctx *ctx, Interval *s, FILE *fp) {
    // Set all to TOP
    abstract_interval_state_set_top(ctx, s);

    char line[256];
    while (fgets(line, 256, fp) != NULL) {

        // Get variable len
        size_t var_len = 0;
        while (line[var_len] != ':' && line[var_len] != '\0') {
            var_len++;
        }

        // Get the variable interval index
        size_t var_index = 0;
        bool found = false;
        for (size_t i = 0; i < ctx->vars.count; ++i) {
            if (var_len == ctx->vars.var[i].len) {
                if (strncmp(line, ctx->vars.var[i].name, var_len) == 0) {
                    var_index = i;
                    found = true;
                }
            }
        }
        if (!found) continue;

        // Parse the interval value
        const char *c = line + var_len;

        // Skip ': '
        if (*c != ':') continue;
        c++;
        if (*c != ' ') continue;
        c++;

        // TOP
        if (strncmp(c, "TOP", 3) == 0) {
            c += 3;

            // Check endline/EOF
            if (*c != '\n' && *c != '\0') continue;

            s[var_index].type = INTERVAL_STD;
            s[var_index].a = INTERVAL_MIN_INF;
            s[var_index].b = INTERVAL_PLUS_INF;
        }
        // BOTTOM
        else if (strncmp(c, "BOTTOM", 6) == 0) {
            c += 6;

            // Check endline/EOF
            if (*c != '\n' && *c != '\0') continue;

            s[var_index].type = INTERVAL_BOTTOM;
        }
        // Interval [a,b]
        else {
            int64_t a;
            int64_t b;
            char *endptr;

            // Skip '['
            if (*c != '[') continue;
            c++;

            // Read a
            if (strncmp(c, "-INF", 4) == 0) {
                a = INTERVAL_MIN_INF;
                c += 4;
            }
            else if (strncmp(c, "+INF", 4) == 0) {
                a = INTERVAL_PLUS_INF;
                c += 4;
            }
            else {
                a = strtoll(c, &endptr, 10);
                if (*endptr != ',') continue;
                c = endptr;
            }

            // Skip ','
            if (*c != ',') continue;
            c++;

            // Read b
            if (strncmp(c, "-INF", 4) == 0) {
                b = INTERVAL_MIN_INF;
                c += 4;
            }
            else if (strncmp(c, "+INF", 4) == 0) {
                b = INTERVAL_PLUS_INF;
                c += 4;
            }
            else {
                b = strtoll(c, &endptr, 10);
                if (*endptr != ']') continue;
                c = endptr;
            }

            // Skip ']'
            if (*c != ']') continue;
            c++;

            // Check endline/EOF
            if (*c != '\n' && *c != '\0') continue;

            s[var_index] = interval_create(ctx, a, b);
        }
    }
}

void abstract_interval_state_print(const Abstract_Interval_Ctx *ctx, const Interval *s, FILE *fp) {
    for (size_t i = 0; i < ctx->vars.count; ++i) {
        const char *var_name = ctx->vars.var[i].name;
        size_t var_len = ctx->vars.var[i].len;

        if (s[i].type == INTERVAL_BOTTOM) {
            fprintf(fp, "  (%.*s) = BOTTOM\n", (int)var_len, var_name);
        }
        else if (s[i].a == INTERVAL_MIN_INF && s[i].b == INTERVAL_PLUS_INF) {
            fprintf(fp, "  (%.*s) = TOP\n", (int)var_len, var_name);
        }
        else if (s[i].a == INTERVAL_MIN_INF) {
            fprintf(fp, "  (%.*s) = (-INF, %"PRId64"]\n", (int)var_len, var_name, s[i].b);
        }
        else if (s[i].b == INTERVAL_PLUS_INF) {
            fprintf(fp, "  (%.*s) = [%"PRId64", +INF)\n", (int)var_len, var_name, s[i].a);
        }
        else {
            fprintf(fp, "  (%.*s) = [%"PRId64", %"PRId64"]\n", (int)var_len, var_name, s[i].a, s[i].b);
        }
    }
    printf("\n");
}

bool abstract_interval_state_leq(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    bool result = true;

    // s1 <= s2 if all elements of s1 are <= all elements of s2
    for (size_t i = 0; i < ctx->vars.count; ++i) {
        result = result && interval_leq(s1[i], s2[i]);
    }

    return result;
}

Interval *abstract_interval_state_union(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    Interval *res = abstract_interval_state_init(ctx);

    for (size_t i = 0; i < ctx->vars.count; ++i) {
        res[i] = interval_union(ctx, s1[i], s2[i]);
    }

    return res;
}

Interval *abstract_interval_state_intersect(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    Interval *res = abstract_interval_state_init(ctx);

    for (size_t i = 0; i < ctx->vars.count; ++i) {
        res[i] = interval_intersect(ctx, s1[i], s2[i]);
    }

    return res;
}

Interval *abstract_interval_state_widening(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2) {
    Interval *res = abstract_interval_state_init(ctx);

    for (size_t i = 0; i < ctx->vars.count; ++i) {
        res[i] = interval_widening(ctx, s1[i], s2[i]);
    }

    return res;
}

/* ================================ Commands execution ================================ */

// Returns a new heap allocated state with the same elements of 's'
static Interval *clone_state(const Abstract_Interval_Ctx *ctx, const Interval *s) {
    Interval *res = abstract_interval_state_init(ctx);
    memcpy(res, s, sizeof(Interval) * ctx->vars.count);
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
            // Get the assigned variable
            String var = node->as.var;

            // Get the interval for that variable (here a variable must be found by contruction)
            size_t var_index = 0;
            for (size_t i = 0; i < ctx->vars.count; ++i) {
                if (var.len == ctx->vars.var[i].len) {
                    if (strncmp(var.name, ctx->vars.var[i].name, var.len) == 0) {
                        var_index = i;
                    }
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
        {
            Interval i1 = exec_aexpr(ctx, s, node->as.child.left);
            Interval i2 = exec_aexpr(ctx, s, node->as.child.right);
            return interval_mult(ctx, i1, i2);
        }
    case NODE_DIV:
        {
            Interval i1 = exec_aexpr(ctx, s, node->as.child.left);
            Interval i2 = exec_aexpr(ctx, s, node->as.child.right);
            return interval_div(ctx, i1, i2);
        }
    default:
        assert(0 && "UNREACHABLE");
    }
}

static Interval *abstract_interval_state_exec_assign(const Abstract_Interval_Ctx *ctx, const Interval *s, const AST_Node *assign) {

    // Get the assigned variable
    String var = assign->as.child.left->as.var;

    // Get the interval for that variable
    size_t var_index = 0;
    for (size_t i = 0; i < ctx->vars.count; ++i) {
        if (strncmp(var.name, ctx->vars.var[i].name, var.len) == 0) {
            var_index = i;
        }
    }

    // Compute the right expression of assign node
    Interval aexpr_res = exec_aexpr(ctx, s, assign->as.child.right);

    // Create the new state and return
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
    case NODE_BOOL_LITERAL:
        {
            bool value = command->as.boolean;
            if (value) {
                // No filtering
                res = clone_state(ctx, s);
            } else {
                // Always false, so return bottom
                res = abstract_interval_state_init(ctx);
                abstract_interval_state_set_bottom(ctx, res);
            }
            break;
        }
    case NODE_EQ:
    case NODE_LEQ:
    case NODE_NOT:
    case NODE_AND:
        // TODO: Advanced abstract tests from Minè Tutorial.
        // That needs the forward traverse of the tree, but
        // the intermediate results needs to be saved in the tree.
        //
        // Then after applying the condition the backward traverse
        // is applied, and for that the backward ops are needed (page 227).
        //
        // After this, we have a tree with the leaf variable that represents
        // the filtered states.
        res = clone_state(ctx, s);
        break;
    case NODE_SKIP:
        res = clone_state(ctx, s);
        break;
    default:
        assert(0 && "UNREACHABLE");
    }

    return res;
}

/* //////////////////////////////////////////////////////////////////////////////////// */
