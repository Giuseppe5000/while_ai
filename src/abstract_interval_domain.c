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
Create an interval beloging to the domain, if a or b.

NOTE: if [a, b] does not belong to the domain dom, an assertion will occurr.
*/
// static Interval interval_create(const Abstract_Int_State *s, int64_t a, int64_t b);x

/*
Check if interval 'a' is a less than or equal to interval 'b'.
Returns true if a <= b, false otherwise.
*/
// static bool interval_leq(const Abstract_Int_State *s, Interval a, Interval b);

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
