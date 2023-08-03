#include "pocketpy/re.h"

namespace pkpy{

struct ReMatch {
    PY_CLASS(ReMatch, re, Match)

    i64 start;
    i64 end;
    std::cmatch m;
    ReMatch(i64 start, i64 end, std::cmatch m) : start(start), end(end), m(m) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<ReMatch>(type);
        vm->bind_method<0>(type, "start", PK_LAMBDA(VAR(_CAST(ReMatch&, args[0]).start)));
        vm->bind_method<0>(type, "end", PK_LAMBDA(VAR(_CAST(ReMatch&, args[0]).end)));

        vm->bind_method<0>(type, "span", [](VM* vm, ArgsView args) {
            auto& self = _CAST(ReMatch&, args[0]);
            return VAR(Tuple({VAR(self.start), VAR(self.end)}));
        });

        vm->bind_method<1>(type, "group", [](VM* vm, ArgsView args) {
            auto& self = _CAST(ReMatch&, args[0]);
            int index = CAST(int, args[1]);
            index = vm->normalized_index(index, self.m.size());
            return VAR(self.m[index].str());
        });
    }
};

static PyObject* _regex_search(const Str& pattern, const Str& string, bool from_start, VM* vm){
    std::regex re(pattern.begin(), pattern.end());
    std::cmatch m;
    if(std::regex_search(string.begin(), string.end(), m, re)){
        if(from_start && m.position() != 0) return vm->None;
        i64 start = string._byte_index_to_unicode(m.position());
        i64 end = string._byte_index_to_unicode(m.position() + m.length());
        return VAR_T(ReMatch, start, end, m);
    }
    return vm->None;
};

void add_module_re(VM* vm){
    PyObject* mod = vm->new_module("re");
    ReMatch::register_class(vm, mod);

    vm->bind_func<2>(mod, "match", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, true, vm);
    });

    vm->bind_func<2>(mod, "search", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, false, vm);
    });

    vm->bind_func<3>(mod, "sub", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& repl = CAST(Str&, args[1]);
        const Str& string = CAST(Str&, args[2]);
        std::regex re(pattern.begin(), pattern.end());
        return VAR(std::regex_replace(string.str(), re, repl.str()));
    });

    vm->bind_func<2>(mod, "split", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        std::regex re(pattern.begin(), pattern.end());
        std::cregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::cregex_token_iterator end;
        List vec;
        for(; it != end; ++it){
            vec.push_back(VAR(it->str()));
        }
        return VAR(vec);
    });
}

}   // namespace pkpy