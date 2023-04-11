#pragma once

#include "codeobject.h"
#include "memory.h"
#include "vector.h"

namespace pkpy{

using ValueStack = pod_vector<PyObject*>;

struct FastLocals{
    NameDictInt_ varnames_inv;
    PyObject** a;

    int size() const{ return varnames_inv->size(); }

    PyObject*& operator[](int i){ return a[i]; }
    PyObject* operator[](int i) const { return a[i]; }

    FastLocals(const CodeObject* co): varnames_inv(co->varnames_inv){
        size_t size = co->varnames.size() * sizeof(void*);
        int* counter = (int*)pool128.alloc(sizeof(int) + size);
        *counter = 1;
        a = (PyObject**)(counter + 1);
        memset(a, 0, size);
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

    FastLocals(): varnames_inv(nullptr), a(nullptr) {}

    FastLocals(const FastLocals& other){
        a = other.a;
        inc_counter();
    }

    FastLocals(FastLocals&& other){
        a = other.a;
        other.a = nullptr;
    }

    FastLocals& operator=(const FastLocals& other){
        dec_counter();
        a = other.a;
        inc_counter();
        return *this;
    }

    FastLocals& operator=(FastLocals&& other) noexcept{
        dec_counter();
        a = other.a;
        other.a = nullptr;
        return *this;
    }

    bool is_valid() const{ return a != nullptr; }

    void inc_counter(){
        if(a == nullptr) return;
        int* counter = (int*)a - 1;
        (*counter)++;
    }

    void dec_counter(){
        if(a == nullptr) return;
        int* counter = (int*)a - 1;
        (*counter)--;
        if(*counter == 0){
            pool128.dealloc(counter);
        }
    }

    ~FastLocals(){
        dec_counter();
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

struct Frame {
    ValueStack _data;
    int _ip = -1;
    int _next_ip = 0;
    const CodeObject* co;
    PyObject* _module;

    FastLocals _locals;
    FastLocals _closure;

    NameDict& f_globals() noexcept { return _module->attr(); }

    Frame(const CodeObject* co, PyObject* _module, FastLocals&& _locals, const FastLocals& _closure)
            : co(co), _module(_module), _locals(std::move(_locals)), _closure(_closure) { }

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
        ss << " [";
        for(int i=0; i<_data.size(); i++){
            ss << (i64)_data[i];
            if(i != _data.size()-1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    void pop(){
#if DEBUG_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        _data.pop_back();
    }

    PyObject* popx(){
#if DEBUG_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        PyObject* ret = _data.back();
        _data.pop_back();
        return ret;
    }

    PyObject*& top(){
#if DEBUG_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        return _data.back();
    }

    PyObject*& top_1(){
#if DEBUG_EXTRA_CHECK
        if(_data.size() < 2) throw std::runtime_error("_data.size() < 2");
#endif
        return _data[_data.size()-2];
    }

    PyObject*& top_n(int n){
        n += 1;
#if DEBUG_EXTRA_CHECK
        if(_data.size() < n) throw std::runtime_error("_data.size() < n");
#endif
        return _data[_data.size()-n];
    }

    void push(PyObject* obj){
#if DEBUG_EXTRA_CHECK
        if(obj == nullptr) throw std::runtime_error("obj == nullptr");
#endif
        _data.push_back(obj);
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
        PyObject* obj = popx();         // pop exception object
        // get the stack size of the try block (depth of for loops)
        int stack_size = co->blocks[block].for_loop_depth;
        // std::cout << "stack_size: " << stack_size << std::endl;
        if(_data.size() < stack_size) throw std::runtime_error("invalid stack size");
        _data.resize(stack_size);       // rollback the stack
        _data.push_back(obj);           // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) pop();
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

    Args popx_n_reversed(int n){
        Args v(n);
        for(int i=n-1; i>=0; i--) v[i] = popx();
        return v;
    }

    void pop_n(int n){
        _data.pop_back_n(n);
    }

    void _gc_mark() const {
        // do return if this frame has been moved
        if(_data._data == nullptr) return;
        for(PyObject* obj : _data) OBJ_MARK(obj);
        OBJ_MARK(_module);
        _locals._gc_mark();
        _closure._gc_mark();
        co->_gc_mark();
    }
};

}; // namespace pkpy