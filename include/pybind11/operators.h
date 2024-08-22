#pragma once

#include "pybind11.h"

namespace pkbind::impl {

enum op_id : int {
    op_add,
    op_sub,
    op_mul,
    op_div,
    op_mod,
    op_divmod,
    op_pow,
    op_lshift,
    op_rshift,
    op_and,
    op_xor,
    op_or,
    op_neg,
    op_pos,
    op_abs,
    op_invert,
    op_int,
    op_long,
    op_float,
    op_str,
    op_cmp,
    op_gt,
    op_ge,
    op_lt,
    op_le,
    op_eq,
    op_ne,
    op_iadd,
    op_isub,
    op_imul,
    op_idiv,
    op_imod,
    op_ilshift,
    op_irshift,
    op_iand,
    op_ixor,
    op_ior,
    op_complex,
    op_bool,
    op_nonzero,
    op_repr,
    op_truediv,
    op_itruediv,
    op_hash
};

enum op_type : int {
    op_l, /* base type on left */
    op_r, /* base type on right */
    op_u  /* unary operator */
};

struct self_t {};

const static self_t self = self_t();

/// Type for an unused type slot
struct undefined_t {};

/// Don't warn about an unused variable
inline self_t __self() { return self; }

/// base template of operator implementations
template <op_id, op_type, typename B, typename L, typename R>
struct op_impl {};

/// Operator implementation generator
template <op_id id, op_type ot, typename L, typename R>
struct op_ {
    constexpr static bool op_enable_if_hook = true;

    template <typename Class, typename... Extra>
    void execute(Class& cl, const Extra&... extra) const {
        using Base = typename Class::underlying_type;
        using L_type = std::conditional_t<std::is_same<L, self_t>::value, Base, L>;
        using R_type = std::conditional_t<std::is_same<R, self_t>::value, Base, R>;
        using op = op_impl<id, ot, Base, L_type, R_type>;
        cl.def(op::name(), &op::execute, extra...);
    }

    template <typename Class, typename... Extra>
    void execute_cast(Class& cl, const Extra&... extra) const {
        using Base = typename Class::type;
        using L_type = std::conditional_t<std::is_same<L, self_t>::value, Base, L>;
        using R_type = std::conditional_t<std::is_same<R, self_t>::value, Base, R>;
        using op = op_impl<id, ot, Base, L_type, R_type>;
        cl.def(op::name(), &op::execute_cast, extra...);
    }
};

#define PKBIND_BINARY_OPERATOR(id, rid, op, expr)                                                                      \
    template <typename B, typename L, typename R>                                                                      \
    struct op_impl<op_##id, op_l, B, L, R> {                                                                           \
        static char const* name() { return "__" #id "__"; }                                                            \
        static auto execute(const L& l, const R& r) -> decltype(expr) { return (expr); }                               \
        static B execute_cast(const L& l, const R& r) { return B(expr); }                                              \
    };                                                                                                                 \
                                                                                                                       \
    template <typename B, typename L, typename R>                                                                      \
    struct op_impl<op_##id, op_r, B, L, R> {                                                                           \
        static char const* name() { return "__" #rid "__"; }                                                           \
        static auto execute(const R& r, const L& l) -> decltype(expr) { return (expr); }                               \
        static B execute_cast(const R& r, const L& l) { return B(expr); }                                              \
    };                                                                                                                 \
                                                                                                                       \
    inline op_<op_##id, op_l, self_t, self_t> op(const self_t&, const self_t&) {                                       \
        return op_<op_##id, op_l, self_t, self_t>();                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template <typename T>                                                                                              \
    op_<op_##id, op_l, self_t, T> op(const self_t&, const T&) {                                                        \
        return op_<op_##id, op_l, self_t, T>();                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    template <typename T>                                                                                              \
    op_<op_##id, op_r, T, self_t> op(const T&, const self_t&) {                                                        \
        return op_<op_##id, op_r, T, self_t>();                                                                        \
    }

#define PKBIND_INPLACE_OPERATOR(id, op, expr)                                                                          \
    template <typename B, typename L, typename R>                                                                      \
    struct op_impl<op_##id, op_l, B, L, R> {                                                                           \
        static char const* name() { return "__" #id "__"; }                                                            \
        static auto execute(L& l, const R& r) -> decltype(expr) { return expr; }                                       \
        static B execute_cast(L& l, const R& r) { return B(expr); }                                                    \
    };                                                                                                                 \
                                                                                                                       \
    template <typename T>                                                                                              \
    op_<op_##id, op_l, self_t, T> op(const self_t&, const T&) {                                                        \
        return op_<op_##id, op_l, self_t, T>();                                                                        \
    }

