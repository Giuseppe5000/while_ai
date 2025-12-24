#ifndef WHILE_AI_ABSTRACT_INTERVAL_DOM_
#define WHILE_AI_ABSTRACT_INTERVAL_DOM_

#include "../lang/parser.h"
#include <stdint.h>
#include <stddef.h>

#define INTERVAL_PLUS_INF INT64_MAX
#define INTERVAL_MIN_INF INT64_MIN

typedef struct Interval Interval;

/*
The Abstract State is an array of variables
represented within the domain of intervals.
*/
typedef struct {
    struct {
        int64_t m;
        int64_t n;
    } interval_params;

    Interval *values;
    const char **var_names;
    size_t count;
} Abstract_Int_State;

/*
Create an Abstract State in the domain of parametric intervals (m,n)
with all variables (defined in 'var_names') set to bottom/top depending on the specific function.

The domain of parametric intervals is defined as the union of this sets:
    { BOTTOM, TOP }
    { [k,k] | k ∈ Z }
    { [a, b] | a < b, [a, b] ⊆ [m, n] }
    { (-INF, k] | k ∈ [m, n] }
    { [k, +INF) | k ∈ [m, n] }

NOTE: m,n represents -infinite/infinite if they are INTERVAL_MIN_INF/INTERVAL_PLUS_INF.
*/
Abstract_Int_State *abstract_int_state_init_bottom(int64_t m, int64_t n, const char **var_names, size_t var_count);
Abstract_Int_State *abstract_int_state_init_top(int64_t m, int64_t n, const char **var_names, size_t var_count);

/* Abstract commands */
Abstract_Int_State *abstract_int_state_exec_command(const Abstract_Int_State *s, const AST_Node *command);

/* Union */
Abstract_Int_State *abstract_int_state_union(const Abstract_Int_State *s1, const Abstract_Int_State *s2);

/* Widening */
Abstract_Int_State *abstract_int_state_widening(const Abstract_Int_State *s1, const Abstract_Int_State *s2);

/* Free the abstract state */
void abstract_int_state_free(Abstract_Int_State *s);

#endif  /* WHILE_AI_ABSTRACT_INTERVAL_DOM_ */
