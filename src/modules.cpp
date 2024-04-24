#include "pocketpy/modules.h"

namespace pkpy{

void add_module_operator(VM* vm){
    PyObject* mod = vm->new_module("operator");
    vm->bind_func<2>(mod, "lt", [](VM* vm, ArgsView args) { return VAR(vm->py_lt(args[0], args[1]));});
    vm->bind_func<2>(mod, "le", [](VM* vm, ArgsView args) { return VAR(vm->py_le(args[0], args[1]));});
    vm->bind_func<2>(mod, "eq", [](VM* vm, ArgsView args) { return VAR(vm->py_eq(args[0], args[1]));});
    vm->bind_func<2>(mod, "ne", [](VM* vm, ArgsView args) { return VAR(vm->py_ne(args[0], args[1]));});
    vm->bind_func<2>(mod, "ge", [](VM* vm, ArgsView args) { return VAR(vm->py_ge(args[0], args[1]));});
    vm->bind_func<2>(mod, "gt", [](VM* vm, ArgsView args) { return VAR(vm->py_gt(args[0], args[1]));});
}

struct PyStructTime{
    PY_CLASS(PyStructTime, time, struct_time)

    int tm_year;
    int tm_mon;
    int tm_mday;
    int tm_hour;
    int tm_min;
    int tm_sec;
    int tm_wday;
    int tm_yday;
    int tm_isdst;

    PyStructTime(std::time_t t){
        std::tm* tm = std::localtime(&t);
        tm_year = tm->tm_year + 1900;
        tm_mon = tm->tm_mon + 1;
        tm_mday = tm->tm_mday;
        tm_hour = tm->tm_hour;
        tm_min = tm->tm_min;
        tm_sec = tm->tm_sec;
        tm_wday = (tm->tm_wday + 6) % 7;
        tm_yday = tm->tm_yday + 1;
        tm_isdst = tm->tm_isdst;
    }

    PyStructTime* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<PyStructTime>(type);
        PY_READONLY_FIELD(PyStructTime, "tm_year", _, tm_year);
        PY_READONLY_FIELD(PyStructTime, "tm_mon", _, tm_mon);
        PY_READONLY_FIELD(PyStructTime, "tm_mday", _, tm_mday);
        PY_READONLY_FIELD(PyStructTime, "tm_hour", _, tm_hour);
        PY_READONLY_FIELD(PyStructTime, "tm_min", _, tm_min);
        PY_READONLY_FIELD(PyStructTime, "tm_sec", _, tm_sec);
        PY_READONLY_FIELD(PyStructTime, "tm_wday", _, tm_wday);
        PY_READONLY_FIELD(PyStructTime, "tm_yday", _, tm_yday);
        PY_READONLY_FIELD(PyStructTime, "tm_isdst", _, tm_isdst);
    }
};

void add_module_time(VM* vm){
    PyObject* mod = vm->new_module("time");
    PyStructTime::register_class(vm, mod);

    vm->bind_func<0>(mod, "time", [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        return VAR(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 1000.0);
    });

    vm->bind_func<1>(mod, "sleep", [](VM* vm, ArgsView args) {
        f64 seconds = CAST_F(args[0]);
        auto begin = std::chrono::system_clock::now();
        while(true){
            auto now = std::chrono::system_clock::now();
            f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() / 1000.0;
            if(elapsed >= seconds) break;
        }
        return vm->None;
    });

    vm->bind_func<0>(mod, "localtime", [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        return VAR_T(PyStructTime, t);
    });
}

void add_module_sys(VM* vm){
    PyObject* mod = vm->new_module("sys");
    vm->setattr(mod, "version", VAR(PK_VERSION));
    vm->setattr(mod, "platform", VAR(kPlatformStrings[PK_SYS_PLATFORM]));

    PyObject* stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    PyObject* stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);

    vm->bind_func<1>(stdout_, "write", [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->stdout_write(s);
        return vm->None;
    });

    vm->bind_func<1>(stderr_, "write", [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->_stderr(s.data, s.size);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyObject* mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
        std::string_view sv;
        if(is_type(args[0], vm->tp_bytes)){
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        }else{
            sv = CAST(Str&, args[0]).sv();
        }
        CodeObject_ code = vm->compile(sv, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
    });
}

