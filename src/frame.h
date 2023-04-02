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
    const uint64_t id;
    std::vector<std::pair<int, std::vector<PyObject*>>> s_try_block;
    const NameDict* names[5];     // name resolution array, zero terminated

    NameDict& f_locals() noexcept { return *_locals; }
    NameDict& f_globals() noexcept { return _module->attr(); }

    Frame(const CodeObject_& co, PyObject* _module, NameDict_ _locals=nullptr, NameDict_ _closure=nullptr)
            : co(co.get()), _module(_module), _locals(_locals), id(kFrameGlobalId++) {
        memset(names, 0, sizeof(names));
        int i = 0;
        if(_locals != nullptr) names[i++] = _locals.get();
        if(_closure != nullptr) names[i++] = _closure.get();
        names[i++] = &_module->attr();
        // names[i++] = builtins
    }

    const Bytecode& next_bytecode() {
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

    PyObject*& top_2(){
#if DEBUG_EXTRA_CHECK
        if(_data.size() < 3) throw std::runtime_error("_data.size() < 3");
#endif
        return _data[_data.size()-3];
    }

    template<typename T>
    void push(T&& obj){ _data.push_back(std::forward<T>(obj)); }

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
        if(co->blocks[i].type == FOR_LOOP) _pop();
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

    Args top_n_reversed(int n){
        Args v(n);
        for(int i=0; i<n; i++) v[i] = _data[_data.size()-1-i];
        return v;
    }

    void pop_n(int n){
        _data.resize(_data.size()-n);
    }

    void _mark() const {
        for(PyObject* obj : _data) OBJ_MARK(obj);
        OBJ_MARK(_module);

        int i = 0;  // names[0] is ensured to be non-null
        do{
            names[i++]->_mark();
        }while(names[i] != nullptr);

        for(auto& p : s_try_block){
            for(PyObject* obj : p.second) OBJ_MARK(obj);
        }
        co->_mark();
    }
};

}; // namespace pkpy