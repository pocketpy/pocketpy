#pragma once

#include "codeobject.h"
#include "vector.h"

namespace pkpy{

static THREAD_LOCAL uint64_t kFrameGlobalId = 0;

using ValueStack = small_vector<PyObject*, 6>;

struct Frame {
    ValueStack _data;
    int _ip = -1;
    int _next_ip = 0;

    const CodeObject* co;
    PyObject* _module;
    NameDict_ _locals;
    NameDict_ _closure;
    const uint64_t id;
    std::vector<std::pair<int, ValueStack>> s_try_block;
    const NameDict* names[5];     // name resolution array, zero terminated

    NameDict& f_locals() noexcept { return *_locals; }
    NameDict& f_globals() noexcept { return _module->attr(); }

    Frame(const CodeObject_& co, PyObject* _module, PyObject* builtins, NameDict_ _locals=nullptr, NameDict_ _closure=nullptr)
            : co(co.get()), _module(_module), _locals(_locals), _closure(_closure), id(kFrameGlobalId++) {
        memset(names, 0, sizeof(names));
        int i = 0;
        if(_locals != nullptr) names[i++] = _locals.get();
        if(_closure != nullptr) names[i++] = _closure.get();
        names[i++] = &_module->attr();  // borrowed reference
        if(builtins != nullptr){
            names[i++] = &builtins->attr(); // borrowed reference
        }
    }

    const Bytecode& next_bytecode() {
        _ip = _next_ip++;
        return co->codes[_ip];
    }

    Str snapshot(){
        int line = co->codes[_ip].line;
        return co->src->snapshot(line);
    }

    Str stack_info(){
        StrStream ss;
        ss << id << " [";
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

    void on_try_block_enter(){
        s_try_block.emplace_back(co->codes[_ip].block, _data);
    }

    void on_try_block_exit(){
        s_try_block.pop_back();
    }

    bool jump_to_exception_handler(){
        if(s_try_block.empty()) return false;
        PyObject* obj = popx();
        auto& p = s_try_block.back();
        _data = std::move(p.second);
        _data.push_back(obj);
        _next_ip = co->blocks[p.first].end;
        on_try_block_exit();
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) pop();
        else if(co->blocks[i].type == TRY_EXCEPT) on_try_block_exit();
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
        for(PyObject* obj : _data) OBJ_MARK(obj);
        OBJ_MARK(_module);
        if(_locals != nullptr) _locals->_gc_mark();
        if(_closure != nullptr) _closure->_gc_mark();
        for(auto& p : s_try_block){
            for(PyObject* obj : p.second) OBJ_MARK(obj);
        }
        co->_gc_mark();
    }
};

}; // namespace pkpy