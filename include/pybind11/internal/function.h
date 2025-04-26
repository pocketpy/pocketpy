#pragma once

#include "cast.h"

namespace pkbind {

namespace impl {

template <typename... Args>
struct constructor {};

template <typename Fn, typename Args = callable_args_t<Fn>>
struct factory;

template <typename Fn, typename... Args>
struct factory<Fn, std::tuple<Args...>> {
    Fn fn;

    auto make() {
        using Self = callable_return_t<Fn>;
        return [fn = std::move(fn)](Self* self, Args... args) {
            new (self) Self(fn(args...));
        };
    }
};

}  // namespace impl

template <typename... Args>
impl::constructor<Args...> init() {
    return {};
}

template <typename Fn>
impl::factory<Fn> init(Fn&& fn) {
    return {std::forward<Fn>(fn)};
}

struct arg_with_default {
    const char* name;
    object value;
};

struct arg {
    const char* name;

    arg(const char* name) : name(name) {}

    template <typename T>
    arg_with_default operator= (T&& value) {
        return arg_with_default{name, cast(std::forward<T>(value))};
    }
};

struct kwargs_proxy {
    handle value;
};

struct args_proxy {
    handle value;

    kwargs_proxy operator* () { return kwargs_proxy{value}; }
};

template <typename Derived>
args_proxy interface<Derived>::operator* () const {
    return args_proxy{handle(this->ptr())};
}

template <typename Derived>
template <return_value_policy policy, typename... Args>
object interface<Derived>::operator() (Args&&... args) const {
    py_push(ptr());
    py_pushnil();

    int argc = 0;
    int kwargsc = 0;

    auto foreach = [&](auto&& argument) {
        using type = std::decay_t<decltype(argument)>;
        if constexpr(std::is_constructible_v<handle, type>) {
            argc += 1;
            py_push(handle(argument).ptr());
        } else if constexpr(std::is_same_v<type, arg_with_default>) {
            kwargsc += 1;
            arg_with_default& default_ = argument;
            py_pushname(name(default_.name).index());
            py_push(default_.value.ptr());
        } else if constexpr(std::is_same_v<type, args_proxy>) {
            tuple args = argument.value.template cast<tuple>();
            for(auto arg: args) {
                argc += 1;
                py_push(arg.ptr());
            }
        } else if constexpr(std::is_same_v<type, kwargs_proxy>) {
            dict kwargs = argument.value.template cast<dict>();
            kwargs.apply([&](handle key, handle value) {
                kwargsc += 1;
                name name = key.cast<std::string_view>();
                py_pushname(name.index());
                py_push(value.ptr());
            });
        } else {
            argc += 1;
            py_push(pkbind::cast(std::forward<decltype(argument)>(argument), policy).ptr());
        }
    };

    (foreach(std::forward<Args>(args)), ...);

    raise_call<py_vectorcall>(argc, kwargsc);

    return object::from_ret();
}

class function : public object {
    PKBIND_TYPE_IMPL(object, function, tp_function);
};

namespace impl {

template <typename Callable,
          typename Extra,
          typename Args = callable_args_t<Callable>,
          typename IndexSequence = std::make_index_sequence<std::tuple_size_v<Args>>>
struct template_parser;

class function_record {
    template <typename C, typename E, typename A, typename I>
    friend struct template_parser;

    using destructor_t = void (*)(function_record*);
    using wrapper_t = bool (*)(function_record&,
                               std::vector<handle>&,
                               std::vector<std::pair<handle, handle>>&,
                               bool convert,
                               handle parent);

    struct arguments_t {
        std::vector<std::string> names;
        std::vector<object> defaults;
    };

public:
    template <typename Fn, typename... Extras>
    function_record(Fn&& f, const Extras&... extras) {
        using Callable = std::decay_t<Fn>;

        if constexpr(std::is_trivially_copyable_v<Callable> && sizeof(Callable) <= sizeof(buffer)) {
            // if the callable object is trivially copyable and the size is less than 16 bytes,
            // store it in the buffer
            new (buffer) auto(std::forward<Fn>(f));
            destructor = [](function_record* self) {
                reinterpret_cast<Callable*>(self->buffer)->~Callable();
            };
        } else {
            // otherwise, store it in the heap
            data = new auto(std::forward<Fn>(f));
            destructor = [](function_record* self) {
                delete static_cast<Callable*>(self->data);
            };
        }

        using Parser = template_parser<Callable, std::tuple<Extras...>>;
        Parser::initialize(*this, extras...);
        wrapper = Parser::call;
    }