#define PKBIND_UNARY_OPERATOR(id, op, expr)                                                                            \
    template <typename B, typename L>                                                                                  \
    struct op_impl<op_##id, op_u, B, L, undefined_t> {                                                                 \
        static char const* name() { return "__" #id "__"; }                                                            \
        static auto execute(const L& l) -> decltype(expr) { return expr; }                                             \
        static B execute_cast(const L& l) { return B(expr); }                                                          \
    };                                                                                                                 \
                                                                                                                       \
    inline op_<op_##id, op_u, self_t, undefined_t> op(const self_t&) {                                                 \
        return op_<op_##id, op_u, self_t, undefined_t>();                                                              \
    }

PKBIND_BINARY_OPERATOR(sub, rsub, operator-, l - r)
PKBIND_BINARY_OPERATOR(add, radd, operator+, l + r)
PKBIND_BINARY_OPERATOR(mul, rmul, operator*, l* r)
PKBIND_BINARY_OPERATOR(truediv, rtruediv, operator/, l / r)
PKBIND_BINARY_OPERATOR(mod, rmod, operator%, l % r)
PKBIND_BINARY_OPERATOR(lshift, rlshift, operator<<, l << r)
PKBIND_BINARY_OPERATOR(rshift, rrshift, operator>>, l >> r)
PKBIND_BINARY_OPERATOR(and, rand, operator&, l& r)
PKBIND_BINARY_OPERATOR(xor, rxor, operator^, l ^ r)
PKBIND_BINARY_OPERATOR(eq, eq, operator==, l == r)
PKBIND_BINARY_OPERATOR(ne, ne, operator!=, l != r)
PKBIND_BINARY_OPERATOR(or, ror, operator|, l | r)
PKBIND_BINARY_OPERATOR(gt, lt, operator>, l > r)
PKBIND_BINARY_OPERATOR(ge, le, operator>=, l >= r)
PKBIND_BINARY_OPERATOR(lt, gt, operator<, l < r)
PKBIND_BINARY_OPERATOR(le, ge, operator<=, l <= r)
// PKBIND_BINARY_OPERATOR(pow,       rpow,         pow,          std::pow(l,  r))
PKBIND_INPLACE_OPERATOR(iadd, operator+=, l += r)
PKBIND_INPLACE_OPERATOR(isub, operator-=, l -= r)
PKBIND_INPLACE_OPERATOR(imul, operator*=, l *= r)
PKBIND_INPLACE_OPERATOR(itruediv, operator/=, l /= r)
PKBIND_INPLACE_OPERATOR(imod, operator%=, l %= r)
PKBIND_INPLACE_OPERATOR(ilshift, operator<<=, l <<= r)
PKBIND_INPLACE_OPERATOR(irshift, operator>>=, l >>= r)
PKBIND_INPLACE_OPERATOR(iand, operator&=, l &= r)
PKBIND_INPLACE_OPERATOR(ixor, operator^=, l ^= r)
PKBIND_INPLACE_OPERATOR(ior, operator|=, l |= r)

PKBIND_UNARY_OPERATOR(neg, operator-, -l)
PKBIND_UNARY_OPERATOR(pos, operator+, +l)

// WARNING: This usage of `abs` should only be done for existing STL overloads.
// Adding overloads directly in to the `std::` namespace is advised against:
// https://en.cppreference.com/w/cpp/language/extending_std

// PKBIND_UNARY_OPERATOR(abs, abs, std::abs(l))
// PKBIND_UNARY_OPERATOR(hash, hash, std::hash<L>()(l))
// PKBIND_UNARY_OPERATOR(invert, operator~, (~l))
// PKBIND_UNARY_OPERATOR(bool, operator!, !!l)
// PKBIND_UNARY_OPERATOR(int, int_, (int)l)
// PKBIND_UNARY_OPERATOR(float, float_, (double)l)

#undef PKBIND_BINARY_OPERATOR
#undef PKBIND_INPLACE_OPERATOR
#undef PKBIND_UNARY_OPERATOR

}  // namespace pkbind::impl

namespace pkbind {

using impl::self;

}  // namespace pkbind
