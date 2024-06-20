#pragma once
#include "cast.h"
#include <map>

namespace pybind11 {

// append the overload to the beginning of the overload list
struct prepend {};

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

//  TODO: support more customized tags
//
// template <std::size_t Nurse, std::size_t... Patients>
// struct keep_alive {};
//
// template <typename T>
// struct call_guard {
//     static_assert(std::is_default_constructible_v<T>, "call_guard must be default constructible");
// };
//
//  struct kw_only {};
//
//  struct pos_only {};

class cpp_function : public function {
    PYBIND11_TYPE_IMPLEMENT(function, pkpy::NativeFunc, vm->tp_native_func);

public:
    template <typename Fn, typename... Extras>
    cpp_function(Fn&& f, const Extras&... extras) {}

    template <typename T>
    decltype(auto) get_userdata_as() {
#if PK_VERSION_MAJOR == 2
        return self()._userdata.as<T>();
#else
        return self()._userdata._cast<T>();
#endif
    }

    template <typename T>
    void set_userdata(T&& value) {
        self()._userdata = std::forward<T>(value);
    }
};

}  // namespace pybind11

namespace pybind11::impl {

template <typename Callable,
          typename Extra,
          typename Args = callable_args_t<Callable>,
          typename IndexSequence = std::make_index_sequence<std::tuple_size_v<Args>>>
struct template_parser;

class function_record {
private:
    template <typename C, typename E, typename A, typename I>
    friend struct template_parser;

    struct arguments_t {
        std::vector<pkpy::StrName> names;
        std::vector<handle> defaults;
    };

    using destructor_t = void (*)(function_record*);
    using wrapper_t = handle (*)(function_record&, pkpy::ArgsView, bool convert, handle parent);

    static_assert(std::is_trivially_copyable_v<pkpy::StrName>);

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

public:
    template <typename Fn, typename... Extras>
    function_record(Fn&& f, const Extras&... extras) {
        using Callable = std::decay_t<Fn>;

        if constexpr(std::is_trivially_copyable_v<Callable> && sizeof(Callable) <= sizeof(buffer)) {
            // if the callable object is trivially copyable and the size is less than 16 bytes, store it in the
            // buffer
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
        wrapper = Parser::wrapper;
    }

    function_record(const function_record&) = delete;
    function_record& operator= (const function_record&) = delete;
    function_record& operator= (function_record&&) = delete;

    function_record(function_record&& other) noexcept {
        std::memcpy(this, &other, sizeof(function_record));
        std::memset(&other, 0, sizeof(function_record));
    }

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
    T& _as() {
        if constexpr(std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(buffer)) {
            return *reinterpret_cast<T*>(buffer);
        } else {
            return *static_cast<T*>(data);
        }
    }

    handle operator() (pkpy::ArgsView view) {
        function_record* p = this;
        // foreach function record and call the function with not convert
        while(p != nullptr) {
            handle result = p->wrapper(*p, view, false, {});
            if(result) { return result; }
            p = p->next;
        }

        p = this;
        // foreach function record and call the function with convert
        while(p != nullptr) {
            handle result = p->wrapper(*p, view, true, {});
            if(result) { return result; }
            p = p->next;
        }

        std::string msg = "no matching function found, function signature:\n";
        std::size_t index = 0;
        p = this;
        while(p != nullptr) {
            msg += "    ";
            msg += p->signature;
            msg += "\n";
            p = p->next;
        }
        vm->TypeError(msg);
        PK_UNREACHABLE();
    }
};

template <typename Fn, std::size_t... Is, typename... Args>
handle invoke(Fn&& fn,
              std::index_sequence<Is...>,
              std::tuple<impl::type_caster<Args>...>& casters,
              return_value_policy policy,
              handle parent) {
    using underlying_type = std::decay_t<Fn>;
    using return_type = callable_return_t<underlying_type>;

    constexpr bool is_void = std::is_void_v<return_type>;
    constexpr bool is_member_function_pointer = std::is_member_function_pointer_v<underlying_type>;

    if constexpr(is_member_function_pointer) {
        // helper function to unpack the arguments to call the member pointer
        auto unpack = [&](class_type_t<underlying_type>& self, auto&... args) {
            return (self.*fn)(args...);
        };

        if constexpr(!is_void) {
            return pybind11::cast(unpack(std::get<Is>(casters).value...), policy, parent);
        } else {
            unpack(std::get<Is>(casters).value...);
            return vm->None;
        }
    } else {
        if constexpr(!is_void) {
            return pybind11::cast(fn(std::get<Is>(casters).value...), policy, parent);
        } else {
            fn(std::get<Is>(casters).value...);
            return vm->None;
        }
    }
}

struct arguments_info_t {
    int argc = 0;
    int args_pos = -1;
    int kwargs_pos = -1;
};

struct extras_info_t {
    int doc_pos = -1;
    int named_argc = 0;
    int policy_pos = -1;
};

template <typename Callable, typename... Extras, typename... Args, std::size_t... Is>
struct template_parser<Callable, std::tuple<Extras...>, std::tuple<Args...>, std::index_sequence<Is...>> {
    constexpr static arguments_info_t parse_arguments() {
        constexpr auto args_count = types_count_v<args, Args...>;
        constexpr auto kwargs_count = types_count_v<kwargs, Args...>;

        static_assert(args_count <= 1, "py::args can occur at most once");
        static_assert(kwargs_count <= 1, "py::kwargs can occur at most once");

        constexpr auto args_pos = type_index_v<args, Args...>;
        constexpr auto kwargs_pos = type_index_v<kwargs, Args...>;

        if constexpr(kwargs_count == 1) {
            static_assert(kwargs_pos == sizeof...(Args) - 1, "py::kwargs must be the last argument");

            // FIXME: temporarily, args and kwargs must be at the end of the arguments list
            if constexpr(args_count == 1) {
                static_assert(args_pos == kwargs_pos - 1, "py::args must be before py::kwargs");
            }
        }

        return {sizeof...(Args), args_pos, kwargs_pos};
    }