    function_record(const function_record&) = delete;

    function_record& operator= (const function_record&) = delete;

    function_record(function_record&& other) noexcept {
        std::memcpy(this, &other, sizeof(function_record));
        std::memset(&other, 0, sizeof(function_record));
    }

    function_record& operator= (function_record&&) = delete;

    ~function_record() {
        if(destructor) { destructor(this); }
        if(arguments) { delete arguments; }
        if(next) { delete next; }
        if(signature) { delete[] signature; }
    }

    void append(function_record* record) {
        function_record* p = this;
        while(p->next) {
            p = p->next;
        }
        p->next = record;
    }

    template <typename T>
    T& as() {
        if constexpr(std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(buffer)) {
            return *reinterpret_cast<T*>(buffer);
        } else {
            return *static_cast<T*>(data);
        }
    }

    static function_record& from(handle h) {
        auto slot = py_getslot(h.ptr(), 0);
        return *static_cast<function_record*>(py_touserdata(slot));
    }

    void operator() (int argc, handle stack) {
        function_record* p = this;

        bool has_self = argc == 3;
        std::vector<handle> args;
        handle self = py_offset(stack.ptr(), 0);
        if(has_self) { args.push_back(self); }

        auto tuple = py_offset(stack.ptr(), 0 + has_self);
        for(int i = 0; i < py_tuple_len(tuple); ++i) {
            args.push_back(py_tuple_getitem(tuple, i));
        }

        auto dict = steal<pkbind::dict>(py_offset(stack.ptr(), 1 + has_self));

        std::vector<std::pair<handle, handle>> kwargs;
        dict.apply([&](handle key, handle value) {
            kwargs.emplace_back(key, value);
        });

        // foreach function record and call the function with not convert
        while(p != nullptr) {
            auto result = p->wrapper(*p, args, kwargs, false, self);
            if(result) { return; }
            p = p->next;
        }

        p = this;
        // foreach function record and call the function with convert
        while(p != nullptr) {
            auto result = p->wrapper(*p, args, kwargs, true, self);
            if(result) { return; }
            p = p->next;
        }

        std::string msg = "no matching function found, function signature:\n";
        p = this;
        while(p != nullptr) {
            msg += "    ";
            msg += p->signature;
            msg += "\n";
            p = p->next;
        }
        throw std::runtime_error(msg);
    }

private:
    union {
        void* data;
        char buffer[16];
    };

