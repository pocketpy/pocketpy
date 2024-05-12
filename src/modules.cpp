#include "pocketpy/modules.h"

namespace pkpy{

struct PyStructTime{
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

    static void _register(VM* vm, PyVar mod, PyVar type){
        PY_READONLY_FIELD(PyStructTime, "tm_year", tm_year);
        PY_READONLY_FIELD(PyStructTime, "tm_mon", tm_mon);
        PY_READONLY_FIELD(PyStructTime, "tm_mday", tm_mday);
        PY_READONLY_FIELD(PyStructTime, "tm_hour", tm_hour);
        PY_READONLY_FIELD(PyStructTime, "tm_min", tm_min);
        PY_READONLY_FIELD(PyStructTime, "tm_sec", tm_sec);
        PY_READONLY_FIELD(PyStructTime, "tm_wday", tm_wday);
        PY_READONLY_FIELD(PyStructTime, "tm_yday", tm_yday);
        PY_READONLY_FIELD(PyStructTime, "tm_isdst", tm_isdst);
    }
};

void add_module_time(VM* vm){
    PyVar mod = vm->new_module("time");
    vm->register_user_class<PyStructTime>(mod, "struct_time");

    vm->bind_func(mod, "time", 0, [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        return VAR(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 1000.0);
    });

    vm->bind_func(mod, "sleep", 1, [](VM* vm, ArgsView args) {
        f64 seconds = CAST_F(args[0]);
        auto begin = std::chrono::system_clock::now();
        while(true){
            auto now = std::chrono::system_clock::now();
            f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() / 1000.0;
            if(elapsed >= seconds) break;
        }
        return vm->None;
    });

    vm->bind_func(mod, "localtime", 0, [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        return vm->new_user_object<PyStructTime>(t);
    });
}

void add_module_sys(VM* vm){
    PyVar mod = vm->new_module("sys");
    vm->setattr(mod, "version", VAR(PK_VERSION));
    vm->setattr(mod, "platform", VAR(kPlatformStrings[PK_SYS_PLATFORM]));

    PyVar stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    PyVar stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);

    vm->bind_func(stdout_, "write", 1, [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->stdout_write(s);
        return vm->None;
    });

