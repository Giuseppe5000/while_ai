#include "abstract_interval_domain_wrap.h"
#include "../domain/abstract_interval_domain.h"

static inline void abstract_interval_state_free_wrapper(Abstract_State *s) {
    abstract_interval_state_free((Interval *) s);
}

static inline void abstract_interval_ctx_free_wrapper(Abstract_Dom_Ctx *ctx) {
    abstract_interval_ctx_free((Abstract_Interval_Ctx *)ctx);
}

static inline void abstract_interval_state_set_bottom_wrapper(const Abstract_Dom_Ctx *ctx, Abstract_State *s) {
    abstract_interval_state_set_bottom((const Abstract_Interval_Ctx *) ctx, (Interval *) s);
}

static inline void abstract_interval_state_set_top_wrapper(const Abstract_Dom_Ctx *ctx, Abstract_State *s) {
    abstract_interval_state_set_top((const Abstract_Interval_Ctx *) ctx, (Interval *) s);
}

static inline void abstract_interval_state_print_wrapper(const Abstract_Dom_Ctx *ctx, const Abstract_State *s, FILE *fp) {
    abstract_interval_state_print((const Abstract_Interval_Ctx *) ctx, (const Interval *) s, fp);
}

static inline Abstract_State *abstract_interval_state_exec_command_wrapper(const Abstract_Dom_Ctx *ctx, const Abstract_State *s, const AST_Node *command) {
    return (Abstract_State *) abstract_interval_state_exec_command((const Abstract_Interval_Ctx *) ctx, (const Interval *) s, command);
}

static inline bool abstract_interval_state_leq_wrapper(const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2) {
    return abstract_interval_state_leq((const Abstract_Interval_Ctx *) ctx, (const Interval *) s1, (const Interval *) s2);
}

static inline Abstract_State *abstract_interval_state_union_wrapper(const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2) {
    return (Abstract_State *) abstract_interval_state_union((const Abstract_Interval_Ctx *) ctx, (const Interval *) s1, (const Interval *) s2);
}

static inline Abstract_State *abstract_interval_state_widening_wrapper(const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2) {
    return (Abstract_State *) abstract_interval_state_widening((const Abstract_Interval_Ctx *) ctx, (const Interval *) s1, (const Interval *) s2);
}

static inline Abstract_State *abstract_interval_state_narrowing_wrapper(const Abstract_Dom_Ctx *ctx, const Abstract_State *s1, const Abstract_State *s2) {
    return (Abstract_State *) abstract_interval_state_narrowing((const Abstract_Interval_Ctx *) ctx, (const Interval *) s1, (const Interval *) s2);
}

const Abstract_Dom_Ops abstract_interval_ops = {
    .ctx_free = abstract_interval_ctx_free_wrapper,
    .state_free = abstract_interval_state_free_wrapper,
    .state_set_bottom = abstract_interval_state_set_bottom_wrapper,
    .state_set_top = abstract_interval_state_set_top_wrapper,
    .state_print = abstract_interval_state_print_wrapper,
    .exec_command = abstract_interval_state_exec_command_wrapper,
    .state_leq = abstract_interval_state_leq_wrapper,
    .union_ = abstract_interval_state_union_wrapper,
    .widening = abstract_interval_state_widening_wrapper,
    .narrowing = abstract_interval_state_narrowing_wrapper,
};
