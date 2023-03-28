#pragma once

#include "codeobject.h"

namespace pkpy{

static THREAD_LOCAL uint64_t kFrameGlobalId = 0;

struct Frame {
    std::vector<PyObject*> _data;
    int _ip = -1;
    int _next_ip = 0;

    const CodeObject* co;
    PyObject* _module;
    NameDict_ _locals;
    NameDict_ _closure;
    const uint64_t id;
    std::vector<std::pair<int, std::vector<PyObject*>>> s_try_block;

    inline NameDict& f_locals() noexcept { return _locals != nullptr ? *_locals : _module->attr(); }
    inline NameDict& f_globals() noexcept { return _module->attr(); }

    inline PyObject** f_closure_try_get(StrName name) noexcept {
        if(_closure == nullptr) return nullptr;
        return _closure->try_get(name);
    }

    Frame(const CodeObject_& co,
        PyObject* _module,
        const NameDict_& _locals=nullptr,
        const NameDict_& _closure=nullptr)
            : co(co.get()), _module(_module), _locals(_locals), _closure(_closure), id(kFrameGlobalId++) { }

    inline const Bytecode& next_bytecode() {
        _ip = _next_ip++;
        return co->codes[_ip];
    }

    Str snapshot(){
        int line = co->codes[_ip].line;
        return co->src->snapshot(line);
    }

    // Str stack_info(){
    //     StrStream ss;
    //     ss << "[";
    //     for(int i=0; i<_data.size(); i++){
    //         ss << OBJ_TP_NAME(_data[i]);
    //         if(i != _data.size()-1) ss << ", ";
    //     }
    //     ss << "]";
    //     return ss.str();
    // }

    inline bool has_next_bytecode() const {
        return _next_ip < co->codes.size();
    }

    inline PyObject* pop(){
#if PK_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        PyObject* v = _data.back();
        _data.pop_back();
        return v;
    }

    inline void _pop(){
#if PK_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        _data.pop_back();
    }

    void try_deref(VM*, PyObject*&);

    inline PyObject* pop_value(VM* vm){
        PyObject* value = pop();
        try_deref(vm, value);
        return value;
    }

    inline PyObject* top_value(VM* vm){
        PyObject* value = top();
        try_deref(vm, value);
        return value;
    }

    inline PyObject*& top(){
#if PK_EXTRA_CHECK
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
#endif
        return _data.back();
    }

    inline PyObject*& top_1(){
#if PK_EXTRA_CHECK
        if(_data.size() < 2) throw std::runtime_error("_data.size() < 2");
#endif
        return _data[_data.size()-2];
    }

    template<typename T>
    inline void push(T&& obj){ _data.push_back(std::forward<T>(obj)); }

    inline void jump_abs(int i){ _next_ip = i; }
    inline void jump_rel(int i){ _next_ip += i; }

    inline void on_try_block_enter(){
        s_try_block.emplace_back(co->codes[_ip].block, _data);
    }

    inline void on_try_block_exit(){
        s_try_block.pop_back();
    }

    bool jump_to_exception_handler(){
        if(s_try_block.empty()) return false;
        PyObject* obj = pop();
        auto& p = s_try_block.back();
        _data = std::move(p.second);
        _data.push_back(obj);
        _next_ip = co->blocks[p.first].end;
        on_try_block_exit();
        return true;
    }

    int _exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _pop();
        else if(co->blocks[i].type == TRY_EXCEPT) on_try_block_exit();
        return co->blocks[i].parent;
    }

    void jump_abs_safe(int target){
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

    Args pop_n_values_reversed(VM* vm, int n){
        Args v(n);
        for(int i=n-1; i>=0; i--){
            v[i] = pop();
            try_deref(vm, v[i]);
        }
        return v;
    }

    Args pop_n_reversed(int n){
        Args v(n);
        for(int i=n-1; i>=0; i--) v[i] = pop();
        return v;
    }
};

}; // namespace pkpy