// https://docs.python.org/3.5/library/math.html
void add_module_math(VM* vm){
    PyObject* mod = vm->new_module("math");
    mod->attr().set("pi", VAR(3.1415926535897932384));
    mod->attr().set("e" , VAR(2.7182818284590452354));
    mod->attr().set("inf", VAR(std::numeric_limits<double>::infinity()));
    mod->attr().set("nan", VAR(std::numeric_limits<double>::quiet_NaN()));

    vm->bind_func<1>(mod, "ceil", PK_LAMBDA(VAR((i64)std::ceil(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fabs", PK_LAMBDA(VAR(std::fabs(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "floor", PK_LAMBDA(VAR((i64)std::floor(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fsum", [](VM* vm, ArgsView args) {
        List& list = CAST(List&, args[0]);
        double sum = 0;
        double c = 0;
        for(PyObject* arg : list){
            double x = CAST_F(arg);
            double y = x - c;
            double t = sum + y;
            c = (t - sum) - y;
            sum = t;
        }
        return VAR(sum);
    });
    vm->bind_func<2>(mod, "gcd", [](VM* vm, ArgsView args) {
        i64 a = CAST(i64, args[0]);
        i64 b = CAST(i64, args[1]);
        if(a < 0) a = -a;
        if(b < 0) b = -b;
        while(b != 0){
            i64 t = b;
            b = a % b;
            a = t;
        }
        return VAR(a);
    });

    vm->bind_func<1>(mod, "isfinite", PK_LAMBDA(VAR(std::isfinite(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isinf", PK_LAMBDA(VAR(std::isinf(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isnan", PK_LAMBDA(VAR(std::isnan(CAST_F(args[0])))));

    vm->bind_func<2>(mod, "isclose", [](VM* vm, ArgsView args) {
        f64 a = CAST_F(args[0]);
        f64 b = CAST_F(args[1]);
        return VAR(std::fabs(a - b) < 1e-9);
    });

    vm->bind_func<1>(mod, "exp", PK_LAMBDA(VAR(std::exp(CAST_F(args[0])))));
    // vm->bind_func<1>(mod, "log", PK_LAMBDA(VAR(std::log(CAST_F(args[0])))));

    vm->bind(mod, "log(x, base=2.718281828459045)", [](VM* vm, ArgsView args){
        f64 x = CAST_F(args[0]);
        f64 base = CAST_F(args[1]);
        return VAR(std::log(x) / std::log(base));
    });

    vm->bind_func<1>(mod, "log2", PK_LAMBDA(VAR(std::log2(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log10", PK_LAMBDA(VAR(std::log10(CAST_F(args[0])))));

    vm->bind_func<2>(mod, "pow", PK_LAMBDA(VAR(std::pow(CAST_F(args[0]), CAST_F(args[1])))));
    vm->bind_func<1>(mod, "sqrt", PK_LAMBDA(VAR(std::sqrt(CAST_F(args[0])))));

    vm->bind_func<1>(mod, "acos", PK_LAMBDA(VAR(std::acos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "asin", PK_LAMBDA(VAR(std::asin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "atan", PK_LAMBDA(VAR(std::atan(CAST_F(args[0])))));
    vm->bind_func<2>(mod, "atan2", PK_LAMBDA(VAR(std::atan2(CAST_F(args[0]), CAST_F(args[1])))));

    vm->bind_func<1>(mod, "cos", PK_LAMBDA(VAR(std::cos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "sin", PK_LAMBDA(VAR(std::sin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "tan", PK_LAMBDA(VAR(std::tan(CAST_F(args[0])))));
    
    vm->bind_func<1>(mod, "degrees", PK_LAMBDA(VAR(CAST_F(args[0]) * 180 / 3.1415926535897932384)));
    vm->bind_func<1>(mod, "radians", PK_LAMBDA(VAR(CAST_F(args[0]) * 3.1415926535897932384 / 180)));

    vm->bind_func<1>(mod, "modf", [](VM* vm, ArgsView args) {
        f64 i;
        f64 f = std::modf(CAST_F(args[0]), &i);
        return VAR(Tuple(VAR(f), VAR(i)));
    });

    vm->bind_func<1>(mod, "factorial", [](VM* vm, ArgsView args) {
        i64 n = CAST(i64, args[0]);
        if(n < 0) vm->ValueError("factorial() not defined for negative values");
        i64 r = 1;
        for(i64 i=2; i<=n; i++) r *= i;
        return VAR(r);
    });
}

void add_module_traceback(VM* vm){
    PyObject* mod = vm->new_module("traceback");
    vm->bind_func<0>(mod, "print_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = _CAST(Exception&, vm->_last_exception);
        vm->stdout_write(e.summary());
        return vm->None;
    });

    vm->bind_func<0>(mod, "format_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = _CAST(Exception&, vm->_last_exception);
        return VAR(e.summary());
    });
}

void add_module_dis(VM* vm){
    PyObject* mod = vm->new_module("dis");

    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
        CodeObject_ code;
        PyObject* obj = args[0];
        if(is_type(obj, vm->tp_str)){
            const Str& source = CAST(Str, obj);
            code = vm->compile(source, "<dis>", EXEC_MODE);
        }
        PyObject* f = obj;
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, obj).func;
        code = CAST(Function&, f).decl->code;
        vm->stdout_write(vm->disassemble(code));
        return vm->None;
    });
}

void add_module_gc(VM* vm){
    PyObject* mod = vm->new_module("gc");
    vm->bind_func<0>(mod, "collect", PK_LAMBDA(VAR(vm->heap.collect())));
}

struct LineProfilerW;
struct _LpGuard{
    PK_ALWAYS_PASS_BY_POINTER(_LpGuard)
    LineProfilerW* lp;
    VM* vm;
    _LpGuard(LineProfilerW* lp, VM* vm);
    ~_LpGuard();
};

// line_profiler wrapper
struct LineProfilerW{
    PY_CLASS(LineProfilerW, line_profiler, LineProfiler)

    LineProfiler profiler;

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_func<1>(type, __new__, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<LineProfilerW>(cls);
        });

        vm->bind(type, "add_function(self, func)", [](VM* vm, ArgsView args){
            LineProfilerW& self = PK_OBJ_GET(LineProfilerW, args[0]);
            vm->check_type(args[1], VM::tp_function);
            auto decl = PK_OBJ_GET(Function, args[1]).decl.get();
            self.profiler.functions.insert(decl);
            return vm->None;
        });

        vm->bind(type, "runcall(self, func, *args)", [](VM* vm, ArgsView view){
            LineProfilerW& self = PK_OBJ_GET(LineProfilerW, view[0]);
            PyObject* func = view[1];
            const Tuple& args = CAST(Tuple&, view[2]);
            vm->s_data.push(func);
            vm->s_data.push(PY_NULL);
            for(PyObject* arg : args) vm->s_data.push(arg);
            _LpGuard guard(&self, vm);
            PyObject* ret = vm->vectorcall(args.size());
            return ret;
        });

        vm->bind(type, "print_stats(self)", [](VM* vm, ArgsView args){
            LineProfilerW& self = PK_OBJ_GET(LineProfilerW, args[0]);
            vm->stdout_write(self.profiler.stats());
            return vm->None;
        });
    }
};


_LpGuard::_LpGuard(LineProfilerW* lp, VM* vm): lp(lp), vm(vm) {
    if(vm->_profiler){
        vm->ValueError("only one profiler can be enabled at a time");
    }
    vm->_profiler = &lp->profiler;
    lp->profiler.begin();
}

_LpGuard::~_LpGuard(){
    vm->_profiler = nullptr;
    lp->profiler.end();
}

void add_module_line_profiler(VM *vm){
    PyObject* mod = vm->new_module("line_profiler");
    LineProfilerW::register_class(vm, mod);
}


void add_module_enum(VM* vm){
    PyObject* mod = vm->new_module("enum");
    CodeObject_ code = vm->compile(kPythonLibs__enum, "enum.py", EXEC_MODE);
    vm->_exec(code, mod);
    PyObject* Enum = mod->attr("Enum");
    vm->_all_types[PK_OBJ_GET(Type, Enum).index].on_end_subclass = \
        [](VM* vm, PyTypeInfo* new_ti){
            new_ti->subclass_enabled = false;    // Enum class cannot be subclassed twice
            NameDict& attr = new_ti->obj->attr();
            for(auto [k, v]: attr.items()){
                // wrap every attribute
                std::string_view k_sv = k.sv();
                if(k_sv.empty() || k_sv[0] == '_') continue;
                attr.set(k, vm->call(new_ti->obj, VAR(k_sv), v));
            }
        };
}

void add_module___builtins(VM* vm){
    PyObject* mod = vm->new_module("__builtins");

    vm->bind_func<1>(mod, "next", [](VM* vm, ArgsView args){
        return vm->py_next(args[0]);
    });

    vm->bind_func<1>(mod, "_enable_instance_dict", [](VM* vm, ArgsView args){
        PyObject* self = args[0];
        if(is_tagged(self)) vm->TypeError("object: tagged object cannot enable instance dict");
        if(self->is_attr_valid()) vm->RuntimeError("object: instance dict is already enabled");
        self->_enable_instance_dict();
        return vm->None;
    });
}

}   // namespace pkpy