    constexpr static extras_info_t parse_extras() {
        constexpr auto doc_count = types_count_v<const char*, Extras...>;
        constexpr auto policy_count = types_count_v<return_value_policy, Extras...>;

        static_assert(doc_count <= 1, "doc can occur at most once");
        static_assert(policy_count <= 1, "return_value_policy can occur at most once");

        constexpr auto doc_pos = type_index_v<const char*, Extras...>;
        constexpr auto policy_pos = type_index_v<return_value_policy, Extras...>;

        constexpr auto named_argc = types_count_v<arg, Extras...>;
        constexpr auto normal_argc =
            sizeof...(Args) - (arguments_info.args_pos != -1) - (arguments_info.kwargs_pos != -1);

        static_assert(named_argc == 0 || named_argc == normal_argc,
                      "named arguments must be the same as the number of function arguments");

        return {doc_pos, named_argc, policy_pos};
    }

    constexpr inline static auto arguments_info = parse_arguments();
    constexpr inline static auto extras_info = parse_extras();

    static void initialize(function_record& record, const Extras&... extras) {
        auto extras_tuple = std::make_tuple(extras...);
        constexpr static bool has_named_args = (extras_info.named_argc > 0);
        // set return value policy
        if constexpr(extras_info.policy_pos != -1) { record.policy = std::get<extras_info.policy_pos>(extras_tuple); }

        // TODO: set others

        // set default arguments
        if constexpr(has_named_args) {
            record.arguments = new function_record::arguments_t();

            auto add_arguments = [&](const auto& arg) {
                if constexpr(std::is_same_v<pybind11::arg, remove_cvref_t<decltype(arg)>>) {
                    auto& arguments = *record.arguments;
                    arguments.names.emplace_back(arg.name);
                    arguments.defaults.emplace_back(arg.default_);
                }
            };

            (add_arguments(extras), ...);
        }

        // set signature
        {
            std::string sig = "(";
            std::size_t index = 0;
            auto append = [&](auto _t) {
                using T = pybind11_decay_t<typename decltype(_t)::type>;
                if constexpr(std::is_same_v<T, args>) {
                    sig += "*args";
                } else if constexpr(std::is_same_v<T, kwargs>) {
                    sig += "**kwargs";
                } else if constexpr(has_named_args) {
                    sig += record.arguments->names[index].c_str();
                    sig += ": ";
                    sig += type_info::of<T>().name;
                    if(record.arguments->defaults[index]) {
                        sig += " = ";
                        sig += record.arguments->defaults[index].repr();
                    }
                } else {
                    sig += "_: ";
                    sig += type_info::of<T>().name;
                }

                if(index + 1 < arguments_info.argc) { sig += ", "; }
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

    static handle wrapper(function_record& record, pkpy::ArgsView view, bool convert, handle parent) {
        constexpr auto argc = arguments_info.argc;
        constexpr auto named_argc = extras_info.named_argc;
        constexpr auto args_pos = arguments_info.args_pos;
        constexpr auto kwargs_pos = arguments_info.kwargs_pos;
        constexpr auto normal_argc = argc - (args_pos != -1) - (kwargs_pos != -1);

        // avoid gc call in bound function
        vm->heap.gc_scope_lock();

        // add 1 to avoid zero-size array when argc is 0
        handle stack[argc + 1] = {};

        // ensure the number of passed arguments is no greater than the number of parameters
        if(args_pos == -1 && view.size() > normal_argc) { return handle(); }

        // if have default arguments, load them
        if constexpr(named_argc > 0) {
            auto& defaults = record.arguments->defaults;
            std::memcpy(stack, defaults.data(), defaults.size() * sizeof(handle));
        }

        // load arguments from call arguments
        const auto size = std::min(view.size(), normal_argc);
        std::memcpy(stack, view.begin(), size * sizeof(handle));

        // pack the args
        if constexpr(args_pos != -1) {
            const auto n = std::max(view.size() - normal_argc, 0);
            tuple args = tuple(n);
            for(std::size_t i = 0; i < n; ++i) {
                args[i] = view[normal_argc + i];
            }
            stack[args_pos] = args;
        }

        // resolve keyword arguments
        const auto n = vm->s_data._sp - view.end();
        int index = 0;

        if constexpr(named_argc > 0) {
            int arg_index = 0;
            auto& arguments = *record.arguments;

            while(arg_index < named_argc && index < n) {
                const auto key = pkpy::_py_cast<pkpy::i64>(vm, view.end()[index]);
                const auto value = view.end()[index + 1];
                const auto name = pkpy::StrName(key);
                auto& arg_name = record.arguments->names[arg_index];

                if(name == arg_name) {
                    stack[arg_index] = value;
                    index += 2;
                }

                arg_index += 1;
            }
        }

        // pack the kwargs
        if constexpr(kwargs_pos != -1) {
            dict kwargs;
            while(index < n) {
                const auto key = pkpy::_py_cast<pkpy::i64>(vm, view.end()[index]);
                const str name = str(pkpy::StrName(key).sv());
                kwargs[name] = view.end()[index + 1];
                index += 2;
            }
            stack[kwargs_pos] = kwargs;
        }

        // if have rest keyword arguments, call fails
        if(index != n) { return handle(); }

        // check if all the arguments are valid
        for(std::size_t i = 0; i < argc; ++i) {
            if(!stack[i]) { return handle(); }
        }

        // ok, all the arguments are valid, call the function
        std::tuple<impl::type_caster<Args>...> casters;

        // check type compatibility
        if(((std::get<Is>(casters).load(stack[Is], convert)) && ...)) {
            return invoke(record._as<Callable>(), std::index_sequence<Is...>{}, casters, record.policy, parent);
        }

        return handle();
    }
};

inline auto _wrapper(pkpy::VM* vm, pkpy::ArgsView view) {
    auto&& record = unpack<function_record>(view);
    return record(view).ptr();
}

template <bool is_method, typename Fn, typename... Extras>
handle bind_function(const handle& obj, const char* name, Fn&& fn, pkpy::BindType type, const Extras&... extras) {
    // do not use cpp_function directly to avoid unnecessary reference count change
    pkpy::PyVar var = obj.ptr();
    cpp_function callable = var->attr().try_get(name);
    function_record* record = nullptr;

    if constexpr(is_method && types_count_v<arg, Extras...> > 0) {
        // if the function is a method and has named arguments
        // prepend self to the arguments list
        record = new function_record(std::forward<Fn>(fn), arg("self"), extras...);
    } else {
        record = new function_record(std::forward<Fn>(fn), extras...);
    }

    if(!callable) {
        // if the function is not bound yet, bind it
        void* data = interpreter::take_ownership(std::move(*record));
        callable = interpreter::bind_func(var, name, -1, _wrapper, data, type);
    } else {
        // if the function is already bound, append the new record to the function
        function_record* last = callable.get_userdata_as<function_record*>();

        if constexpr((types_count_v<prepend, Extras...> != 0)) {
            // if prepend is specified, append the new record to the beginning of the list
            callable.set_userdata(record);
            record->append(last);
        } else {
            // otherwise, append the new record to the end of the list
            last->append(record);
        }
    }

    return callable;
}

}  // namespace pybind11::impl

namespace pybind11::impl {

template <typename Getter>
pkpy::PyVar getter_wrapper(pkpy::VM* vm, pkpy::ArgsView view) {
    handle result = vm->None;
    auto&& getter = unpack<Getter>(view);
    constexpr auto policy = return_value_policy::reference_internal;

    if constexpr(std::is_member_pointer_v<Getter>) {
        using Self = class_type_t<Getter>;
        auto& self = handle(view[0])._as<instance>()._as<Self>();

        if constexpr(std::is_member_object_pointer_v<Getter>) {
            // specialize for pointer to data member
            result = cast(self.*getter, policy, view[0]);
        } else {
            // specialize for pointer to member function
            result = cast((self.*getter)(), policy, view[0]);
        }
    } else {
        // specialize for function pointer and lambda
        using Self = remove_cvref_t<std::tuple_element_t<0, callable_args_t<Getter>>>;
        auto& self = handle(view[0])._as<instance>()._as<Self>();

        result = cast(getter(self), policy, view[0]);
    }

    return result.ptr();
}

template <typename Setter>
pkpy::PyVar setter_wrapper(pkpy::VM* vm, pkpy::ArgsView view) {
    auto&& setter = unpack<Setter>(view);

    if constexpr(std::is_member_pointer_v<Setter>) {
        using Self = class_type_t<Setter>;
        auto& self = handle(view[0])._as<instance>()._as<Self>();

        if constexpr(std::is_member_object_pointer_v<Setter>) {
            // specialize for pointer to data member
            impl::type_caster<member_type_t<Setter>> caster;
            if(caster.load(view[1], true)) {
                self.*setter = caster.value;
                return vm->None;
            }
        } else {
            // specialize for pointer to member function
            impl::type_caster<std::tuple_element_t<1, callable_args_t<Setter>>> caster;
            if(caster.load(view[1], true)) {
                (self.*setter)(caster.value);
                return vm->None;
            }
        }
    } else {
        // specialize for function pointer and lambda
        using Self = remove_cvref_t<std::tuple_element_t<0, callable_args_t<Setter>>>;
        auto& self = handle(view[0])._as<instance>()._as<Self>();

        impl::type_caster<std::tuple_element_t<1, callable_args_t<Setter>>> caster;
        if(caster.load(view[1], true)) {
            setter(self, caster.value);
            return vm->None;
        }
    }

    vm->TypeError("Unexpected argument type");
    PK_UNREACHABLE();
}

template <typename Getter, typename Setter, typename... Extras>
handle bind_property(const handle& obj, const char* name, Getter&& getter_, Setter&& setter_, const Extras&... extras) {
    handle getter = none();
    handle setter = none();
    using Wrapper = pkpy::PyVar (*)(pkpy::VM*, pkpy::ArgsView);

    constexpr auto create = [](Wrapper wrapper, int argc, auto&& f) {
        if constexpr(need_host<remove_cvref_t<decltype(f)>>) {
            // otherwise, store it in the type_info
            void* data = interpreter::take_ownership(std::forward<decltype(f)>(f));
            // store the index in the object
            return vm->heap.gcnew<pkpy::NativeFunc>(vm->tp_native_func, wrapper, argc, data);
        } else {
            // if the function is trivially copyable and the size is less than 16 bytes, store it in the object
            // directly
            return vm->heap.gcnew<pkpy::NativeFunc>(vm->tp_native_func, wrapper, argc, f);
        }
    };

    getter = create(impl::getter_wrapper<std::decay_t<Getter>>, 1, std::forward<Getter>(getter_));

    if constexpr(!std::is_same_v<Setter, std::nullptr_t>) {
        setter = create(impl::setter_wrapper<std::decay_t<Setter>>, 2, std::forward<Setter>(setter_));
    }

    handle property = pybind11::property(getter, setter);
    setattr(obj, name, property);
    return property;
}

}  // namespace pybind11::impl