    wrapper_t wrapper = nullptr;
    function_record* next = nullptr;
    arguments_t* arguments = nullptr;
    destructor_t destructor = nullptr;
    const char* signature = nullptr;
    return_value_policy policy = return_value_policy::automatic;
};

template <typename Fn, std::size_t... Is, typename... Args>
void invoke(Fn&& fn,
            std::index_sequence<Is...>,
            std::tuple<type_caster<Args>...>& casters,
            return_value_policy policy,
            handle parent) {
    using underlying_type = std::decay_t<Fn>;
    using return_type = callable_return_t<underlying_type>;

    constexpr bool is_void = std::is_void_v<return_type>;
    constexpr bool is_member_function_pointer = std::is_member_function_pointer_v<underlying_type>;

    if constexpr(is_member_function_pointer) {
        // helper function to unpack the arguments to call the member pointer
        auto unpack = [&](class_type_t<underlying_type>& self, auto&&... args) {
            return (self.*fn)(args...);
        };

        if constexpr(!is_void) {
            py_assign(py_retval(),
                      pkbind::cast(unpack(std::get<Is>(casters).value()...), policy, parent).ptr());
        } else {
            unpack(std::get<Is>(casters).value()...);
            py_newnone(py_retval());
        }
    } else {
        if constexpr(!is_void) {
            py_assign(py_retval(),
                      pkbind::cast(fn(std::get<Is>(casters).value()...), policy, parent).ptr());
        } else {
            fn(std::get<Is>(casters).value()...);
            py_newnone(py_retval());
        }
    }
}

template <typename Callable, typename... Extras, typename... Args, std::size_t... Is>
struct template_parser<Callable,
                       std::tuple<Extras...>,
                       std::tuple<Args...>,
                       std::index_sequence<Is...>> {
    using types = type_list<Args...>;

    /// count of the Callable parameters.
    constexpr inline static auto argc = types::size;

    // count the number of py::args and py::kwargs
    constexpr inline static auto args_count = types::template count<pkbind::args>;
    constexpr inline static auto kwargs_count = types::template count<pkbind::kwargs>;
    static_assert(args_count <= 1, "py::args can occur at most once");
    static_assert(kwargs_count <= 1, "py::kwargs can occur at most once");

    /// find the position of py::args and py::kwargs
    constexpr inline static auto args_pos = types::template find<pkbind::args>;
    constexpr inline static auto kwargs_pos = types::template find<pkbind::kwargs>;

    // FIXME: temporarily, args and kwargs must be at the end of the arguments list
    /// if have py::kwargs, it must be at the end of the arguments list.
    static_assert(kwargs_count == 0 || kwargs_pos == argc - 1,
                  "py::kwargs must be the last parameter");
    /// if have py::args, it must be before py::kwargs or at the end of the arguments list.
    static_assert(args_count == 0 || args_pos == kwargs_pos - 1 || args_pos == argc - 1,
                  "py::args must be before py::kwargs or at the end of the parameter list");

    using extras = type_list<Extras...>;

    // count the number of py::doc and py::return_value_policy
    constexpr inline static auto doc_count = extras::template count<const char*>;
    constexpr inline static auto policy_count = extras::template count<pkbind::return_value_policy>;
    static_assert(doc_count <= 1, "doc can occur at most once");
    static_assert(policy_count <= 1, "return_value_policy can occur at most once");

    constexpr inline static auto policy_pos = extras::template find<pkbind::return_value_policy>;

    constexpr inline static auto last_arg_without_default_pos =
        types::template find_last<pkbind::arg>;
    constexpr inline static auto first_arg_with_default_pos =
        types::template find<pkbind::arg_with_default>;
    static_assert(last_arg_without_default_pos < first_arg_with_default_pos ||
                      first_arg_with_default_pos == -1,
                  "parameter with default value must be after parameter without default value");

    /// count of named parameters(explicit with name).
    constexpr inline static auto named_only_argc = extras::template count<pkbind::arg>;
    constexpr inline static auto named_default_argc =
        extras::template count<pkbind::arg_with_default>;
    constexpr inline static auto named_argc = named_only_argc + named_default_argc;

    /// count of normal parameters(which are not py::args or py::kwargs).
    constexpr inline static auto normal_argc = argc - (args_pos != -1) - (kwargs_pos != -1);

    /// all parameters must either have no names or all must have names.
    static_assert(named_argc == 0 || named_argc == normal_argc,
                  "all parameters must either have no names or all must have names.");

    static void initialize(function_record& record, const Extras&... extras) {
        auto extras_tuple = std::make_tuple(extras...);
        constexpr static bool has_named_args = (named_argc > 0);
        if constexpr(policy_pos != -1) { record.policy = std::get<policy_pos>(extras_tuple); }

        // TODO: set others

        // set default arguments
        if constexpr(has_named_args) {
            record.arguments = new function_record::arguments_t();

            auto add_arguments = [&](const auto& default_) {
                using type = remove_cvref_t<decltype(default_)>;
                if constexpr(std::is_same_v<arg, type>) {
                    auto& arguments = *record.arguments;
                    arguments.names.emplace_back(default_.name);
                    arguments.defaults.emplace_back();
                } else if constexpr(std::is_same_v<arg_with_default, type>) {
                    auto& arguments = *record.arguments;
                    arguments.names.emplace_back(default_.name);
                    arguments.defaults.emplace_back(std::move(default_.value));
                }
            };

            (add_arguments(extras), ...);
        }

        // set signature
        {
            std::string sig = "(";
            std::size_t index = 0;
            auto append = [&](auto _t) {
                using T = remove_cvref_t<typename decltype(_t)::type>;
                if constexpr(std::is_same_v<T, args>) {
                    sig += "*args";
                } else if constexpr(std::is_same_v<T, kwargs>) {
                    sig += "**kwargs";
                } else if constexpr(has_named_args) {
                    sig += record.arguments->names[index].c_str();
                    sig += ": ";
                    sig += type_info::of<T>().name;
                    if(!record.arguments->defaults[index].empty()) {
                        sig += " = ";
                        sig += record.arguments->defaults[index]
                                   .attr("__repr__")()
                                   .cast<std::string_view>();
                    }
                } else {
                    sig += "_: ";
                    sig += type_info::of<T>().name;
                }
                if(index + 1 < argc) { sig += ", "; }
                index++;
            };
            (append(type_identity<Args>{}), ...);
            sig += ")";
            char* buffer = new char[sig.size() + 1];
            std::memcpy(buffer, sig.data(), sig.size());
            buffer[sig.size()] = '\0';
            record.signature = buffer;
        }
    }

    /// try to call a C++ function(store in function_record) with the arguments which are from
    /// Python. if success, return true, otherwise return false.
    static bool call(function_record& record,
                     std::vector<handle>& args,
                     std::vector<std::pair<handle, handle>>& kwargs,
                     bool convert,
                     handle parent) {
        // first, we try to load arguments into the stack.
        // use argc + 1 to avoid compile error when argc is 0.
        handle stack[argc + 1] = {};

        // if have default arguments, load them
        if constexpr(named_default_argc > 0) {
            auto& defaults = record.arguments->defaults;
            for(std::size_t i = named_only_argc; i < named_argc; ++i) {
                stack[i] = defaults[i];
            }
        }

        // load arguments from call arguments
        if(args.size() > normal_argc) {
            if constexpr(args_pos == -1) { return false; }
        }

        for(std::size_t i = 0; i < std::min(normal_argc, (int)args.size()); ++i) {
            stack[i] = args[i];
        }

        object repack_args;
        // pack the args
        if constexpr(args_pos != -1) {
            const auto n =
                static_cast<int>(args.size() > normal_argc ? args.size() - normal_argc : 0);
            auto pack = tuple(n);
            for(int i = 0; i < n; ++i) {
                pack[i] = args[normal_argc + i];
            }
            repack_args = std::move(pack);
            stack[args_pos] = repack_args;
        }

        // pack the kwargs
        int index = 0;
        if constexpr(named_argc != 0) {
            int arg_index = 0;
            while(arg_index < named_argc && index < kwargs.size()) {
                const auto name = kwargs[index].first;
                const auto value = kwargs[index].second;
                if(name.cast<std::string_view>() == record.arguments->names[arg_index]) {
                    stack[arg_index] = value;
                    index += 1;
                }
                arg_index += 1;
            }
        }

        object repacked_kwargs;
        if constexpr(kwargs_pos != -1) {
            auto pack = dict();
            while(index < kwargs.size()) {
                pack[kwargs[index].first] = kwargs[index].second;
                index += 1;
            }
            repacked_kwargs = std::move(pack);
            stack[kwargs_pos] = repacked_kwargs;
        }

        // check if all the arguments are valid
        for(std::size_t i = 0; i < argc; ++i) {
            if(!stack[i]) { return false; }
        }

        // ok, all the arguments are valid, call the function
        std::tuple<type_caster<Args>...> casters;

        if(((std::get<Is>(casters).load(stack[Is], convert)) && ...)) {
            invoke(record.as<Callable>(),
                   std::index_sequence<Is...>{},
                   casters,
                   record.policy,
                   parent);
            return true;
        }

        return false;
    }
};

}  // namespace impl

class cpp_function : public function {
    PKBIND_TYPE_IMPL(function, cpp_function, tp_function);

