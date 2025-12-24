#ifndef WHILE_AI_ABSTRACT_INTERVAL_DOM_
#define WHILE_AI_ABSTRACT_INTERVAL_DOM_

#include "../lang/parser.h"
#include <stdint.h>
#include <stddef.h>

typedef struct Abstract_Int_State Abstract_Int_State;

/*
Set the parameters for Int(m,n).
If this function is not called then the abstract states will use Int(-inf, inf).

[NOTE]: This function is not thread safe.
*/
void abstract_int_set_params(int64_t m, int64_t n);

/*
Create an Abstract State in the domain of parametric intervals (m,n)
with all variables (defined in 'var_names') set to bottom/top depending on the specific function.

The domain of parametric intervals is defined as the union of this sets:
    { BOTTOM, TOP }
    { [k,k] | k ∈ Z }
    { [a, b] | a < b, [a, b] ⊆ [m, n] }
    { (-INF, k] | k ∈ [m, n] }
    { [k, +INF) | k ∈ [m, n] }
*/
Abstract_Int_State *abstract_int_state_init_bottom(const char **var_names, size_t var_count);
Abstract_Int_State *abstract_int_state_init_top(const char **var_names, size_t var_count);

/* Free the abstract state */
void abstract_int_state_free(Abstract_Int_State *s);

/* Abstract commands */
Abstract_Int_State *abstract_int_state_exec_command(const Abstract_Int_State *s, const AST_Node *command);

/* Compare */
bool abstract_int_state_leq(const Abstract_Int_State *s1, const Abstract_Int_State *s2);

/* Union */
Abstract_Int_State *abstract_int_state_union(const Abstract_Int_State *s1, const Abstract_Int_State *s2);

/* Widening */
Abstract_Int_State *abstract_int_state_widening(const Abstract_Int_State *s1, const Abstract_Int_State *s2);

/* Narrowing */
Abstract_Int_State *abstract_int_state_narrowing(const Abstract_Int_State *s1, const Abstract_Int_State *s2);

#endif  /* WHILE_AI_ABSTRACT_INTERVAL_DOM_ */