    vm->bind_func(stderr_, "write", 1, [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->_stderr(s.data, s.size);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyVar mod = vm->new_module("json");
    vm->bind_func(mod, "loads", 1, [](VM* vm, ArgsView args) {
        std::string_view sv;
        if(is_type(args[0], vm->tp_bytes)){
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        }else{
            sv = CAST(Str&, args[0]).sv();
        }
        CodeObject_ code = vm->compile(sv, "<json>", JSON_MODE);
        return vm->_exec(code, vm->callstack.top()._module);
    });

    vm->bind_func(mod, "dumps", 1, [](VM* vm, ArgsView args) {
        return VAR(vm->py_json(args[0]));
    });
}

// https://docs.python.org/3.5/library/math.html
void add_module_math(VM* vm){
    PyVar mod = vm->new_module("math");
    mod->attr().set("pi", VAR(3.1415926535897932384));
    mod->attr().set("e" , VAR(2.7182818284590452354));
    mod->attr().set("inf", VAR(std::numeric_limits<double>::infinity()));
    mod->attr().set("nan", VAR(std::numeric_limits<double>::quiet_NaN()));

    vm->bind_func(mod, "ceil", 1, PK_LAMBDA(VAR((i64)std::ceil(CAST_F(args[0])))));
    vm->bind_func(mod, "fabs", 1, PK_LAMBDA(VAR(std::fabs(CAST_F(args[0])))));
    vm->bind_func(mod, "floor", 1, PK_LAMBDA(VAR((i64)std::floor(CAST_F(args[0])))));
    vm->bind_func(mod, "fsum", 1, [](VM* vm, ArgsView args) {
        List& list = CAST(List&, args[0]);
        double sum = 0;
        double c = 0;
        for(PyVar arg : list){
            double x = CAST_F(arg);
            double y = x - c;
            double t = sum + y;
            c = (t - sum) - y;
            sum = t;
        }
        return VAR(sum);
    });
    vm->bind_func(mod, "gcd", 2, [](VM* vm, ArgsView args) {
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

    vm->bind_func(mod, "isfinite", 1, PK_LAMBDA(VAR(std::isfinite(CAST_F(args[0])))));
    vm->bind_func(mod, "isinf", 1, PK_LAMBDA(VAR(std::isinf(CAST_F(args[0])))));
    vm->bind_func(mod, "isnan", 1, PK_LAMBDA(VAR(std::isnan(CAST_F(args[0])))));

    vm->bind_func(mod, "isclose", 2, [](VM* vm, ArgsView args) {
        f64 a = CAST_F(args[0]);
        f64 b = CAST_F(args[1]);
        return VAR(std::fabs(a - b) < 1e-9);
    });

    vm->bind_func(mod, "exp", 1, PK_LAMBDA(VAR(std::exp(CAST_F(args[0])))));

    vm->bind(mod, "log(x, base=2.718281828459045)", [](VM* vm, ArgsView args){
        f64 x = CAST_F(args[0]);
        f64 base = CAST_F(args[1]);
        return VAR(std::log(x) / std::log(base));
    });

    vm->bind_func(mod, "log2", 1, PK_LAMBDA(VAR(std::log2(CAST_F(args[0])))));
    vm->bind_func(mod, "log10", 1, PK_LAMBDA(VAR(std::log10(CAST_F(args[0])))));

    vm->bind_func(mod, "pow", 2, PK_LAMBDA(VAR(std::pow(CAST_F(args[0]), CAST_F(args[1])))));
    vm->bind_func(mod, "sqrt", 1, PK_LAMBDA(VAR(std::sqrt(CAST_F(args[0])))));

    vm->bind_func(mod, "acos", 1, PK_LAMBDA(VAR(std::acos(CAST_F(args[0])))));
    vm->bind_func(mod, "asin", 1, PK_LAMBDA(VAR(std::asin(CAST_F(args[0])))));
    vm->bind_func(mod, "atan", 1, PK_LAMBDA(VAR(std::atan(CAST_F(args[0])))));
    vm->bind_func(mod, "atan2", 2, PK_LAMBDA(VAR(std::atan2(CAST_F(args[0]), CAST_F(args[1])))));

    vm->bind_func(mod, "cos", 1, PK_LAMBDA(VAR(std::cos(CAST_F(args[0])))));
    vm->bind_func(mod, "sin", 1, PK_LAMBDA(VAR(std::sin(CAST_F(args[0])))));
    vm->bind_func(mod, "tan", 1, PK_LAMBDA(VAR(std::tan(CAST_F(args[0])))));
    
    vm->bind_func(mod, "degrees", 1, PK_LAMBDA(VAR(CAST_F(args[0]) * 180 / 3.1415926535897932384)));
    vm->bind_func(mod, "radians", 1, PK_LAMBDA(VAR(CAST_F(args[0]) * 3.1415926535897932384 / 180)));

    vm->bind_func(mod, "modf", 1, [](VM* vm, ArgsView args) {
        f64 i;
        f64 f = std::modf(CAST_F(args[0]), &i);
        return VAR(Tuple(VAR(f), VAR(i)));
    });

    vm->bind_func(mod, "factorial", 1, [](VM* vm, ArgsView args) {
        i64 n = CAST(i64, args[0]);
        if(n < 0) vm->ValueError("factorial() not defined for negative values");
        i64 r = 1;
        for(i64 i=2; i<=n; i++) r *= i;
        return VAR(r);
    });
}

void add_module_traceback(VM* vm){
    PyVar mod = vm->new_module("traceback");
    vm->bind_func(mod, "print_exc", 0, [](VM* vm, ArgsView args) {
        if(vm->__last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = _CAST(Exception&, vm->__last_exception);
        vm->stdout_write(e.summary());
        return vm->None;
    });

    vm->bind_func(mod, "format_exc", 0, [](VM* vm, ArgsView args) {
        if(vm->__last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = _CAST(Exception&, vm->__last_exception);
        return VAR(e.summary());
    });
}

void add_module_dis(VM* vm){
    PyVar mod = vm->new_module("dis");

    vm->bind_func(mod, "dis", 1, [](VM* vm, ArgsView args) {
        CodeObject_ code;
        PyVar obj = args[0];
        if(is_type(obj, vm->tp_str)){
            const Str& source = CAST(Str, obj);
            code = vm->compile(source, "<dis>", EXEC_MODE);
        }
        PyVar f = obj;
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, obj).func;
        code = CAST(Function&, f).decl->code;
        vm->stdout_write(vm->disassemble(code));
        return vm->None;
    });
}

void add_module_gc(VM* vm){
    PyVar mod = vm->new_module("gc");
    vm->bind_func(mod, "collect", 0, PK_LAMBDA(VAR(vm->heap.collect())));
}

void add_module_enum(VM* vm){
    PyVar mod = vm->new_module("enum");
    CodeObject_ code = vm->compile(kPythonLibs__enum, "enum.py", EXEC_MODE);
    vm->_exec(code, mod);
    PyVar Enum = mod->attr("Enum");
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
    PyVar mod = vm->new_module("__builtins");

    vm->bind_func(mod, "next", 1, [](VM* vm, ArgsView args){
        return vm->py_next(args[0]);
    });

    vm->bind_func(mod, "_enable_instance_dict", 1, [](VM* vm, ArgsView args){
        PyVar self = args[0];
        if(is_tagged(self)) vm->TypeError("object: tagged object cannot enable instance dict");
        if(self->is_attr_valid()) vm->RuntimeError("object: instance dict is already enabled");
        self->_enable_instance_dict();
        return vm->None;
    });
}


/************************************************/
#if PK_ENABLE_PROFILER
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
    LineProfiler profiler;

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_func(type, __new__, 1, [](VM* vm, ArgsView args){
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
            PyVar func = view[1];
            const Tuple& args = CAST(Tuple&, view[2]);
            vm->s_data.push(func);
            vm->s_data.push(PY_NULL);
            for(PyVar arg : args) vm->s_data.push(arg);
            _LpGuard guard(&self, vm);
            PyVar ret = vm->vectorcall(args.size());
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
    PyVar mod = vm->new_module("line_profiler");
    vm->register_user_class<LineProfilerW>(mod, "LineProfiler");
}
#else
void add_module_line_profiler(VM* vm){
    (void)vm;
}
#endif

}   // namespace pkpy