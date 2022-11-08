#pragma once

#include "codeobject.h"
#include "iter.h"

#define DEF_NATIVE(type, ctype, ptype)                          \
    inline PyVar Py##type(ctype value) {                        \
        return newObject(ptype, value);                         \
    }                                                           \
                                                                \
    inline ctype& Py##type##_AS_C(const PyVar& obj) {           \
        __checkType(obj, ptype);                                \
        return std::get<ctype>(obj->_native);                   \
    }

#define BINARY_XXX(i)      \
          {PyVar rhs = frame->popValue(this);   \
          PyVar lhs = frame->popValue(this);    \
          frame->push(fastCall(lhs, BIN_SPECIAL_METHODS[i], {lhs,rhs}));}

#define COMPARE_XXX(i)      \
          {PyVar rhs = frame->popValue(this);   \
          PyVar lhs = frame->popValue(this);   \
          frame->push(fastCall(lhs, CMP_SPECIAL_METHODS[i], {lhs,rhs}));}      

// TODO: we should split this into stdout and stderr
typedef void(*PrintFn)(const char*);

class VM{
private:
    std::stack< std::shared_ptr<Frame> > callstack;
public:
    StlDict _types;         // builtin types
    PyVar None, True, False;

    PrintFn printFn = [](auto s){};
    
    PyVar builtins;         // builtins module
    PyVar _main;            // __main__ module
    StlDict _modules;       // 3rd modules

    VM(){
        initializeBuiltinClasses();
    }

    void cleanError(){
        while(!callstack.empty()) callstack.pop();
    }

    void nameError(const _Str& name){
        _error("NameError", "name '" + name + "' is not defined");
    }

    void attributeError(PyVar obj, const _Str& name){
        _error("AttributeError", "type '" + obj->getTypeName() + "' has no attribute '" + name + "'");
    }

    inline void __checkType(const PyVar& obj, const PyVar& type){
        if(!obj->isType(type)){
            _error("TypeError", "expected '" + type->getName() + "', but got '" + obj->getTypeName() + "'");
        }
    }

    PyVar asStr(const PyVar& obj){
        PyVarOrNull str_fn = getAttr(obj, __str__, false);
        if(str_fn != nullptr) return call(str_fn, {});
        return asRepr(obj);
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
        if(tp == _tp_float) return PyBool(PyFloat_AS_C(obj) != 0.0f);
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

    PyVar call(PyVar callable, PyVarList args){
        if(callable->isType(_tp_type)){
            // add type itself as the first argument
            args.insert(args.begin(), callable);
            callable = getAttr(callable, __new__);
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
            _Func fn = PyFunction_AS_C(callable);
            StlDict locals;
            int i = 0;
            for(const auto& name : fn.args){
                if(i < args.size()) {
                    locals[name] = args[i++];
                }else{
                    _error("TypeError", "missing positional argument '" + name + "'");
                }
            }
            // handle *args
            if(!fn.starredArg.empty()){
                PyVarList vargs;
                while(i < args.size()) vargs.push_back(args[i++]);
                locals[fn.starredArg] = PyTuple(vargs);
            }
            // handle keyword arguments
            for(const auto& [name, value] : fn.kwArgs){
                if(i < args.size()) {
                    locals[name] = args[i++];
                }else{
                    locals[name] = value;
                }
            }

            if(i < args.size()) _error("TypeError", "too many arguments");

            // TODO: handle **kwargs
            return exec(fn.code, locals);
        }
        _error("TypeError", "'" + callable->getTypeName() + "' object is not callable");
        return None;
    }

    inline PyVar call(const PyVar& obj, const _Str& func, PyVarList args){
        return call(getAttr(obj, func), args);
    }
    
    PyVar runFrame(std::shared_ptr<Frame> frame){
        callstack.push(frame);
        while(!frame->isEnd()){
            const ByteCode& byte = frame->readCode();
            //printf("%s (%d) stack_size: %d\n", OP_NAMES[byte.op], byte.arg, frame->stackSize());

            switch (byte.op)
            {
            case OP_NO_OP: break;       // do nothing
            case OP_LOAD_CONST: frame->push(frame->code->co_consts[byte.arg]); break;
            case OP_LOAD_NAME_PTR: {
                frame->push(PyPointer(frame->code->co_names[byte.arg]));
            } break;
            case OP_STORE_NAME_PTR: {
                const auto& p = frame->code->co_names[byte.arg];
                p->set(this, frame.get(), frame->popValue(this));
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
                p->set(this, frame.get(), obj);
            } break;
            case OP_DELETE_PTR: {
                _Pointer p = PyPointer_AS_C(frame->__pop());
                p->del(this, frame.get());
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
                    frame->f_globals->operator[](fn.name) = obj;
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
                        setAttr(cls, f.name, fn);
                    }
                    frame->f_globals->operator[](clsName) = cls;
                } break;
            case OP_RETURN_VALUE:
                {
                    PyVar ret = frame->popValue(this);
                    callstack.pop();
                    return ret;
                } break;
            case OP_PRINT_EXPR:
                {
                    const PyVar& expr = frame->topValue(this);
                    if(expr == None) break;
                    printFn(PyStr_AS_C(asRepr(expr)));
                    printFn("\n");
                } break;
            case OP_POP_TOP: frame->popValue(this); break;
            case OP_BINARY_OP: BINARY_XXX(byte.arg) break;
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
                    PyVar right = frame->popValue(this);
                    PyVar left = frame->popValue(this);
                    bool ret_c = PyBool_AS_C(call(right, __contains__, {left}));
                    if(byte.arg == 1) ret_c = !ret_c;
                    frame->push(PyBool(ret_c));
                } break;
            case OP_UNARY_NEGATIVE:
                {
                    PyVar obj = frame->popValue(this);
                    frame->push(call(obj, __neg__, {}));
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
                    if(!PyBool_AS_C(expr)) _error("AssertionError", "assertion failed");
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
                    frame->push(call(callable, args));
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
                        _error("TypeError", "'" + obj->getTypeName() + "' object is not iterable");
                    }
                } break;
            case OP_FOR_ITER:
                {
                    const PyVar& iter = frame->topValue(this);
                    auto& it = PyIter_AS_C(iter);
                    if(it->hasNext()){
                        it->var->set(this, frame.get(), it->next());
                    }
                    else{
                        frame->popValue(this);
                        frame->jumpTo(byte.arg);
                    }
                } break;
            case OP_JUMP_IF_FALSE_OR_POP:
                {
                    const PyVar& expr = frame->topValue(this);
                    if(!PyBool_AS_C(asBool(expr))) frame->jumpTo(byte.arg);
                    else frame->popValue(this);
                } break;
            case OP_JUMP_IF_TRUE_OR_POP:
                {
                    const PyVar& expr = frame->topValue(this);
                    if(PyBool_AS_C(asBool(expr))) frame->jumpTo(byte.arg);
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
                    if(it == _modules.end()){
                        _error("ImportError", "module '" + name + "' not found");
                    }else{
                        frame->push(it->second);
                    }
                } break;
            default:
                _error("SystemError", _Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
                break;
            }
        }

