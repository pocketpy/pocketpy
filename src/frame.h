#pragma once

#include "codeobject.h"
#include "memory.h"
#include "vector.h"

namespace pkpy{

struct FastLocals{
    NameDictInt_ varnames_inv;
    PyObject** a;

    int size() const{
        return varnames_inv->size();
    }

    PyObject*& operator[](int i){ return a[i]; }
    PyObject* operator[](int i) const { return a[i]; }

    FastLocals(): varnames_inv(nullptr), a(nullptr) {}
    FastLocals(std::nullptr_t): varnames_inv(nullptr), a(nullptr) {}

    FastLocals(const CodeObject* co): varnames_inv(co->varnames_inv){
        size_t size = this->size() * sizeof(void*);
        int* counter = (int*)pool128.alloc(sizeof(int) + size);
        *counter = 1;
        a = (PyObject**)(counter + 1);
        memset(a, 0, this->size() * sizeof(void*));
    }

    PyObject* try_get(StrName name){
        if(!is_valid()) return nullptr;
        int index = varnames_inv->try_get(name);
        if(index == -1) return nullptr;
        return a[index];
    }

    bool try_set(StrName name, PyObject* value){
        if(!is_valid()) return false;
        int index = varnames_inv->try_get(name);
        if(index == -1) return false;
        a[index] = value;
        return true;
    }

    FastLocals(const FastLocals& other){
        varnames_inv = other.varnames_inv;
        a = other.a;
        _inc_counter();
    }

    FastLocals(FastLocals&& other) noexcept{
        varnames_inv = std::move(other.varnames_inv);
        a = other.a;
        other.a = nullptr;
    }

    FastLocals& operator=(const FastLocals& other){
        _dec_counter();
        varnames_inv = other.varnames_inv;
        a = other.a;
        _inc_counter();
        return *this;
    }

    FastLocals& operator=(FastLocals&& other) noexcept{
        _dec_counter();
        varnames_inv = std::move(other.varnames_inv);
        a = other.a;
        other.a = nullptr;
        return *this;
    }

    bool is_valid() const{ return a != nullptr; }

    void _inc_counter(){
        if(a == nullptr) return;
        int* counter = (int*)a - 1;
        (*counter)++;
    }

    void _dec_counter(){
        if(a == nullptr) return;
        int* counter = (int*)a - 1;
        (*counter)--;
        if(*counter == 0){
            pool128.dealloc(counter);
        }
    }

    ~FastLocals(){
        _dec_counter();
    }

    void _gc_mark() const{
        if(a == nullptr) return;
        for(int i=0; i<size(); i++){
            if(a[i] != nullptr) OBJ_MARK(a[i]);
        }
    }
};

struct Function{
    FuncDecl_ decl;
    PyObject* _module;
    FastLocals _closure;
};

template<> inline void gc_mark<Function>(Function& t){
    t.decl->_gc_mark();
    if(t._module != nullptr) OBJ_MARK(t._module);
    t._closure._gc_mark();
}

struct ValueStack {
    PyObject** _begin;
    PyObject** _sp;

    ValueStack(int n=16): _begin((PyObject**)pool128.alloc(n * sizeof(void*))), _sp(_begin) { }

    PyObject*& top(){ return _sp[-1]; }
    PyObject* top() const { return _sp[-1]; }
    PyObject*& second(){ return _sp[-2]; }
    PyObject* second() const { return _sp[-2]; }
    PyObject*& peek(int n){ return _sp[-n]; }
    PyObject* peek(int n) const { return _sp[-n]; }
    void push(PyObject* v){ *_sp++ = v; }
    void pop(){ --_sp; }
    PyObject* popx(){ return *--_sp; }
    ArgsView view(int n){ return ArgsView(_sp-n, _sp); }
    void shrink(int n){ _sp -= n; }
    int size() const { return _sp - _begin; }
    bool empty() const { return _sp == _begin; }
    PyObject** begin() const { return _begin; }
    PyObject** end() const { return _sp; }
    void resize(int n) { _sp = _begin + n; }

    ValueStack(ValueStack&& other) noexcept{
        _begin = other._begin;
        _sp = other._sp;
        other._begin = nullptr;
    }

    ValueStack& operator=(ValueStack&& other) noexcept{
        if(_begin != nullptr) pool128.dealloc(_begin);
        _begin = other._begin;
        _sp = other._sp;
        other._begin = nullptr;
        return *this;
    }

    ~ValueStack(){ if(_begin!=nullptr) pool128.dealloc(_begin); }
};

struct Frame {
    int _ip = -1;
    int _next_ip = 0;
    const CodeObject* co;
    PyObject* _module;

    FastLocals _locals;
    FastLocals _closure;
    ValueStack _s;

    NameDict& f_globals() noexcept { return _module->attr(); }

    Frame(const CodeObject* co, PyObject* _module, FastLocals&& _locals, const FastLocals& _closure)
            : co(co), _module(_module), _locals(std::move(_locals)), _closure(_closure) { }

    Frame(const CodeObject* co, PyObject* _module, const FastLocals& _locals, const FastLocals& _closure)
            : co(co), _module(_module), _locals(_locals), _closure(_closure) { }

    Frame(const CodeObject_& co, PyObject* _module)
            : co(co.get()), _module(_module), _locals(), _closure() { }

    Frame(const Frame& other) = delete;
    Frame& operator=(const Frame& other) = delete;
    Frame(Frame&& other) noexcept = default;
    Frame& operator=(Frame&& other) noexcept = default;

    Bytecode next_bytecode() {
        _ip = _next_ip++;
        return co->codes[_ip];
    }

    Str snapshot(){
        int line = co->lines[_ip];
        return co->src->snapshot(line);
    }

    std::string stack_info(){
        std::stringstream ss;
        ss << this << ": [";
        for(PyObject** t=_s.begin(); t<_s.end(); t++){
            ss << *t;
            if(t != _s.end()-1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    void jump_abs(int i){ _next_ip = i; }
    void jump_rel(int i){ _next_ip += i; }

    bool jump_to_exception_handler(){
        // try to find a parent try block
        int block = co->codes[_ip].block;
        while(block >= 0){
            if(co->blocks[block].type == TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return false;
        PyObject* obj = _s.popx();         // pop exception object
        // get the stack size of the try block (depth of for loops)
        int stack_size = co->blocks[block].for_loop_depth;
        if(_s.size() < stack_size) throw std::runtime_error("invalid stack size");
        _s.resize(stack_size);             // rollback the stack   
        _s.push(obj);                      // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _s.pop();
        return co->blocks[i].parent;
    }

    void jump_abs_break(int target){
        const Bytecode& prev = co->codes[_ip];
        int i = prev.block;
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(i);
        }else{
            const Bytecode& next = co->codes[target];
            while(i>=0 && i!=next.block) i = _exit_block(i);
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
    }

    void _gc_mark() const {
        // do return if this frame has been moved
        if(!_locals.is_valid()) return;
        for(PyObject* obj: _s) OBJ_MARK(obj);
        OBJ_MARK(_module);
        _locals._gc_mark();
        _closure._gc_mark();
        co->_gc_mark();
    }
};

}; // namespace pkpy