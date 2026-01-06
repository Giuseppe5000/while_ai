#ifndef WHILE_AI_ABSTRACT_INTERVAL_DOM_
#define WHILE_AI_ABSTRACT_INTERVAL_DOM_

#include "../lang/parser.h"
#include "../common.h"
#include <stdio.h>

#define INTERVAL_PLUS_INF INT64_MAX
#define INTERVAL_MIN_INF INT64_MIN

enum Interval_Type {
    INTERVAL_BOTTOM, // Bottom element
    INTERVAL_STD,    // Standard interval (Top included)
};

// Define the integer interval [a,b].
// If a or b are INF, then their value is:
//     INTERVAL_PLUS_INF for represent infinite.
//     INTERVAL_MIN_INF for represent -infinite.
typedef struct {
    enum Interval_Type type;
    int64_t a;
    int64_t b;
} Interval;

typedef struct Abstract_Interval_Ctx Abstract_Interval_Ctx;

// Return the domain context, setting parameters for Int(m,n) and the variables of the program.
//
// [NOTE]: The ownership of the 'vars' array is up to the caller.
Abstract_Interval_Ctx *abstract_interval_ctx_init(int64_t m, int64_t n, Variables vars, Constants c);

// Free the context
void abstract_interval_ctx_free(Abstract_Interval_Ctx *ctx);

// Create an Abstract Interval State in the domain of parametric intervals (m,n).
// The state is just an array of intervals.
//
// The array size depdens on the context, if variable number is N then an array
// of N Intervals will be returned.
//
// [NOTE]: This function sets all intervals to bottom.
//
// Given m,n ∈ (Z union {-INF, +INF}),
// the domain of parametric intervals is defined as the union of this sets:
//     { BOTTOM, TOP }
//     { [k,k] | k ∈ Z }
//     { [a, b] | a < b, [a, b] ⊆ [m, n] }
//     { (-INF, k] | k ∈ [m, n] }
//     { [k, +INF) | k ∈ [m, n] }
Interval *abstract_interval_state_init(const Abstract_Interval_Ctx *ctx);

// Free the abstract state
void abstract_interval_state_free(Interval *s);

// Helper functions to set all the intervals of a state to bottom or top
void abstract_interval_state_set_bottom(const Abstract_Interval_Ctx *ctx, Interval *s);
void abstract_interval_state_set_top(const Abstract_Interval_Ctx *ctx, Interval *s);

// Prints the state intervals (plain text) to fp
void abstract_interval_state_print(const Abstract_Interval_Ctx *ctx, const Interval *s, FILE *fp);

// Abstract commands
Interval *abstract_interval_state_exec_command(const Abstract_Interval_Ctx *ctx, const Interval *s, const AST_Node *command);

// Compare function, returns true if state 's1' <= 's2'
bool abstract_interval_state_leq(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2);

// Union
Interval *abstract_interval_state_union(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2);

// Intersection
Interval *abstract_interval_state_intersect(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2);

// Widening
Interval *abstract_interval_state_widening(const Abstract_Interval_Ctx *ctx, const Interval *s1, const Interval *s2);

#endif  // WHILE_AI_ABSTRACT_INTERVAL_DOM_
