#pragma once

#include "codeobject.h"
#include "iter.h"
#include "error.h"

#define __DEF_PY_AS_C(type, ctype, ptype)                       \
    inline ctype& Py##type##_AS_C(const PyVar& obj) {           \
        __checkType(obj, ptype);                                \
        return std::get<ctype>(obj->_native);                   \
    }

#define __DEF_PY(type, ctype, ptype)                            \
    inline PyVar Py##type(ctype value) {                        \
        return newObject(ptype, value);                         \
    }

#define DEF_NATIVE(type, ctype, ptype)                          \
    __DEF_PY(type, ctype, ptype)                                \
    __DEF_PY_AS_C(type, ctype, ptype)

#define __DEF_PY_POOL(name, ctype, ptype, max_size) \
    std::vector<PyObject*> _pool##name;             \
    PyVar Py##name(ctype _native) {                 \
        PyObject* _raw = nullptr;                   \
        if(_pool##name.size() > 0) {                \
            _raw = _pool##name.back();              \
            _raw->_native = _native;                \
            _pool##name.pop_back();                 \
        }else{                                      \
            __checkType(ptype, _tp_type);           \
            _raw = new PyObject(_native);           \
            _raw->attribs[__class__] = ptype;       \
        }                                           \
        PyVar obj = PyVar(_raw, [this](PyObject* p){\
            if(_pool##name.size() < max_size){      \
                _pool##name.push_back(p);           \
            }else{                                  \
                delete p;                           \
            }                                       \
        });                                         \
        return obj;                                 \
    }

typedef void(*PrintFn)(const VM*, const char*);

class VM: public PkExportedResource{
private:
    std::stack< std::unique_ptr<Frame> > callstack;
    PyVarDict _modules;       // 3rd modules

    PyVar __py2py_call_signal;

