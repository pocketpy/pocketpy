#pragma once

#include "codeobject.h"
#include "iter.h"
#include "error.h"

#define __DEF_PY_AS_C(type, ctype, ptype)                       \
    inline ctype& Py##type##_AS_C(const PyVar& obj) {           \
        __checkType(obj, ptype);                                \
        return UNION_GET(ctype, obj);                           \
    }

#define __DEF_PY(type, ctype, ptype)                            \
    inline PyVar Py##type(ctype value) {                        \
        return newObject(ptype, value);                         \
    }

#define DEF_NATIVE(type, ctype, ptype)                          \
    __DEF_PY(type, ctype, ptype)                                \
    __DEF_PY_AS_C(type, ctype, ptype)


class VM {
    std::atomic<bool> _stopFlag = false;
    std::vector<PyVar> _smallIntegers;      // [-5, 256]
protected:
    std::deque< pkpy::unique_ptr<Frame> > callstack;
    PyVarDict _modules;                     // loaded modules
    std::map<_Str, _Code> _lazyModules;     // lazy loaded modules
    PyVar __py2py_call_signal;
    
    void _checkStopFlag(){
        if(_stopFlag){
            _stopFlag = false;
            _error("KeyboardInterrupt", "");
        }
    }

    PyVar runFrame(Frame* frame){
        while(!frame->isCodeEnd()){
            const ByteCode& byte = frame->readCode();
            //printf("[%d] %s (%d)\n", frame->stackSize(), OP_NAMES[byte.op], byte.arg);
            //printf("%s\n", frame->code->src->getLine(byte.line).c_str());

            _checkStopFlag();

            switch (byte.op)
            {
            case OP_NO_OP: break;       // do nothing
            case OP_LOAD_CONST: frame->push(frame->code->co_consts[byte.arg]); break;
            case OP_LOAD_LAMBDA: {
                PyVar obj = frame->code->co_consts[byte.arg];
                setAttr(obj, __module__, frame->_module);
                frame->push(obj);
            } break;
            case OP_LOAD_NAME_REF: {
                frame->push(PyRef(NameRef(
                    &(frame->code->co_names[byte.arg])
                )));
            } break;
            case OP_STORE_NAME_REF: {
                const auto& p = frame->code->co_names[byte.arg];
                NameRef(&p).set(this, frame, frame->popValue(this));
            } break;
            case OP_BUILD_ATTR_REF: {
                const auto& attr = frame->code->co_names[byte.arg];
                PyVar obj = frame->popValue(this);
                frame->push(PyRef(AttrRef(obj, NameRef(&attr))));
            } break;
            case OP_BUILD_ATTR_REF_PTR: {
                const auto& attr = frame->code->co_names[byte.arg];
                PyVar obj = frame->popValue(this);
                __checkType(obj, _tp_user_pointer);
                const PyVarRef& var = UNION_GET(PyVarRef, obj);
                auto p = PyRef_AS_C(var);
                frame->push(PyRef(AttrRef(p->get(this, frame), &attr)));
            } break;
            case OP_BUILD_INDEX_REF: {
                PyVar index = frame->popValue(this);
                PyVarRef obj = frame->popValue(this);
                frame->push(PyRef(IndexRef(obj, index)));
            } break;
            case OP_STORE_REF: {
                PyVar obj = frame->popValue(this);
                PyVarRef r = frame->__pop();
                PyRef_AS_C(r)->set(this, frame, std::move(obj));
            } break;
            case OP_DELETE_REF: {
                PyVarRef r = frame->__pop();
                PyRef_AS_C(r)->del(this, frame);
            } break;
            case OP_BUILD_SMART_TUPLE:
            {
                pkpy::ArgList items = frame->__popNReversed(byte.arg);
                bool done = false;
                for(int i=0; i<items.size(); i++){
                    if(!items[i]->isType(_tp_ref)) {
                        done = true;
                        PyVarList values(items.size());
                        for(int i=0; i<items.size(); i++){
                            values[i] = frame->__deref_pointer(this, items[i]);
                        }
                        frame->push(PyTuple(values));
                        break;
                    }
                }
                if(done) break;
                frame->push(PyRef(TupleRef(items.toList())));
            } break;
            case OP_BUILD_STRING:
            {
                pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg);
                _StrStream ss;
                for(int i=0; i<items.size(); i++) ss << PyStr_AS_C(asStr(items[i]));
                frame->push(PyStr(ss.str()));
            } break;
            case OP_LOAD_EVAL_FN: {
                frame->push(builtins->attribs["eval"_c]);
            } break;
            case OP_LIST_APPEND: {
                pkpy::ArgList args(2);
                args[1] = frame->popValue(this);            // obj
                args[0] = frame->__topValueN(this, -2);     // list
                fastCall("append"_c, std::move(args));
            } break;
            case OP_STORE_FUNCTION:
                {
                    PyVar obj = frame->popValue(this);
                    const _Func& fn = PyFunction_AS_C(obj);
                    setAttr(obj, __module__, frame->_module);
                    frame->f_globals()[fn->name] = obj;
                } break;
            case OP_BUILD_CLASS:
                {
                    const _Str& clsName = frame->code->co_names[byte.arg].first;
                    PyVar clsBase = frame->popValue(this);
                    if(clsBase == None) clsBase = _tp_object;
                    __checkType(clsBase, _tp_type);
                    PyVar cls = newUserClassType(clsName, clsBase);
                    while(true){
                        PyVar fn = frame->popValue(this);
                        if(fn == None) break;
                        const _Func& f = PyFunction_AS_C(fn);
                        setAttr(fn, __module__, frame->_module);
                        setAttr(cls, f->name, fn);
                    }
                    frame->f_globals()[clsName] = cls;
                } break;
            case OP_RETURN_VALUE: return frame->popValue(this);
            case OP_PRINT_EXPR:
                {
                    const PyVar expr = frame->topValue(this);
                    if(expr == None) break;
                    *_stdout << PyStr_AS_C(asRepr(expr)) << '\n';
                } break;
            case OP_POP_TOP: frame->popValue(this); break;
            case OP_BINARY_OP:
                {
                    frame->push(
                        fastCall(BINARY_SPECIAL_METHODS[byte.arg],
                        frame->popNValuesReversed(this, 2))
                    );
                } break;
            case OP_BITWISE_OP:
                {
                    frame->push(
                        fastCall(BITWISE_SPECIAL_METHODS[byte.arg],
                        frame->popNValuesReversed(this, 2))
                    );
                } break;
            case OP_COMPARE_OP:
                {
                    // for __ne__ we use the negation of __eq__
                    int op = byte.arg == 3 ? 2 : byte.arg;
                    PyVar res = fastCall(CMP_SPECIAL_METHODS[op], frame->popNValuesReversed(this, 2));
                    if(op != byte.arg) res = PyBool(!PyBool_AS_C(res));
                    frame->push(std::move(res));
                } break;
            case OP_IS_OP:
                {
                    bool ret_c = frame->popValue(this) == frame->popValue(this);
                    if(byte.arg == 1) ret_c = !ret_c;
                    frame->push(PyBool(ret_c));
                } break;
            case OP_CONTAINS_OP:
                {
                    PyVar rhs = frame->popValue(this);
                    bool ret_c = PyBool_AS_C(call(rhs, __contains__, pkpy::oneArg(frame->popValue(this))));
                    if(byte.arg == 1) ret_c = !ret_c;
                    frame->push(PyBool(ret_c));
                } break;
            case OP_UNARY_NEGATIVE:
                {
                    PyVar obj = frame->popValue(this);
                    frame->push(numNegated(obj));
                } break;
            case OP_UNARY_NOT:
                {
                    PyVar obj = frame->popValue(this);
                    const PyVar& obj_bool = asBool(obj);
                    frame->push(PyBool(!PyBool_AS_C(obj_bool)));
                } break;
            case OP_UNARY_REF:
                {
                    // _pointer to pointer
                    PyVarRef obj = frame->__pop();
                    __checkType(obj, _tp_ref);
                    frame->push(newObject(
                        _tp_user_pointer,
                        PyRef(UserPointer(obj, frame->id))
                    ));
                } break;
            case OP_UNARY_DEREF:
                {
                    // pointer to _pointer
                    PyVar obj = frame->popValue(this);
                    __checkType(obj, _tp_user_pointer);
                    frame->push(UNION_GET(PyVarRef, obj));
                } break;
            case OP_POP_JUMP_IF_FALSE:
                if(!PyBool_AS_C(asBool(frame->popValue(this)))) frame->jump(byte.arg);
                break;
            case OP_LOAD_NONE: frame->push(None); break;
            case OP_LOAD_TRUE: frame->push(True); break;
            case OP_LOAD_FALSE: frame->push(False); break;
            case OP_LOAD_ELLIPSIS: frame->push(Ellipsis); break;
            case OP_ASSERT:
                {
                    PyVar expr = frame->popValue(this);
                    _assert(PyBool_AS_C(expr), "assertion failed");
                } break;
            case OP_RAISE_ERROR:
                {
                    _Str msg = PyStr_AS_C(asRepr(frame->popValue(this)));
                    _Str type = PyStr_AS_C(frame->popValue(this));
                    _error(type, msg);
                } break;
            case OP_BUILD_LIST:
                {
                    pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg);
                    frame->push(PyList(items.toList()));
                } break;
            case OP_BUILD_MAP:
                {
                    pkpy::ArgList items = frame->popNValuesReversed(this, byte.arg*2);
                    PyVar obj = call(builtins->attribs["dict"]);
                    for(int i=0; i<items.size(); i+=2){
                        call(obj, __setitem__, pkpy::twoArgs(items[i], items[i+1]));
                    }
                    frame->push(obj);
                } break;
            case OP_DUP_TOP: frame->push(frame->topValue(this)); break;
            case OP_CALL:
                {
                    int ARGC = byte.arg & 0xFFFF;
                    int KWARGC = (byte.arg >> 16) & 0xFFFF;
                    pkpy::ArgList kwargs(0);
                    if(KWARGC > 0) kwargs = frame->popNValuesReversed(this, KWARGC*2);
                    pkpy::ArgList args = frame->popNValuesReversed(this, ARGC);
                    PyVar callable = frame->popValue(this);
                    PyVar ret = call(callable, std::move(args), kwargs, true);
                    if(ret == __py2py_call_signal) return ret;
                    frame->push(std::move(ret));
                } break;
            case OP_JUMP_ABSOLUTE: frame->jump(byte.arg); break;
            case OP_SAFE_JUMP_ABSOLUTE: frame->safeJump(byte.arg); break;
            case OP_GOTO: {
                PyVar obj = frame->popValue(this);
                const _Str& label = PyStr_AS_C(obj);
                int* target = frame->code->co_labels.try_get(label);
                if(target == nullptr){
                    _error("KeyError", "label '" + label + "' not found");
                }
                frame->safeJump(*target);
            } break;
            case OP_GET_ITER:
                {
                    PyVar obj = frame->popValue(this);
                    PyVarOrNull iter_fn = getAttr(obj, __iter__, false);
                    if(iter_fn != nullptr){
                        PyVar tmp = call(iter_fn, pkpy::oneArg(obj));
                        PyVarRef var = frame->__pop();
                        __checkType(var, _tp_ref);
                        PyIter_AS_C(tmp)->var = var;
                        frame->push(std::move(tmp));
                    }else{
                        typeError("'" + UNION_TP_NAME(obj) + "' object is not iterable");
                    }
                } break;
            case OP_FOR_ITER:
                {
                    frame->__reportForIter();
                    // __top() must be PyIter, so no need to __deref()
                    auto& it = PyIter_AS_C(frame->__top());
                    if(it->hasNext()){
                        PyRef_AS_C(it->var)->set(this, frame, it->next());
                    }
                    else{
                        frame->safeJump(byte.arg);
                    }
                } break;
            case OP_JUMP_IF_FALSE_OR_POP:
                {
                    const PyVar expr = frame->topValue(this);
                    if(asBool(expr)==False) frame->jump(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_JUMP_IF_TRUE_OR_POP:
                {
                    const PyVar expr = frame->topValue(this);
                    if(asBool(expr)==True) frame->jump(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_BUILD_SLICE:
                {
                    PyVar stop = frame->popValue(this);
                    PyVar start = frame->popValue(this);
                    _Slice s;
                    if(start != None) {__checkType(start, _tp_int); s.start = (int)PyInt_AS_C(start);}
                    if(stop != None) {__checkType(stop, _tp_int); s.stop = (int)PyInt_AS_C(stop);}
                    frame->push(PySlice(s));
                } break;
            case OP_IMPORT_NAME:
                {
                    const _Str& name = frame->code->co_names[byte.arg].first;
                    auto it = _modules.find(name);
                    if(it == _modules.end()){
                        auto it2 = _lazyModules.find(name);
                        if(it2 == _lazyModules.end()){
                            _error("ImportError", "module '" + name + "' not found");
                        }else{
                            _Code code = it2->second;
                            PyVar _m = newModule(name);
                            _exec(code, _m, {});
                            frame->push(_m);
                            _lazyModules.erase(it2);
                        }
                    }else{
                        frame->push(it->second);
                    }
                } break;
            case OP_WITH_ENTER: call(frame->popValue(this), "__enter__"_c); break;
            case OP_WITH_EXIT: call(frame->popValue(this), "__exit__"_c); break;
            default:
                systemError(_Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
                break;
            }
        }

        if(frame->code->src->mode == EVAL_MODE || frame->code->src->mode == JSON_MODE){
            if(frame->stackSize() != 1) systemError("stack size is not 1 in EVAL_MODE/JSON_MODE");
            return frame->popValue(this);
        }

        if(frame->stackSize() != 0) systemError("stack not empty in EXEC_MODE");
        return None;
    }

public:
    PyVarDict _types;
    PyVar None, True, False, Ellipsis;

    bool use_stdio;
    std::ostream* _stdout;
    std::ostream* _stderr;
    
    PyVar builtins;         // builtins module
    PyVar _main;            // __main__ module

    int maxRecursionDepth = 1000;

    VM(bool use_stdio){
        this->use_stdio = use_stdio;
        if(use_stdio){
            std::cout.setf(std::ios::unitbuf);
            std::cerr.setf(std::ios::unitbuf);
            this->_stdout = &std::cout;
            this->_stderr = &std::cerr;
        }else{
            this->_stdout = new _StrStream();
            this->_stderr = new _StrStream();
        }
        initializeBuiltinClasses();

        _smallIntegers.reserve(300);
        for(_Int i=-5; i<=256; i++) _smallIntegers.push_back(newObject(_tp_int, i));
    }

    void keyboardInterrupt(){
        _stopFlag = true;
    }

    void sleepForSecs(_Float sec){
        _Int ms = (_Int)(sec * 1000);
        for(_Int i=0; i<ms; i+=20){
            _checkStopFlag();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    PyVar asStr(const PyVar& obj){
        PyVarOrNull str_fn = getAttr(obj, __str__, false);
        if(str_fn != nullptr) return call(str_fn);
        return asRepr(obj);
    }

    Frame* __findFrame(uint64_t up_f_id){
        for(auto it=callstack.crbegin(); it!=callstack.crend(); ++it){
            uint64_t f_id = it->get()->id;
            if(f_id == up_f_id) return it->get();
            if(f_id < up_f_id) return nullptr;
        }
        return nullptr;
    }

    Frame* topFrame(){
        if(callstack.size() == 0) UNREACHABLE();
        return callstack.back().get();
    }

    PyVar asRepr(const PyVar& obj){
        if(obj->isType(_tp_type)) return PyStr("<class '" + UNION_GET(_Str, obj->attribs[__name__]) + "'>");
        return call(obj, __repr__);
    }

    PyVar asJson(const PyVar& obj){
        return call(obj, __json__);
    }

    const PyVar& asBool(const PyVar& obj){
        if(obj == None) return False;
        if(obj->_type == _tp_bool) return obj;
        if(obj->_type == _tp_int) return PyBool(PyInt_AS_C(obj) != 0);
        if(obj->_type == _tp_float) return PyBool(PyFloat_AS_C(obj) != 0.0);
        PyVarOrNull len_fn = getAttr(obj, __len__, false);
        if(len_fn != nullptr){
            PyVar ret = call(len_fn);
            return PyBool(PyInt_AS_C(ret) > 0);
        }
        return True;
    }

    PyVar fastCall(const _Str& name, pkpy::ArgList&& args){
        PyObject* cls = args[0]->_type.get();
        while(cls != None.get()) {
            PyVar* val = cls->attribs.try_get(name);
            if(val != nullptr) return call(*val, std::move(args));
            cls = cls->attribs[__base__].get();
        }
        attributeError(args[0], name);
        return nullptr;
    }

    inline PyVar call(const PyVar& _callable){
        return call(_callable, pkpy::noArg(), pkpy::noArg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<std::remove_const_t<std::remove_reference_t<ArgT>>, pkpy::ArgList>, PyVar>
    call(const PyVar& _callable, ArgT&& args){
        return call(_callable, std::forward<ArgT>(args), pkpy::noArg(), false);
    }

    template<typename ArgT>
    inline std::enable_if_t<std::is_same_v<std::remove_const_t<std::remove_reference_t<ArgT>>, pkpy::ArgList>, PyVar>
    call(const PyVar& obj, const _Str& func, ArgT&& args){
        return call(getAttr(obj, func), std::forward<ArgT>(args), pkpy::noArg(), false);
    }

    inline PyVar call(const PyVar& obj, const _Str& func){
        return call(getAttr(obj, func), pkpy::noArg(), pkpy::noArg(), false);
    }

    PyVar call(const PyVar& _callable, pkpy::ArgList args, const pkpy::ArgList& kwargs, bool opCall){
        if(_callable->isType(_tp_type)){
            auto it = _callable->attribs.find(__new__);
            PyVar obj;
            if(it != _callable->attribs.end()){
                obj = call(it->second, args, kwargs, false);
            }else{
                obj = newObject(_callable, (_Int)-1);
                PyVarOrNull init_fn = getAttr(obj, __init__, false);
                if (init_fn != nullptr) call(init_fn, args, kwargs, false);
            }
            return obj;
        }

        const PyVar* callable = &_callable;
        if((*callable)->isType(_tp_bounded_method)){
            auto& bm = PyBoundedMethod_AS_C((*callable));
            // TODO: avoid insertion here, bad performance
            pkpy::ArgList new_args(args.size()+1);
            new_args[0] = bm.obj;
            for(int i=0; i<args.size(); i++) new_args[i+1] = args[i];
            callable = &bm.method;
            args = std::move(new_args);
        }
        
        if((*callable)->isType(_tp_native_function)){
            const auto& f = UNION_GET(_CppFunc, *callable);
            // _CppFunc do not support kwargs
            return f(this, args);
        } else if((*callable)->isType(_tp_function)){
            const _Func& fn = PyFunction_AS_C((*callable));
            PyVarDict locals;
            int i = 0;
            for(const auto& name : fn->args){
                if(i < args.size()){
                    locals.emplace(name, args[i++]);
                    continue;
                }
                typeError("missing positional argument '" + name + "'");
            }

            locals.insert(fn->kwArgs.begin(), fn->kwArgs.end());

            std::vector<_Str> positional_overrided_keys;
            if(!fn->starredArg.empty()){
                // handle *args
                PyVarList vargs;
                while(i < args.size()) vargs.push_back(args[i++]);
                locals.emplace(fn->starredArg, PyTuple(std::move(vargs)));
            }else{
                for(const auto& key : fn->kwArgsOrder){
                    if(i < args.size()){
                        locals[key] = args[i++];
                        positional_overrided_keys.push_back(key);
                    }else{
                        break;
                    }
                }
                if(i < args.size()) typeError("too many arguments");
            }
            
            for(int i=0; i<kwargs.size(); i+=2){
                const _Str& key = PyStr_AS_C(kwargs[i]);
                if(!fn->kwArgs.contains(key)){
                    typeError(key.__escape(true) + " is an invalid keyword argument for " + fn->name + "()");
                }
                const PyVar& val = kwargs[i+1];
                if(!positional_overrided_keys.empty()){
                    auto it = std::find(positional_overrided_keys.begin(), positional_overrided_keys.end(), key);
                    if(it != positional_overrided_keys.end()){
                        typeError("multiple values for argument '" + key + "'");
                    }
                }
                locals[key] = val;
            }

            PyVar* it_m = (*callable)->attribs.try_get(__module__);
            PyVar _module = it_m != nullptr ? *it_m : topFrame()->_module;
            if(opCall){
                __pushNewFrame(fn->code, _module, std::move(locals));
                return __py2py_call_signal;
            }
            return _exec(fn->code, _module, std::move(locals));
        }
        typeError("'" + UNION_TP_NAME(*callable) + "' object is not callable");
        return None;
    }


    // repl mode is only for setting `frame->id` to 0
    virtual PyVarOrNull exec(const _Code& code, PyVar _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            return _exec(code, _module, {});
        }catch (const _Error& e){
            *_stderr << e.what() << '\n';
        }catch (const std::exception& e) {
            auto re = RuntimeError("UnexpectedError", e.what(), _cleanErrorAndGetSnapshots());
            *_stderr << re.what() << '\n';
        }
        return nullptr;
    }

    virtual void execAsync(const _Code& code) {
        exec(code);
    }

    Frame* __pushNewFrame(const _Code& code, PyVar _module, PyVarDict&& locals){
        if(code == nullptr) UNREACHABLE();
        if(callstack.size() > maxRecursionDepth){
            throw RuntimeError("RecursionError", "maximum recursion depth exceeded", _cleanErrorAndGetSnapshots());
        }
        Frame* frame = new Frame(code.get(), _module, std::move(locals));
        callstack.emplace_back(pkpy::unique_ptr<Frame>(frame));
        return frame;
    }

    PyVar _exec(const _Code& code, PyVar _module, PyVarDict&& locals){
        Frame* frame = __pushNewFrame(code, _module, std::move(locals));
        if(code->mode() == SINGLE_MODE) frame->id = 0;
        Frame* frameBase = frame;
        PyVar ret = nullptr;

        while(true){
            ret = runFrame(frame);
            if(ret != __py2py_call_signal){
                if(frame == frameBase){         // [ frameBase<- ]
                    break;
                }else{
                    callstack.pop_back();
                    frame = callstack.back().get();
                    frame->push(ret);
                }
            }else{
                frame = callstack.back().get();  // [ frameBase, newFrame<- ]
            }
        }

        callstack.pop_back();
        return ret;
    }

    PyVar newUserClassType(_Str name, PyVar base){
        PyVar obj = newClassType(name, base);
        setAttr(obj, __name__, PyStr(name));
        _types.erase(name);
        return obj;
    }

    PyVar newClassType(_Str name, PyVar base=nullptr) {
        if(base == nullptr) base = _tp_object;
        PyVar obj = pkpy::make_shared<PyObject, Py_<_Int>>((_Int)0, _tp_type);
        setAttr(obj, __base__, base);
        _types[name] = obj;
        return obj;
    }

    template<typename T>
    inline PyVar newObject(PyVar type, T _value) {
        __checkType(type, _tp_type);
        return pkpy::make_shared<PyObject, Py_<T>>(_value, type);
    }

    PyVar newModule(_Str name) {
        PyVar obj = newObject(_tp_module, (_Int)-2);
        setAttr(obj, __name__, PyStr(name));
        _modules[name] = obj;
        return obj;
    }

    void addLazyModule(_Str name, _Code code){
        _lazyModules[name] = code;
    }

    PyVarOrNull getAttr(const PyVar& obj, const _Str& name, bool throw_err=true) {
        PyVarDict::iterator it;
        PyObject* cls;

        if(obj->isType(_tp_super)){
            const PyVar* root = &obj;
            int depth = 1;
            while(true){
                root = &UNION_GET(PyVar, *root);
                if(!(*root)->isType(_tp_super)) break;
                depth++;
            }
            cls = (*root)->_type.get();
            for(int i=0; i<depth; i++) cls = cls->attribs[__base__].get();

            it = (*root)->attribs.find(name);
            if(it != (*root)->attribs.end()) return it->second;        
        }else{
            it = obj->attribs.find(name);
            if(it != obj->attribs.end()) return it->second;
            cls = obj->_type.get();
        }

        while(cls != None.get()) {
            it = cls->attribs.find(name);
            if(it != cls->attribs.end()){
                PyVar valueFromCls = it->second;
                if(valueFromCls->isType(_tp_function) || valueFromCls->isType(_tp_native_function)){
                    return PyBoundedMethod({obj, std::move(valueFromCls)});
                }else{
                    return valueFromCls;
                }
            }
            cls = cls->attribs[__base__].get();
        }
        if(throw_err) attributeError(obj, name);
        return nullptr;
    }

    void setAttr(PyVar& obj, const _Str& name, const PyVar& value) {
        if(obj->isType(_tp_super)){
            const PyVar* root = &obj;
            while(true){
                root = &UNION_GET(PyVar, *root);
                if(!(*root)->isType(_tp_super)) break;
            }
            (*root)->attribs[name] = value;
        }else{
            obj->attribs[name] = value;
        }
    }

    void setAttr(PyVar& obj, const _Str& name, PyVar&& value) {
        if(obj->isType(_tp_super)){
            const PyVar* root = &obj;
            while(true){
                root = &UNION_GET(PyVar, *root);
                if(!(*root)->isType(_tp_super)) break;
            }
            (*root)->attribs[name] = std::move(value);
        }else{
            obj->attribs[name] = std::move(value);
        }
    }

    void bindMethod(_Str typeName, _Str funcName, _CppFunc fn) {
        funcName.intern();
        PyVar type = _types[typeName];
        PyVar func = PyNativeFunction(fn);
        setAttr(type, funcName, func);
    }

    void bindMethodMulti(std::vector<_Str> typeNames, _Str funcName, _CppFunc fn) {
        for(auto& typeName : typeNames){
            bindMethod(typeName, funcName, fn);
        }
    }

    void bindBuiltinFunc(_Str funcName, _CppFunc fn) {
        bindFunc(builtins, funcName, fn);
    }

    void bindFunc(PyVar module, _Str funcName, _CppFunc fn) {
        funcName.intern();
        __checkType(module, _tp_module);
        PyVar func = PyNativeFunction(fn);
        setAttr(module, funcName, func);
    }

    bool isInstance(PyVar obj, PyVar type){
        __checkType(type, _tp_type);
        PyObject* t = obj->_type.get();
        while (t != None.get()){
            if (t == type.get()) return true;
            t = t->attribs[__base__].get();
        }
        return false;
    }

    inline bool isIntOrFloat(const PyVar& obj){
        return obj->isType(_tp_int) || obj->isType(_tp_float);
    }

    inline bool isIntOrFloat(const PyVar& obj1, const PyVar& obj2){
        return isIntOrFloat(obj1) && isIntOrFloat(obj2);
    }

    inline _Float numToFloat(const PyVar& obj){
        if (obj->isType(_tp_int)){
            return (_Float)PyInt_AS_C(obj);
        }else if(obj->isType(_tp_float)){
            return PyFloat_AS_C(obj);
        }
        UNREACHABLE();
    }

    PyVar numNegated(const PyVar& obj){
        if (obj->isType(_tp_int)){
            return PyInt(-PyInt_AS_C(obj));
        }else if(obj->isType(_tp_float)){
            return PyFloat(-PyFloat_AS_C(obj));
        }
        typeError("unsupported operand type(s) for -");
        return nullptr;
    }

    int normalizedIndex(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            indexError("index out of range, " + std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    // for quick access
    PyVar _tp_object, _tp_type, _tp_int, _tp_float, _tp_bool, _tp_str;
    PyVar _tp_list, _tp_tuple;
    PyVar _tp_function, _tp_native_function, _tp_native_iterator, _tp_bounded_method;
    PyVar _tp_slice, _tp_range, _tp_module, _tp_ref;
    PyVar _tp_user_pointer, _tp_super;

    template<typename P>
    inline PyVarRef PyRef(P&& value) {
        static_assert(std::is_base_of<BaseRef, P>::value, "P should derive from BaseRef");
        return newObject(_tp_ref, std::forward<P>(value));
    }

    inline const BaseRef* PyRef_AS_C(const PyVar& obj)
    {
        if(!obj->isType(_tp_ref)) typeError("expected an l-value");
        return (const BaseRef*)(obj->value());
    }

    __DEF_PY_AS_C(Int, _Int, _tp_int)
    inline PyVar PyInt(_Int value) { 
        if(value >= -5 && value <= 256) return _smallIntegers[value + 5];
        return newObject(_tp_int, value);
    }

    DEF_NATIVE(Float, _Float, _tp_float)
    DEF_NATIVE(Str, _Str, _tp_str)
    DEF_NATIVE(List, PyVarList, _tp_list)
    DEF_NATIVE(Tuple, PyVarList, _tp_tuple)
    DEF_NATIVE(Function, _Func, _tp_function)
    DEF_NATIVE(NativeFunction, _CppFunc, _tp_native_function)
    DEF_NATIVE(Iter, _Iterator, _tp_native_iterator)
    DEF_NATIVE(BoundedMethod, _BoundedMethod, _tp_bounded_method)
    DEF_NATIVE(Range, _Range, _tp_range)
    DEF_NATIVE(Slice, _Slice, _tp_slice)
    
    // there is only one True/False, so no need to copy them!
    inline bool PyBool_AS_C(const PyVar& obj){return obj == True;}
    inline const PyVar& PyBool(bool value){return value ? True : False;}

    void initializeBuiltinClasses(){
        _tp_object = pkpy::make_shared<PyObject, Py_<_Int>>((_Int)0, nullptr);
        _tp_type = pkpy::make_shared<PyObject, Py_<_Int>>((_Int)0, nullptr);

        _types["object"] = _tp_object;
        _types["type"] = _tp_type;

        _tp_bool = newClassType("bool");
        _tp_int = newClassType("int");
        _tp_float = newClassType("float");
        _tp_str = newClassType("str");
        _tp_list = newClassType("list");
        _tp_tuple = newClassType("tuple");
        _tp_slice = newClassType("slice");
        _tp_range = newClassType("range");
        _tp_module = newClassType("module");
        _tp_ref = newClassType("_pointer");
        _tp_user_pointer = newClassType("pointer");

        newClassType("NoneType");
        newClassType("ellipsis");
        
        _tp_function = newClassType("function");
        _tp_native_function = newClassType("_native_function");
        _tp_native_iterator = newClassType("_native_iterator");
        _tp_bounded_method = newClassType("_bounded_method");
        _tp_super = newClassType("super");

        this->None = newObject(_types["NoneType"], (_Int)0);
        this->Ellipsis = newObject(_types["ellipsis"], (_Int)0);
        this->True = newObject(_tp_bool, true);
        this->False = newObject(_tp_bool, false);
        this->builtins = newModule("builtins");
        this->_main = newModule("__main__"_c);

        setAttr(_tp_type, __base__, _tp_object);
        _tp_type->_type = _tp_type;
        setAttr(_tp_object, __base__, None);
        _tp_object->_type = _tp_type;
        
        for (auto& [name, type] : _types) {
            setAttr(type, __name__, PyStr(name));
        }

        this->__py2py_call_signal = newObject(_tp_object, (_Int)7);

        std::vector<_Str> publicTypes = {"type", "object", "bool", "int", "float", "str", "list", "tuple", "range"};
        for (auto& name : publicTypes) {
            setAttr(builtins, name, _types[name]);
        }
    }

    _Int hash(const PyVar& obj){
        if (obj->isType(_tp_int)) return PyInt_AS_C(obj);
        if (obj->isType(_tp_bool)) return PyBool_AS_C(obj) ? 1 : 0;
        if (obj->isType(_tp_float)){
            _Float val = PyFloat_AS_C(obj);
            return (_Int)std::hash<_Float>()(val);
        }
        if (obj->isType(_tp_str)) return PyStr_AS_C(obj).hash();
        if (obj->isType(_tp_type)) return (_Int)obj.get();
        if (obj->isType(_tp_tuple)) {
            _Int x = 1000003;
            for (const auto& item : PyTuple_AS_C(obj)) {
                _Int y = hash(item);
                // this is recommended by Github Copilot
                // i am not sure whether it is a good idea
                x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
            }
            return x;
        }
        typeError("unhashable type: " +  UNION_TP_NAME(obj));
        return 0;
    }

    /***** Error Reporter *****/
private:
    void _error(const _Str& name, const _Str& msg){
        throw RuntimeError(name, msg, _cleanErrorAndGetSnapshots());
    }

    std::stack<_Str> _cleanErrorAndGetSnapshots(){
        std::stack<_Str> snapshots;
        while (!callstack.empty()){
            if(snapshots.size() < 8){
                snapshots.push(callstack.back()->errorSnapshot());
            }
            callstack.pop_back();
        }
        return snapshots;
    }

public:
    void typeError(const _Str& msg){
        _error("TypeError", msg);
    }

    void systemError(const _Str& msg){
        _error("SystemError", msg);
    }

    void nullPointerError(){
        _error("NullPointerError", "pointer is invalid");
    }

    void zeroDivisionError(){
        _error("ZeroDivisionError", "division by zero");
    }

    void indexError(const _Str& msg){
        _error("IndexError", msg);
    }

    void valueError(const _Str& msg){
        _error("ValueError", msg);
    }

    void nameError(const _Str& name){
        _error("NameError", "name '" + name + "' is not defined");
    }

    void attributeError(PyVar obj, const _Str& name){
        _error("AttributeError", "type '" +  UNION_TP_NAME(obj) + "' has no attribute '" + name + "'");
    }

    inline void __checkType(const PyVar& obj, const PyVar& type){
#ifndef PKPY_NO_TYPE_CHECK
        if(!obj->isType(type)) typeError("expected '" + UNION_NAME(type) + "', but got '" + UNION_TP_NAME(obj) + "'");
#endif
    }

    inline void __checkArgSize(const pkpy::ArgList& args, int size, bool method=false){
        if(args.size() == size) return;
        if(method) typeError(args.size()>size ? "too many arguments" : "too few arguments");
        else typeError("expected " + std::to_string(size) + " arguments, but got " + std::to_string(args.size()));
    }

    void _assert(bool val, const _Str& msg){
        if (!val) _error("AssertionError", msg);
    }

    virtual ~VM() {
        if(!use_stdio){
            delete _stdout;
            delete _stderr;
        }
    }
};

/***** Pointers' Impl *****/

PyVar NameRef::get(VM* vm, Frame* frame) const{
    PyVar* val = frame->f_locals.try_get(pair->first);
    if(val) return *val;
    val = frame->f_globals().try_get(pair->first);
    if(val) return *val;
    val = vm->builtins->attribs.try_get(pair->first);
    if(val) return *val;
    vm->nameError(pair->first);
    return nullptr;
}

void NameRef::set(VM* vm, Frame* frame, PyVar val) const{
    switch(pair->second) {
        case NAME_LOCAL: frame->f_locals[pair->first] = std::move(val); break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals.count(pair->first) > 0){
                frame->f_locals[pair->first] = std::move(val);
            }else{
                frame->f_globals()[pair->first] = std::move(val);
            }
        } break;
        default: UNREACHABLE();
    }
}

void NameRef::del(VM* vm, Frame* frame) const{
    switch(pair->second) {
        case NAME_LOCAL: {
            if(frame->f_locals.count(pair->first) > 0){
                frame->f_locals.erase(pair->first);
            }else{
                vm->nameError(pair->first);
            }
        } break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals.count(pair->first) > 0){
                frame->f_locals.erase(pair->first);
            }else{
                if(frame->f_globals().count(pair->first) > 0){
                    frame->f_globals().erase(pair->first);
                }else{
                    vm->nameError(pair->first);
                }
            }
        } break;
        default: UNREACHABLE();
    }
}

PyVar AttrRef::get(VM* vm, Frame* frame) const{
    return vm->getAttr(obj, attr.pair->first);
}

void AttrRef::set(VM* vm, Frame* frame, PyVar val) const{
    vm->setAttr(obj, attr.pair->first, val);
}

void AttrRef::del(VM* vm, Frame* frame) const{
    vm->typeError("cannot delete attribute");
}

PyVar IndexRef::get(VM* vm, Frame* frame) const{
    return vm->call(obj, __getitem__, pkpy::oneArg(index));
}

void IndexRef::set(VM* vm, Frame* frame, PyVar val) const{
    vm->call(obj, __setitem__, pkpy::twoArgs(index, val));
}

void IndexRef::del(VM* vm, Frame* frame) const{
    vm->call(obj, __delitem__, pkpy::oneArg(index));
}

PyVar TupleRef::get(VM* vm, Frame* frame) const{
    PyVarList args(varRefs.size());
    for (int i = 0; i < varRefs.size(); i++) {
        args[i] = vm->PyRef_AS_C(varRefs[i])->get(vm, frame);
    }
    return vm->PyTuple(args);
}

void TupleRef::set(VM* vm, Frame* frame, PyVar val) const{
    if(!val->isType(vm->_tp_tuple) && !val->isType(vm->_tp_list)){
        vm->typeError("only tuple or list can be unpacked");
    }
    const PyVarList& args = UNION_GET(PyVarList, val);
    if(args.size() > varRefs.size()) vm->valueError("too many values to unpack");
    if(args.size() < varRefs.size()) vm->valueError("not enough values to unpack");
    for (int i = 0; i < varRefs.size(); i++) {
        vm->PyRef_AS_C(varRefs[i])->set(vm, frame, args[i]);
    }
}

void TupleRef::del(VM* vm, Frame* frame) const{
    for (auto& r : varRefs) vm->PyRef_AS_C(r)->del(vm, frame);
}

PyVar UserPointer::get(VM* vm, Frame* frame) const{
    frame = vm->__findFrame(f_id);
    if(frame == nullptr) vm->nullPointerError();
    return vm->PyRef_AS_C(p)->get(vm, frame);
}

void UserPointer::set(VM* vm, Frame* frame, PyVar val) const{
    frame = vm->__findFrame(f_id);
    if(frame == nullptr) vm->nullPointerError();
    vm->PyRef_AS_C(p)->set(vm, frame, val);
}

void UserPointer::del(VM* vm, Frame* frame) const{
    vm->typeError("delete is unsupported");
}

/***** Frame's Impl *****/
inline PyVar Frame::__deref_pointer(VM* vm, PyVar v){
    if(v->isType(vm->_tp_ref)) return vm->PyRef_AS_C(v)->get(vm, this);
    return v;
}

/***** Iterators' Impl *****/
PyVar RangeIterator::next(){
    PyVar val = vm->PyInt(current);
    current += r.step;
    return val;
}

PyVar StringIterator::next(){
    return vm->PyStr(str.u8_getitem(index++));
}

enum ThreadState {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SUSPENDED,
    THREAD_FINISHED
};

class ThreadedVM : public VM {
    std::thread* _thread = nullptr;
    std::atomic<ThreadState> _state = THREAD_READY;
    _Str _sharedStr = ""_c;
    
    void __deleteThread(){
        if(_thread != nullptr){
            terminate();
            _thread->join();
            delete _thread;
            _thread = nullptr;
        }
    }
public:
    ThreadedVM(bool use_stdio) : VM(use_stdio) {
        bindBuiltinFunc("__string_channel_call", [](VM* vm, const pkpy::ArgList& args){
            vm->__checkArgSize(args, 1);
            _Str data = vm->PyStr_AS_C(args[0]);

            ThreadedVM* tvm = (ThreadedVM*)vm;
            tvm->_sharedStr = data;
            tvm->suspend();
            return tvm->PyStr(tvm->readJsonRpcRequest());
        });
    }

    void terminate(){
        if(_state == THREAD_RUNNING || _state == THREAD_SUSPENDED){
            keyboardInterrupt();
            while(_state != THREAD_FINISHED);
        }
    }

    void suspend(){
        if(_state != THREAD_RUNNING) UNREACHABLE();
        _state = THREAD_SUSPENDED;
        while(_state == THREAD_SUSPENDED) _checkStopFlag();
    }

    _Str readJsonRpcRequest(){
        _Str copy = _sharedStr;
        _sharedStr = ""_c;
        return copy;
    }

    /***** For outer use *****/

    ThreadState getState(){
        return _state;
    }

    void writeJsonrpcResponse(const char* value){
        if(_state != THREAD_SUSPENDED) UNREACHABLE();
        _sharedStr = _Str(value);
        _state = THREAD_RUNNING;
    }

    void execAsync(const _Code& code) override {
        if(_state != THREAD_READY) UNREACHABLE();
        __deleteThread();
        _thread = new std::thread([this, code](){
            this->_state = THREAD_RUNNING;
            VM::exec(code);
            this->_state = THREAD_FINISHED;
        });
    }

    PyVarOrNull exec(const _Code& code, PyVar _module = nullptr) override {
        if(_state == THREAD_READY) return VM::exec(code, _module);
        auto callstackBackup = std::move(callstack);
        callstack.clear();
        PyVarOrNull ret = VM::exec(code, _module);
        callstack = std::move(callstackBackup);
        return ret;
    }

    void resetState(){
        if(this->_state != THREAD_FINISHED) return;
        this->_state = THREAD_READY;
    }

    ~ThreadedVM(){
        __deleteThread();
    }
};