#pragma once

#include "codeobject.h"
#include "common.h"
#include "memory.h"
#include "obj.h"
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

    bool contains(StrName name){
        return varnames_inv->contains(name);
    }

    void erase(StrName name){
        if(!is_valid()) return;
        int index = varnames_inv->try_get(name);
        if(index == -1) FATAL_ERROR();
        a[index] = nullptr;
    }

    bool _try_set(StrName name, PyObject* value){
        int index = varnames_inv->try_get(name);
        if(index == -1) return false;
        a[index] = value;
        return true;
    }

    bool try_set(StrName name, PyObject* value){
        if(!is_valid()) return false;
        return _try_set(name, value);
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
    static const size_t MAX_SIZE = 8192;
    // We allocate 512 more bytes to keep `_sp` valid when `is_overflow() == true`.
    PyObject* _begin[MAX_SIZE + 512];
    PyObject** _sp;

    ValueStack(): _sp(_begin) {}

    PyObject*& top(){ return _sp[-1]; }
    PyObject* top() const { return _sp[-1]; }
    PyObject*& second(){ return _sp[-2]; }
    PyObject* second() const { return _sp[-2]; }
    PyObject*& third(){ return _sp[-3]; }
    PyObject* third() const { return _sp[-3]; }
    PyObject*& peek(int n){ return _sp[-n]; }
    PyObject* peek(int n) const { return _sp[-n]; }
    void push(PyObject* v){ *_sp++ = v; }
    void pop(){ --_sp; }
    PyObject* popx(){ return *--_sp; }
    ArgsView view(int n){ return ArgsView(_sp-n, _sp); }
    void shrink(int n){ _sp -= n; }
    int size() const { return _sp - _begin; }
    bool empty() const { return _sp == _begin; }
    PyObject** begin() { return _begin; }
    PyObject** end() { return _sp; }
    void reset(PyObject** sp) {
#if DEBUG_EXTRA_CHECK
        if(sp < _begin || sp > _begin + MAX_SIZE) FATAL_ERROR();
#endif
        _sp = sp;
    }
    void clear() { _sp = _begin; }
    bool is_overflow() const { return _sp >= _begin + MAX_SIZE; }
    
    ValueStack(const ValueStack&) = delete;
    ValueStack(ValueStack&&) = delete;
    ValueStack& operator=(const ValueStack&) = delete;
    ValueStack& operator=(ValueStack&&) = delete;
};

struct Frame {
    int _ip = -1;
    int _next_ip = 0;
    ValueStack* _s;
    PyObject** _sp_base;
    const CodeObject* co;

    PyObject* _module;
    FastLocals _locals;
    PyObject* _callable;

    NameDict& f_globals() noexcept { return _module->attr(); }
    
    PyObject* f_closure_try_get(StrName name){
        if(_callable == nullptr) return nullptr;
        Function& fn = OBJ_GET(Function, _callable);
        return fn._closure.try_get(name);
    }

    Frame(ValueStack* _s, PyObject** _sp_base, const CodeObject* co, PyObject* _module, FastLocals&& _locals, PyObject* _callable)
            : _s(_s), _sp_base(_sp_base), co(co), _module(_module), _locals(std::move(_locals)), _callable(_callable) { }

    Frame(ValueStack* _s, PyObject** _sp_base, const CodeObject* co, PyObject* _module, const FastLocals& _locals, PyObject* _callable)
            : _s(_s), _sp_base(_sp_base), co(co), _module(_module), _locals(_locals), _callable(_callable) { }

    Frame(ValueStack* _s, PyObject** _sp_base, const CodeObject_& co, PyObject* _module)
            : _s(_s), _sp_base(_sp_base), co(co.get()), _module(_module), _locals(), _callable(nullptr) { }

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

    int stack_size() const { return _s->_sp - _sp_base; }
    ArgsView stack_view() const { return ArgsView(_sp_base, _s->_sp); }

    void jump_abs(int i){ _next_ip = i; }
    // void jump_rel(int i){ _next_ip += i; }

    bool jump_to_exception_handler(){
        // try to find a parent try block
        int block = co->codes[_ip].block;
        while(block >= 0){
            if(co->blocks[block].type == TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return false;
        PyObject* obj = _s->popx();         // pop exception object
        // get the stack size of the try block (depth of for loops)
        int _stack_size = co->blocks[block].for_loop_depth;
        if(stack_size() < _stack_size) throw std::runtime_error("invalid stack size");
        _s->reset(_sp_base + _stack_size);             // rollback the stack   
        _s->push(obj);                      // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _s->pop();
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
        // TODO: fix here
        OBJ_MARK(_module);
        _locals._gc_mark();
        if(_callable != nullptr) OBJ_MARK(_callable);
        co->_gc_mark();
    }
};

}; // namespace pkpy