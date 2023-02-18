#pragma once

#include "codeobject.h"

static THREAD_LOCAL i64 kFrameGlobalId = 0;

struct Frame {
    std::vector<PyVar> _data;
    int _ip = -1;
    int _next_ip = 0;

    const CodeObject_ co;
    PyVar _module;
    pkpy::shared_ptr<pkpy::NameDict> _locals;
    const i64 id;
    std::stack<std::pair<int, std::vector<PyVar>>> s_try_block;

    inline pkpy::NameDict& f_locals() noexcept { return *_locals; }
    inline pkpy::NameDict& f_globals() noexcept { return _module->attr(); }

    Frame(const CodeObject_ co, PyVar _module, pkpy::shared_ptr<pkpy::NameDict> _locals)
        : co(co), _module(_module), _locals(_locals), id(kFrameGlobalId++) { }

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

    inline PyVar pop(){
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
        PyVar v = std::move(_data.back());
        _data.pop_back();
        return v;
    }

    inline void _pop(){
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
        _data.pop_back();
    }

    inline void try_deref(VM*, PyVar&);

    inline PyVar pop_value(VM* vm){
        PyVar value = pop();
        try_deref(vm, value);
        return value;
    }

    inline PyVar top_value(VM* vm){
        PyVar value = top();
        try_deref(vm, value);
        return value;
    }

    inline PyVar& top(){
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
        return _data.back();
    }

    inline PyVar top_value_offset(VM* vm, int n){
        PyVar value = _data[_data.size() + n];
        try_deref(vm, value);
        return value;
    }

    template<typename T>
    inline void push(T&& obj){ _data.push_back(std::forward<T>(obj)); }

    inline void jump_abs(int i){ _next_ip = i; }
    inline void jump_rel(int i){ _next_ip += i; }

    inline void on_try_block_enter(){
        s_try_block.push(std::make_pair(co->codes[_ip].block, _data));
    }

    inline void on_try_block_exit(){
        s_try_block.pop();
    }

    bool jump_to_exception_handler(){
        if(s_try_block.empty()) return false;
        PyVar obj = pop();
        auto& p = s_try_block.top();
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

    pkpy::Args pop_n_values_reversed(VM* vm, int n){
        pkpy::Args v(n);
        for(int i=n-1; i>=0; i--){
            v[i] = pop();
            try_deref(vm, v[i]);
        }
        return v;
    }

    pkpy::Args pop_n_reversed(int n){
        pkpy::Args v(n);
        for(int i=n-1; i>=0; i--) v[i] = pop();
        return v;
    }
};