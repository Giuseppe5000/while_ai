#include "abstract_interval_domain.h"
#include "../common.h"
#include <stdlib.h>
#include <string.h>

#define INTERVAL_PLUS_INF INT64_MAX
#define INTERVAL_MIN_INF INT64_MIN

enum Interval_Type {
    INTERVAL_BOTTOM, /* Bottom element */
    INTERVAL_STD,    /* Standard interval (inf included) */
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

/* By default the Interval is standard Int domain */
static int64_t int_m = INTERVAL_MIN_INF;
static int64_t int_n = INTERVAL_PLUS_INF;
static size_t var_count = 0;

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
static Interval interval_create(int64_t a, int64_t b) {
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
        return i;
    }

    /* { [a, b] | a < b, [a, b] ⊆ [m, n] } */
    if (a < b) {
        Interval i_mn = {
            .type = INTERVAL_STD,
            .a = int_m,
            .b = int_n,
        };

        if (interval_leq(i, i_mn)) {
            return i;
        }
    }

    /* { (-INF, k] | k ∈ [m, n] } */
    if (a == INTERVAL_MIN_INF && (b >= int_m && b <= int_n)) {
        return i;
    }

    /* { [k, +INF) | k ∈ [m, n] } */
    if (b == INTERVAL_PLUS_INF && (a >= int_m && a <= int_n)) {
        return i;
    }

    /* [a,b] is not in the domain, so return Top */
    i.type = INTERVAL_STD;
    i.a = INTERVAL_MIN_INF;
    i.b = INTERVAL_PLUS_INF;
    return i;
}


/* Returns the union of intervals 'a' and 'b' */
// static Interval interval_union(const Abstract_Int_State *s, Interval a, Interval b);

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

void abstract_int_configure(int64_t m, int64_t n, size_t _var_count) {
    int_m = m;
    int_n = n;
    var_count = _var_count;
}

Interval *abstract_int_state_init(size_t count) {
    Interval *s = xmalloc(sizeof(Interval) * var_count * count);

    /* By default set all to bottom*/
    memset(s, 0, sizeof(Interval) * var_count * count);

    return s;
}

void abstract_int_state_set_bottom(Interval *s) {
    /* Since BOTTOM enum = 0, all the vars will be bottom */
    memset(s, 0, sizeof(Interval) * var_count);
}

void abstract_int_state_set_top(Interval *s) {
    /* Set every interval to TOP */
    for (size_t i = 0; i < var_count; ++i) {
        s[i].type = INTERVAL_STD;
        s[i].a = INTERVAL_MIN_INF;
        s[i].b = INTERVAL_PLUS_INF;
    }
}

void abstract_int_state_free(Interval *s) {
    free(s);
}