    inline static lazy<py_Type> tp_function_record = +[](py_Type& type) {
        type = py_newtype("function_record", tp_object, nullptr, [](void* data) {
            static_cast<impl::function_record*>(data)->~function_record();
        });
    };

    static bool is_function_record(handle h) {
        if(isinstance<function>(h)) {
            auto slot = py_getslot(h.ptr(), 0);
            if(slot) { return py_typeof(slot) == tp_function_record; }
        }
        return false;
    }

    template <typename Fn, typename... Extras>
    cpp_function(bool is_method, const char* name, Fn&& fn, const Extras&... extras) :
        function(alloc_t{}) {
        // bind the function
        std::string sig = name;
        sig += is_method ? "(self, *args, **kwargs)" : "(*args, **kwargs)";

        py_newfunction(m_ptr, sig.c_str(), call, nullptr, 1);
        auto slot = py_getslot(m_ptr, 0);
        void* data = py_newobject(slot, tp_function_record, 0, sizeof(impl::function_record));
        new (data) impl::function_record(std::forward<Fn>(fn), extras...);
    }

private:
    static bool call(int argc, py_Ref stack) {
        handle func = py_inspect_currentfunction();
        auto data = py_touserdata(py_getslot(func.ptr(), 0));
        auto& record = *static_cast<impl::function_record*>(data);
        try {
            record(argc, stack);
            return true;
        } catch(std::domain_error& e) {
            py_exception(tp_ValueError, e.what());
        } catch(std::invalid_argument& e) {
            py_exception(tp_ValueError, e.what());
        } catch(std::length_error& e) {
            py_exception(tp_ValueError, e.what());
        } catch(std::out_of_range& e) {
            py_exception(tp_IndexError, e.what());
        } catch(std::range_error& e) {
            py_exception(tp_ValueError, e.what());
        } catch(stop_iteration& e) {
            if(auto value_ptr = e.value().ptr()) {
                bool ok = py_tpcall(tp_StopIteration, 1, value_ptr);
                if(ok) { py_raise(py_retval()); }
            } else {
                StopIteration();
            }
        } catch(index_error& e) {
            py_exception(tp_IndexError, e.what());
        } catch(key_error& e) { py_exception(tp_KeyError, e.what()); } catch(value_error& e) {
            py_exception(tp_ValueError, e.what());
        } catch(type_error& e) { py_exception(tp_TypeError, e.what()); } catch(import_error& e) {
            py_exception(tp_ImportError, e.what());
        } catch(error_already_set&) {
            // exception already set, do nothing
        } catch(attribute_error& e) {
            py_exception(tp_AttributeError, e.what());
        } catch(std::exception& e) { py_exception(tp_RuntimeError, e.what()); }
        return false;
    };
};

class property : public object {
    PKBIND_TYPE_IMPL(object, property, tp_property);

