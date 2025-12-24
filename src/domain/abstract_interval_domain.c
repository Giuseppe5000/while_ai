#include "abstract_interval_domain.h"

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
static Interval interval_create(int64_t m, int64_t n, int64_t a, int64_t b) {
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
            .a = m,
            .b = n,
        };

        if (interval_leq(i, i_mn)) {
            return i;
        }
    }

    /* { (-INF, k] | k ∈ [m, n] } */
    if (a == INTERVAL_MIN_INF && (b >= m && b <= n)) {
        return i;
    }

    /* { [k, +INF) | k ∈ [m, n] } */
    if (b == INTERVAL_PLUS_INF && (a >= m && a <= n)) {
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