    PyVar runFrame(Frame* frame){
        while(!frame->isCodeEnd()){
            const ByteCode& byte = frame->readCode();
            //printf("%s (%d) stack_size: %d\n", OP_NAMES[byte.op], byte.arg, frame->stackSize());

            switch (byte.op)
            {
            case OP_NO_OP: break;       // do nothing
            case OP_LOAD_CONST: frame->push(frame->code->co_consts[byte.arg]); break;
            case OP_LOAD_LAMBDA: {
                PyVar obj = frame->code->co_consts[byte.arg];
                setAttr(obj, __module__, frame->_module);
                frame->push(obj);
            } break;
            case OP_LOAD_NAME_PTR: {
                frame->push(PyPointer(frame->code->co_names[byte.arg]));
            } break;
            case OP_STORE_NAME_PTR: {
                const auto& p = frame->code->co_names[byte.arg];
                p->set(this, frame, frame->popValue(this));
            } break;
            case OP_BUILD_ATTR_PTR: {
                const auto& attr = frame->code->co_names[byte.arg];
                PyVar obj = frame->popValue(this);
                frame->push(PyPointer(std::make_shared<AttrPointer>(obj, attr.get())));
            } break;
            case OP_BUILD_INDEX_PTR: {
                PyVar index = frame->popValue(this);
                PyVar obj = frame->popValue(this);
                frame->push(PyPointer(std::make_shared<IndexPointer>(obj, index)));
            } break;
            case OP_STORE_PTR: {
                PyVar obj = frame->popValue(this);
                _Pointer p = PyPointer_AS_C(frame->__pop());
                p->set(this, frame, obj);
            } break;
            case OP_DELETE_PTR: {
                _Pointer p = PyPointer_AS_C(frame->__pop());
                p->del(this, frame);
            } break;
            case OP_BUILD_SMART_TUPLE:
            {
                PyVarList items = frame->__popNReversed(byte.arg);
                bool done = false;
                for(auto& item : items){
                    if(!item->isType(_tp_pointer)) {
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
                std::vector<_Pointer> pointers(items.size());
                for(int i=0; i<items.size(); i++)
                    pointers[i] = PyPointer_AS_C(items[i]);
                frame->push(PyPointer(std::make_shared<CompoundPointer>(pointers)));
            } break;
            case OP_BUILD_STRING:
            {
                PyVarList items = frame->popNValuesReversed(this, byte.arg);
                _StrStream ss;
                for(const auto& i : items) ss << PyStr_AS_C(asStr(i));
                frame->push(PyStr(ss));
            } break;
            case OP_LOAD_EVAL_FN: {
                frame->push(builtins->attribs["eval"]);
            } break;
            case OP_LIST_APPEND: {
                PyVar obj = frame->popValue(this);
                PyVar list = frame->topNValue(this, -2);
                fastCall(list, "append", {list, obj});
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
                    const _Str& clsName = frame->code->co_names[byte.arg]->name;
                    PyVar clsBase = frame->popValue(this);
                    if(clsBase == None) clsBase = _tp_object;
                    __checkType(clsBase, _tp_type);
                    PyVar cls = newUserClassType(clsName, clsBase);
                    while(true){
                        PyVar fn = frame->popValue(this);
                        if(fn == None) break;
                        const _Func& f = PyFunction_AS_C(fn);
                        setAttr(cls, f->name, fn);
                    }
                    frame->f_globals()[clsName] = cls;
                } break;
            case OP_RETURN_VALUE: return frame->popValue(this);
            case OP_PRINT_EXPR:
                {
                    const PyVar& expr = frame->topValue(this);
                    if(expr == None) break;
                    _stdout(this, PyStr_AS_C(asRepr(expr)).c_str());
                    _stdout(this, "\n");
                } break;
            case OP_POP_TOP: frame->popValue(this); break;
            case OP_BINARY_OP:
                {
                    PyVar rhs = frame->popValue(this);
                    PyVar lhs = frame->popValue(this);
                    frame->push(fastCall(lhs, BIN_SPECIAL_METHODS[byte.arg], {lhs,rhs}));
                } break;
            case OP_COMPARE_OP:
                {
                    PyVar rhs = frame->popValue(this);
                    PyVar lhs = frame->popValue(this);
                    // for __ne__ we use the negation of __eq__
                    int op = byte.arg == 3 ? 2 : byte.arg;
                    PyVar res = fastCall(lhs, CMP_SPECIAL_METHODS[op], {lhs,rhs});
                    if(op != byte.arg) res = PyBool(!PyBool_AS_C(res));
                    frame->push(res);
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
                    PyVar lhs = frame->popValue(this);
                    bool ret_c = PyBool_AS_C(call(rhs, __contains__, {lhs}));
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
                    PyVar obj_bool = asBool(obj);
                    frame->push(PyBool(!PyBool_AS_C(obj_bool)));
                } break;
            case OP_POP_JUMP_IF_FALSE:
                if(!PyBool_AS_C(asBool(frame->popValue(this)))) frame->jumpTo(byte.arg);
                break;
            case OP_LOAD_NONE: frame->push(None); break;
            case OP_LOAD_TRUE: frame->push(True); break;
            case OP_LOAD_FALSE: frame->push(False); break;
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
                    PyVarList items = frame->popNValuesReversed(this, byte.arg);
                    frame->push(PyList(items));
                } break;
            case OP_BUILD_MAP:
                {
                    PyVarList items = frame->popNValuesReversed(this, byte.arg*2);
                    PyVar obj = call(builtins->attribs["dict"], {});
                    for(int i=0; i<items.size(); i+=2){
                        call(obj, __setitem__, {items[i], items[i+1]});
                    }
                    frame->push(obj);
                } break;
            case OP_DUP_TOP: frame->push(frame->topValue(this)); break;
            case OP_CALL:
                {
                    PyVarList args = frame->popNValuesReversed(this, byte.arg);
                    PyVar callable = frame->popValue(this);
                    PyVar ret = call(callable, args, true);
                    if(ret == __py2py_call_signal) return ret;
                    frame->push(ret);
                } break;
            case OP_JUMP_ABSOLUTE: frame->jumpTo(byte.arg); break;
            case OP_GET_ITER:
                {
                    PyVar obj = frame->popValue(this);
                    PyVarOrNull iter_fn = getAttr(obj, __iter__, false);
                    if(iter_fn != nullptr){
                        PyVar tmp = call(iter_fn, {obj});
                        PyIter_AS_C(tmp)->var = PyPointer_AS_C(frame->__pop());
                        frame->push(tmp);
                    }else{
                        typeError("'" + obj->getTypeName() + "' object is not iterable");
                    }
                } break;
            case OP_FOR_ITER:
                {
                    const PyVar& iter = frame->topValue(this);
                    auto& it = PyIter_AS_C(iter);
                    if(it->hasNext()){
                        it->var->set(this, frame, it->next());
                    }
                    else{
                        frame->popValue(this);
                        frame->jumpTo(byte.arg);
                    }
                } break;
            case OP_JUMP_IF_FALSE_OR_POP:
                {
                    const PyVar& expr = frame->topValue(this);
                    if(asBool(expr)==False) frame->jumpTo(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_JUMP_IF_TRUE_OR_POP:
                {
                    const PyVar& expr = frame->topValue(this);
                    if(asBool(expr)==True) frame->jumpTo(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_BUILD_SLICE:
                {
                    PyVar stop = frame->popValue(this);
                    PyVar start = frame->popValue(this);
                    _Slice s;
                    if(start != None) {__checkType(start, _tp_int); s.start = PyInt_AS_C(start);}
                    if(stop != None) {__checkType(stop, _tp_int); s.stop = PyInt_AS_C(stop);}
                    frame->push(PySlice(s));
                } break;
            case OP_IMPORT_NAME:
                {
                    const _Str& name = frame->code->co_names[byte.arg]->name;
                    auto it = _modules.find(name);
                    if(it == _modules.end()) _error("ImportError", "module '" + name + "' not found");
                    else frame->push(it->second); 
                } break;
            default:
                systemError(_Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
                break;
            }
        }

        if(frame->code->src->mode == EVAL_MODE) {
            if(frame->stackSize() != 1) systemError("stack size is not 1 in EVAL_MODE");
            return frame->popValue(this);
        }

        if(frame->stackSize() != 0) systemError("stack not empty in EXEC_MODE");
        return None;
    }

public:
    PyVarDict _types;         // builtin types
    PyVar None, True, False;

    PrintFn _stdout = [](const VM* vm, auto s){};
    PrintFn _stderr = [](const VM* vm, auto s){};
    
    PyVar builtins;         // builtins module
    PyVar _main;            // __main__ module

    int maxRecursionDepth = 1000;

    VM(){
        initializeBuiltinClasses();
    }

    PyVar asStr(const PyVar& obj){
        PyVarOrNull str_fn = getAttr(obj, __str__, false);
        if(str_fn != nullptr) return call(str_fn, {});
        return asRepr(obj);
    }

    Frame* topFrame(){
        if(callstack.size() == 0) UNREACHABLE();
        return callstack.top().get();
    }

    PyVar asRepr(const PyVar& obj){
        if(obj->isType(_tp_type)) return PyStr("<class '" + obj->getName() + "'>");
        return call(obj, __repr__, {});
    }

    PyVar asBool(const PyVar& obj){
        if(obj == None) return False;
        PyVar tp = obj->attribs[__class__];
        if(tp == _tp_bool) return obj;
        if(tp == _tp_int) return PyBool(PyInt_AS_C(obj) != 0);
        if(tp == _tp_float) return PyBool(PyFloat_AS_C(obj) != 0.0);
        PyVarOrNull len_fn = getAttr(obj, "__len__", false);
        if(len_fn != nullptr){
            PyVar ret = call(len_fn, {});
            return PyBool(PyInt_AS_C(ret) > 0);
        }
        return True;
    }

    PyVar fastCall(const PyVar& obj, const _Str& name, PyVarList args){
        PyVar cls = obj->attribs[__class__];
        while(cls != None) {
            auto it = cls->attribs.find(name);
            if(it != cls->attribs.end()){
                return call(it->second, args);
            }
            cls = cls->attribs[__base__];
        }
        attributeError(obj, name);
        return nullptr;
    }

    PyVar call(PyVar callable, PyVarList args, bool opCall=false){
        if(callable->isType(_tp_type)){
            auto it = callable->attribs.find(__new__);
            PyVar obj;
            if(it != callable->attribs.end()){
                obj = call(it->second, args);
            }else{
                obj = newObject(callable, (_Int)-1);
            }
            if(obj->isType(callable)){
                PyVarOrNull init_fn = getAttr(obj, __init__, false);
                if (init_fn != nullptr) call(init_fn, args);
            }
            return obj;
        }

        if(callable->isType(_tp_bounded_method)){
            auto& bm = PyBoundedMethod_AS_C(callable);
            args.insert(args.begin(), bm.obj);
            callable = bm.method;
        }
        
        if(callable->isType(_tp_native_function)){
            auto f = std::get<_CppFunc>(callable->_native);
            return f(this, args);
        } else if(callable->isType(_tp_function)){
            const _Func& fn = PyFunction_AS_C(callable);
            PyVarDict locals;
            int i = 0;
            for(const auto& name : fn->args){
                if(i < args.size()) {
                    locals[name] = args[i++];
                }else{
                    typeError("missing positional argument '" + name + "'");
                }
            }
            // handle *args
            if(!fn->starredArg.empty()){
                PyVarList vargs;
                while(i < args.size()) vargs.push_back(args[i++]);
                locals[fn->starredArg] = PyTuple(vargs);
            }
            // handle keyword arguments
            for(const auto& [name, value] : fn->kwArgs){
                if(i < args.size()) {
                    locals[name] = args[i++];
                }else{
                    locals[name] = value;
                }
            }

            if(i < args.size()) typeError("too many arguments");

            auto it_m = callable->attribs.find(__module__);
            PyVar _module = it_m != callable->attribs.end() ? it_m->second : topFrame()->_module;
            if(opCall){
                __pushNewFrame(fn->code, _module, locals);
                return __py2py_call_signal;
            }
            return _exec(fn->code, _module, locals);
        }
        typeError("'" + callable->getTypeName() + "' object is not callable");
        return None;
    }

    inline PyVar call(const PyVar& obj, const _Str& func, PyVarList args){
        return call(getAttr(obj, func), args);
    }

    PyVarOrNull exec(const _Code& code, PyVar _module=nullptr){
        if(_module == nullptr) _module = _main;
        try {
            return _exec(code, _module);
        } catch (const std::exception& e) {
            if(const _Error* _ = dynamic_cast<const _Error*>(&e)){
                _stderr(this, e.what());
            }else{
                auto re = RuntimeError("UnexpectedError", e.what(), _cleanErrorAndGetSnapshots());
                _stderr(this, re.what());
            }
            _stderr(this, "\n");
            return nullptr;
        }
    }

    Frame* __pushNewFrame(const _Code& code, PyVar _module, const PyVarDict& locals){
        if(code == nullptr) UNREACHABLE();
        if(callstack.size() > maxRecursionDepth){
            throw RuntimeError("RecursionError", "maximum recursion depth exceeded", _cleanErrorAndGetSnapshots());
        }
        Frame* frame = new Frame(code.get(), _module, locals);
        callstack.push(std::unique_ptr<Frame>(frame));
        return frame;
    }

    PyVar _exec(const _Code& code, PyVar _module, const PyVarDict& locals={}){
        Frame* frame = __pushNewFrame(code, _module, locals);
        Frame* frameBase = frame;
        PyVar ret = nullptr;

        while(true){
            ret = runFrame(frame);
            if(ret != __py2py_call_signal){
                if(frame == frameBase){         // [ frameBase<- ]
                    break;
                }else{
                    callstack.pop();
                    frame = callstack.top().get();
                    frame->push(ret);
                }
            }else{
                frame = callstack.top().get();  // [ frameBase, newFrame<- ]
            }
        }

        callstack.pop();
        return ret;
    }

    PyVar newUserClassType(_Str name, PyVar base){
        PyVar obj = newClassType(name, base);
        setAttr(obj, "__name__", PyStr(name));
        _types.erase(name);
        return obj;
    }

    PyVar newClassType(_Str name, PyVar base=nullptr) {
        if(base == nullptr) base = _tp_object;
        PyVar obj = std::make_shared<PyObject>((_Int)0);
        setAttr(obj, __class__, _tp_type);
        setAttr(obj, __base__, base);
        _types[name] = obj;
        return obj;
    }

    PyVar newObject(PyVar type, _Value _native) {
        __checkType(type, _tp_type);
        PyVar obj = std::make_shared<PyObject>(_native);
        setAttr(obj, __class__, type);
        return obj;
    }

    PyVar newModule(_Str name, bool saveToPath=true) {
        PyVar obj = newObject(_tp_module, (_Int)-2);
        setAttr(obj, "__name__", PyStr(name));
        if(saveToPath) _modules[name] = obj;
        return obj;
    }

    PyVarOrNull getAttr(const PyVar& obj, const _Str& name, bool throw_err=true) {
        auto it = obj->attribs.find(name);
        if(it != obj->attribs.end()) return it->second;

        PyVar cls = obj->attribs[__class__];
        while(cls != None) {
            it = cls->attribs.find(name);
            if(it != cls->attribs.end()){
                PyVar valueFromCls = it->second;
                if(valueFromCls->isType(_tp_function) || valueFromCls->isType(_tp_native_function)){
                    return PyBoundedMethod({obj, valueFromCls});
                }else{
                    return valueFromCls;
                }
            }
            cls = cls->attribs[__base__];
        }
        if(throw_err) attributeError(obj, name);
        return nullptr;
    }

    inline void setAttr(PyVar& obj, const _Str& name, PyVar value) {
        obj->attribs[name] = value;
    }

    void bindMethod(_Str typeName, _Str funcName, _CppFunc fn) {
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
        __checkType(module, _tp_module);
        PyVar func = PyNativeFunction(fn);
        setAttr(module, funcName, func);
    }

    bool isInstance(PyVar obj, PyVar type){
        __checkType(type, _tp_type);
        PyVar t = obj->attribs[__class__];
        while (t != None){
            if (t == type) return true;
            t = t->attribs[__base__];
        }
        return false;
    }

    inline bool isIntOrFloat(const PyVar& obj){
        return obj->isType(_tp_int) || obj->isType(_tp_float);
    }

    inline bool isIntOrFloat(const PyVar& obj1, const PyVar& obj2){
        return isIntOrFloat(obj1) && isIntOrFloat(obj2);
    }

    _Float numToFloat(const PyVar& obj){
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
    PyVar _tp_slice, _tp_range, _tp_module, _tp_pointer;

    __DEF_PY_POOL(Int, _Int, _tp_int, 256);
    __DEF_PY_AS_C(Int, _Int, _tp_int)
    __DEF_PY_POOL(Float, _Float, _tp_float, 256);
    __DEF_PY_AS_C(Float, _Float, _tp_float)
    __DEF_PY_POOL(Pointer, _Pointer, _tp_pointer, 512)
    __DEF_PY_AS_C(Pointer, _Pointer, _tp_pointer)

    DEF_NATIVE(Str, _Str, _tp_str)
    DEF_NATIVE(List, PyVarList, _tp_list)
    DEF_NATIVE(Tuple, PyVarList, _tp_tuple)
    DEF_NATIVE(Function, _Func, _tp_function)
    DEF_NATIVE(NativeFunction, _CppFunc, _tp_native_function)
    DEF_NATIVE(Iter, std::shared_ptr<_Iterator>, _tp_native_iterator)
    DEF_NATIVE(BoundedMethod, _BoundedMethod, _tp_bounded_method)
    DEF_NATIVE(Range, _Range, _tp_range)
    DEF_NATIVE(Slice, _Slice, _tp_slice)
    
    inline bool PyBool_AS_C(PyVar obj){return obj == True;}
    inline PyVar PyBool(bool value){return value ? True : False;}

    void initializeBuiltinClasses(){
        _tp_object = std::make_shared<PyObject>((_Int)0);
        _tp_type = std::make_shared<PyObject>((_Int)0);

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
        _tp_pointer = newClassType("_pointer");

        newClassType("NoneType");
        
        _tp_function = newClassType("function");
        _tp_native_function = newClassType("_native_function");
        _tp_native_iterator = newClassType("_native_iterator");
        _tp_bounded_method = newClassType("_bounded_method");

        this->None = newObject(_types["NoneType"], (_Int)0);
        this->True = newObject(_tp_bool, true);
        this->False = newObject(_tp_bool, false);
        this->builtins = newModule("builtins");
        this->_main = newModule("__main__", false);

        setAttr(_tp_type, __base__, _tp_object);
        setAttr(_tp_type, __class__, _tp_type);
        setAttr(_tp_object, __base__, None);
        setAttr(_tp_object, __class__, _tp_type);
        
        for (auto& [name, type] : _types) {
            setAttr(type, "__name__", PyStr(name));
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
        typeError("unhashable type: " + obj->getTypeName());
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
                snapshots.push(callstack.top()->errorSnapshot());
            }
            callstack.pop();
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
        _error("AttributeError", "type '" + obj->getTypeName() + "' has no attribute '" + name + "'");
    }

    inline void __checkType(const PyVar& obj, const PyVar& type){
        if(!obj->isType(type)) typeError("expected '" + type->getName() + "', but got '" + obj->getTypeName() + "'");
    }

    inline void __checkArgSize(const PyVarList& args, int size, bool method=false){
        if(args.size() == size) return;
        if(method) typeError(args.size()>size ? "too many arguments" : "too few arguments");
        else typeError("expected " + std::to_string(size) + " arguments, but got " + std::to_string(args.size()));
    }

    void _assert(bool val, const _Str& msg){
        if (!val) _error("AssertionError", msg);
    }
};

/***** Pointers' Impl *****/

PyVar NamePointer::get(VM* vm, Frame* frame) const{
    auto it = frame->f_locals.find(name);
    if(it != frame->f_locals.end()) return it->second;
    it = frame->f_globals().find(name);
    if(it != frame->f_globals().end()) return it->second;
    it = vm->builtins->attribs.find(name);
    if(it != vm->builtins->attribs.end()) return it->second;
    vm->nameError(name);
    return nullptr;
}

void NamePointer::set(VM* vm, Frame* frame, PyVar val) const{
    switch(scope) {
        case NAME_LOCAL: frame->f_locals[name] = val; break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals.count(name) > 0){
                frame->f_locals[name] = val;
            }else{
                frame->f_globals()[name] = val;
            }
        } break;
        default: UNREACHABLE();
    }
}

void NamePointer::del(VM* vm, Frame* frame) const{
    switch(scope) {
        case NAME_LOCAL: {
            if(frame->f_locals.count(name) > 0){
                frame->f_locals.erase(name);
            }else{
                vm->nameError(name);
            }
        } break;
        case NAME_GLOBAL:
        {
            if(frame->f_locals.count(name) > 0){
                frame->f_locals.erase(name);
            }else{
                if(frame->f_globals().count(name) > 0){
                    frame->f_globals().erase(name);
                }else{
                    vm->nameError(name);
                }
            }
        } break;
        default: UNREACHABLE();
    }
}

PyVar AttrPointer::get(VM* vm, Frame* frame) const{
    return vm->getAttr(obj, attr->name);
}

void AttrPointer::set(VM* vm, Frame* frame, PyVar val) const{
    vm->setAttr(obj, attr->name, val);
}

void AttrPointer::del(VM* vm, Frame* frame) const{
    vm->typeError("cannot delete attribute");
}

PyVar IndexPointer::get(VM* vm, Frame* frame) const{
    return vm->call(obj, __getitem__, {index});
}

void IndexPointer::set(VM* vm, Frame* frame, PyVar val) const{
    vm->call(obj, __setitem__, {index, val});
}

void IndexPointer::del(VM* vm, Frame* frame) const{
    vm->call(obj, __delitem__, {index});
}

PyVar CompoundPointer::get(VM* vm, Frame* frame) const{
    PyVarList args(pointers.size());
    for (int i = 0; i < pointers.size(); i++) {
        args[i] = pointers[i]->get(vm, frame);
    }
    return vm->PyTuple(args);
}

void CompoundPointer::set(VM* vm, Frame* frame, PyVar val) const{
    if(!val->isType(vm->_tp_tuple) && !val->isType(vm->_tp_list)){
        vm->typeError("only tuple or list can be unpacked");
    }
    const PyVarList& args = std::get<PyVarList>(val->_native);
    if(args.size() > pointers.size()) vm->valueError("too many values to unpack");
    if(args.size() < pointers.size()) vm->valueError("not enough values to unpack");
    for (int i = 0; i < pointers.size(); i++) {
        pointers[i]->set(vm, frame, args[i]);
    }
}

void CompoundPointer::del(VM* vm, Frame* frame) const{
    for (auto& ptr : pointers) ptr->del(vm, frame);
}

/***** Frame's Impl *****/
inline PyVar Frame::__deref_pointer(VM* vm, PyVar v){
    if(v->isType(vm->_tp_pointer)) v = vm->PyPointer_AS_C(v)->get(vm, this);
    return v;
}

/***** Iterators' Impl *****/
PyVar RangeIterator::next(){
    PyVar val = vm->PyInt(current);
    current += r.step;
    return val;
}

PyVar StringIterator::next(){
    return vm->PyStr(str->u8_getitem(index++));
}