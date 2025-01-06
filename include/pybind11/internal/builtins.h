#pragma once

#include "types.h"

namespace pkbind {

template <typename... Args>
inline void print(Args&&... args) {
    handle print = py_getbuiltin(py_name("print"));
    print(std::forward<Args>(args)...);
}

inline object eval(std::string_view code, handle globals = none(), handle locals = none()) {
    if(globals.is_none() && locals.is_none()) {
        std::string src{code};
        raise_call<py_eval>(src.c_str(), nullptr);
        return object::from_ret();
    } else {
        handle eval = py_getbuiltin(py_name("eval"));
        return eval(str(code),
                    globals.is_none() ? dict() : globals,
                    locals.is_none() ? dict() : locals);
    }
}

inline object exec(std::string_view code, handle globals = none(), handle locals = none()) {
    if(globals.is_none() && locals.is_none()) {
        std::string src{code};
        raise_call<py_exec>(src.c_str(), "exec", EXEC_MODE, nullptr);
        return object::from_ret();
    } else {
        handle exec = py_getbuiltin(py_name("exec"));
        return exec(str(code), globals, locals);
    }
}

inline object locals() {
    handle locals = py_getbuiltin(py_name("locals"));
    return locals();
}

inline object globals() {
    handle globals = py_getbuiltin(py_name("globals"));
    return globals();
}

inline bool hasattr(handle obj, name name) {
    auto pc = py_peek(0);
    auto result = py_getattr(obj.ptr(), name.index());
    if(result) {
        return true;
    } else {
        py_clearexc(pc);
        return false;
    }
}

inline object getattr(handle obj, name name) {
    raise_call<py_getattr>(obj.ptr(), name.index());
    return object::from_ret();
}

inline void setattr(handle obj, name name, handle value) {
    raise_call<py_setattr>(obj.ptr(), name.index(), value.ptr());
}

inline void delattr(handle obj, name name) { raise_call<py_delattr>(obj.ptr(), name.index()); }

inline py_i64 hash(handle obj) {
    py_i64 result = 0;
    raise_call<py_hash>(obj.ptr(), &result);
    return result;
}

inline bool isinstance(handle obj, type type) { return py_isinstance(obj.ptr(), type.index()); }

inline bool python_error::match(type type) const { return isinstance(m_exception.ptr(), type); }

inline bool error_already_set::match(type type) const { return py_matchexc(type.index()); }

template <typename T>
constexpr inline bool is_pyobject_v =
    std::is_base_of_v<object, std::decay_t<T>> || std::is_same_v<type, T>;

template <typename T>
inline type type::of() {
    if constexpr(is_pyobject_v<T>) {
        if constexpr(is_check_v<T>) {
            return type(tp_object);
        } else {
            return type(T::type_or_check());
        }
    } else {
        auto it = m_type_map.find(typeid(T));
        if(it != m_type_map.end()) {
            return type(it->second);
        } else {
            // if not found, raise error
            std::string msg = "can not c++ instance cast to object, type: {";
            msg += type_name<T>();
            msg += "} is not registered.";
            throw std::runtime_error(msg);
        }
    }
}

template <typename T>
inline bool type::isinstance(handle obj) {
    if constexpr(is_pyobject_v<T>) {
        // for every python object wrapper type, there must be a `type_or_check` method.
        // for some types, it returns the underlying type in pkpy, e.g., `int_` -> `tp_int`.
        // for other types that may not have a corresponding type in pkpy, it returns a check
        // function. e.g., `iterable` -> `[](handle h){ return hasattr(h, "iter"); }`.
        auto type_or_check = T::type_or_check();
        if constexpr(is_check_v<T>) {
            return type_or_check(obj);
        } else {
            return pkbind::isinstance(obj, type(type_or_check));
        }
    } else {
        return pkbind::isinstance(obj, of<T>());
    }
}

inline bool issubclass(type derived, type base) {
    return py_issubclass(derived.index(), base.index());
}

template <typename T>
inline bool isinstance(handle obj) {
    return type::isinstance<T>(obj);
}

template <>
inline bool isinstance<handle>(handle) = delete;

template <typename T, typename SFINAE = void>
struct type_caster;

template <typename T>
object cast(T&& value,
            return_value_policy policy = return_value_policy::automatic_reference,
            handle parent = {}) {
    // decay_t can resolve c-array type, but remove_cv_ref_t can't.
    using underlying_type = std::decay_t<T>;

    if constexpr(std::is_convertible_v<underlying_type, handle> &&
                 !is_pyobject_v<underlying_type>) {
        return object(std::forward<T>(value), object::realloc_t{});
    } else if constexpr(is_unique_pointer_v<underlying_type>) {
        using pointer = typename underlying_type::pointer;
        return type_caster<pointer>::cast(value.release(),
                                          return_value_policy::take_ownership,
                                          parent);
    } else {
        static_assert(!is_multiple_pointer_v<underlying_type>,
                      "multiple pointer is not supported.");
        static_assert(!std::is_void_v<std::remove_pointer_t<underlying_type>>,
                      "void* is not supported, consider using py::capsule.");

        // resolve for automatic policy.
        if(policy == return_value_policy::automatic) {
            policy = std::is_pointer_v<underlying_type> ? return_value_policy::take_ownership
                     : std::is_lvalue_reference_v<T&&>  ? return_value_policy::copy
                                                        : return_value_policy::move;
        } else if(policy == return_value_policy::automatic_reference) {
            policy = std::is_pointer_v<underlying_type> ? return_value_policy::reference
                     : std::is_lvalue_reference_v<T&&>  ? return_value_policy::copy
                                                        : return_value_policy::move;
        }

        return type_caster<underlying_type>::cast(std::forward<T>(value), policy, parent);
    }
}

template <typename T>
T cast(handle obj, bool convert = true) {
    using caster_t = type_caster<T>;
    constexpr auto is_dangling_v =
        (std::is_reference_v<T> || is_pointer_v<T>) && caster_t::is_temporary_v;
    static_assert(!is_dangling_v, "dangling reference or pointer is not allowed.");
    assert(obj.ptr() != nullptr);

    caster_t caster;
    if(caster.load(obj, convert)) {
        if constexpr(std::is_reference_v<T>) {
            return caster.value();
        } else {
            return std::move(caster.value());
        }
    } else {
        std::string msg = "cast python instance to c++ failed, obj type is: {";
        msg += type::of(obj).name();
        msg += "}, target type is: {";
        msg += type_name<T>();
        msg += "}.";
        throw cast_error(msg);
    }
}

template <typename Derived>
template <typename T>
T interface<Derived>::cast() const {
    return pkbind::cast<T>(handle(this->ptr()), true);
}

}  // namespace pkbind