    property(handle getter, handle setter = none()) :
        object(type::of<property>()(getter, setter)) {}
};

class staticmethod : public cpp_function {
    PKBIND_TYPE_IMPL(cpp_function, staticmethod, tp_staticmethod);
};

namespace impl {

template <bool is_method, bool is_static, typename Fn, typename... Extras>
void bind_function(handle obj, const char* name_, Fn&& fn, const Extras&... extras) {
    constexpr bool has_named_args =
        ((std::is_same_v<Extras, arg> || std::is_same_v<Extras, arg_with_default>) || ...);
    auto name = py_name(name_);
    auto func = py_getdict(obj.ptr(), name);

    if(func && cpp_function::is_function_record(func)) {
        auto slot = py_getslot(func, 0);
        auto& record = *static_cast<function_record*>(py_touserdata(slot));
        if constexpr(has_named_args && is_method) {
            record.append(new function_record(std::forward<Fn>(fn), arg("self"), extras...));
        } else {
            record.append(new function_record(std::forward<Fn>(fn), extras...));
        }
    } else {
        if constexpr(is_static) {
            py_setdict(
                obj.ptr(),
                name,
                staticmethod(is_method, name_, std::forward<Fn>(fn), extras...).ptr());
        } else {
            if constexpr(has_named_args && is_method) {
                py_setdict(
                    obj.ptr(),
                    name,
                    cpp_function(is_method, name_, std::forward<Fn>(fn), arg("self"), extras...)
                        .ptr());
            } else {
                py_setdict(obj.ptr(),
                           name,
                           cpp_function(is_method, name_, std::forward<Fn>(fn), extras...).ptr());
            }
        }
    }
}

template <typename Getter, typename Setter, typename... Extras>
void bind_property(handle obj,
                   const char* name,
                   Getter&& getter_,
                   Setter&& setter_,
                   const Extras&... extras) {
    if constexpr(std::is_same_v<std::decay_t<Setter>, std::nullptr_t>) {
        cpp_function getter(true,
                            name,
                            std::forward<Getter>(getter_),
                            return_value_policy::reference_internal,
                            extras...);
        property prop(getter.ptr());
        setattr(obj, name, prop);
    } else {
        cpp_function getter(true,
                            name,
                            std::forward<Getter>(getter_),
                            return_value_policy::reference_internal,
                            extras...);
        cpp_function setter(true,
                            name,
                            std::forward<Setter>(setter_),
                            return_value_policy::reference_internal,
                            extras...);
        property prop(getter.ptr(), setter.ptr());
        setattr(obj, name, prop);
    }
}

}  // namespace impl

inline dict::dict(std::initializer_list<arg_with_default> args) : dict() {
    for(auto& arg: args) {
        this->operator[] (arg.name) = arg.value;
    }
}

}  // namespace pkbind