        if(frame->code->mode == EVAL_MODE) {
            if(frame->stackSize() != 1) {
                _error("SystemError", "stack size is not 1 in EVAL_MODE");
            }
            return frame->popValue(this);
        }

        if(frame->stackSize() != 0) _error("SystemError", "stack not empty in EXEC_MODE");
        callstack.pop();
        return None;
    }

    PyVar exec(const _Code& code, const StlDict& locals={}, PyVar _module=nullptr){
        if(_module == nullptr) _module = _main;
        auto frame = std::make_shared<Frame>(
            code.get(),
            locals,
            &_module->attribs
        );
        return runFrame(frame);
    }

    void _assert(bool val, const _Str& msg){
        if (!val) _error("AssertionError", msg);
    }

    void _error(const _Str& name, const _Str& msg){
        _StrStream ss;
        auto frame = callstack.top();
        ss << "Traceback (most recent call last):" << std::endl;
        ss << "  File '" << frame->code->co_filename << "', line ";
        ss << frame->currentLine() << '\n' << name << ": " << msg;
        throw std::runtime_error(ss.str());
    }

    PyVar newUserClassType(_Str name, PyVar base){
        PyVar obj = newClassType(name, base);
        setAttr(obj, "__name__", PyStr(name));
        _types.erase(name);
        return obj;
    }

    PyVar newClassType(_Str name, PyVar base=nullptr) {
        if(base == nullptr) base = _tp_object;
        PyVar obj = std::make_shared<PyObject>(0);
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

    PyVar newModule(_Str name) {
        PyVar obj = newObject(_tp_module, 0);
        setAttr(obj, "__name__", PyStr(name));
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
                    if(name == __new__) return valueFromCls;
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

    float numToFloat(const PyVar& obj){
        if (obj->isType(_tp_int)){
            return (float)PyInt_AS_C(obj);
        }else if(obj->isType(_tp_float)){
            return PyFloat_AS_C(obj);
        }
        UNREACHABLE();
    }

    int normalizedIndex(int index, int size){
        if(index < 0) index += size;
        if(index < 0 || index >= size){
            _error("IndexError", "index out of range, " + std::to_string(index) + " not in [0, " + std::to_string(size) + ")");
        }
        return index;
    }

    // for quick access
    PyVar _tp_object, _tp_type, _tp_int, _tp_float, _tp_bool, _tp_str;
    PyVar _tp_list, _tp_tuple;
    PyVar _tp_function, _tp_native_function, _tp_native_iterator, _tp_bounded_method;
    PyVar _tp_slice, _tp_range, _tp_module, _tp_pointer;

    DEF_NATIVE(Int, int, _tp_int)
    DEF_NATIVE(Float, float, _tp_float)
    DEF_NATIVE(Str, _Str, _tp_str)
    DEF_NATIVE(List, PyVarList, _tp_list)
    DEF_NATIVE(Tuple, PyVarList, _tp_tuple)
    DEF_NATIVE(Function, _Func, _tp_function)
    DEF_NATIVE(NativeFunction, _CppFunc, _tp_native_function)
    DEF_NATIVE(Iter, std::shared_ptr<_Iterator>, _tp_native_iterator)
    DEF_NATIVE(BoundedMethod, BoundedMethod, _tp_bounded_method)
    DEF_NATIVE(Range, _Range, _tp_range)
    DEF_NATIVE(Slice, _Slice, _tp_slice)
    DEF_NATIVE(Pointer, _Pointer, _tp_pointer)
    
    inline bool PyBool_AS_C(PyVar obj){return obj == True;}
    inline PyVar PyBool(bool value){return value ? True : False;}

    void initializeBuiltinClasses(){
        _tp_object = std::make_shared<PyObject>(0);
        _tp_type = std::make_shared<PyObject>(0);

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

        this->None = newObject(_types["NoneType"], 0);
        this->True = newObject(_tp_bool, true);
        this->False = newObject(_tp_bool, false);
        this->builtins = newModule("__builtins__");
        this->_main = newModule("__main__");
        
        setAttr(_tp_type, __base__, _tp_object);
        setAttr(_tp_type, __class__, _tp_type);
        setAttr(_tp_object, __base__, None);
        setAttr(_tp_object, __class__, _tp_type);
        
        for (auto& [name, type] : _types) {
            setAttr(type, "__name__", PyStr(name));
        }

        std::vector<_Str> publicTypes = {"type", "object", "bool", "int", "float", "str", "list", "tuple", "range"};
        for (auto& name : publicTypes) {
            setAttr(builtins, name, _types[name]);
        }
    }

    int hash(const PyVar& obj){
        if (obj->isType(_tp_int)) return PyInt_AS_C(obj);
        if (obj->isType(_tp_bool)) return PyBool_AS_C(obj) ? 1 : 0;
        if (obj->isType(_tp_float)){
            float val = PyFloat_AS_C(obj);
            return (int)std::hash<float>()(val);
        }
        if (obj->isType(_tp_str)) return PyStr_AS_C(obj).hash();
        if (obj->isType(_tp_type)) return (int64_t)obj.get();
        _error("TypeError", "unhashable type: " + obj->getTypeName());
        return 0;
    }

    void registerCompiledModule(_Str name, _Code code){
        PyVar _m = newModule(name);
        exec(code, {}, _m);
        _modules[name] = _m;
    }
};

/**************** Pointers' Impl ****************/

PyVar NamePointer::get(VM* vm, Frame* frame) const{
    auto it = frame->f_locals.find(name);
    if(it != frame->f_locals.end()) return it->second;
    it = frame->f_globals->find(name);
    if(it != frame->f_globals->end()) return it->second;
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
                frame->f_globals->operator[](name) = val;
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
                if(frame->f_globals->count(name) > 0){
                    frame->f_globals->erase(name);
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
    vm->_error("AttributeError", "can't delete attribute");
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
        vm->_error("TypeError", "only tuple or list can be unpacked");
    }
    const PyVarList& args = std::get<PyVarList>(val->_native);
    if(args.size() > pointers.size()) vm->_error("ValueError", "too many values to unpack");
    if(args.size() < pointers.size()) vm->_error("ValueError", "not enough values to unpack");
    for (int i = 0; i < pointers.size(); i++) {
        pointers[i]->set(vm, frame, args[i]);
    }
}

void CompoundPointer::del(VM* vm, Frame* frame) const{
    for (auto& ptr : pointers) ptr->del(vm, frame);
}

/**************** Frame ****************/
inline PyVar Frame::__deref_pointer(VM* vm, PyVar v){
    if(v->isType(vm->_tp_pointer)) v = vm->PyPointer_AS_C(v)->get(vm, this);
    return v;
}