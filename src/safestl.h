#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

struct PyObject;
typedef pkpy::shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef PyVar PyVarRef;

namespace pkpy{
class List: public std::vector<PyVar> {
    PyVar& at(size_t) = delete;

    inline void _check_index(size_t i) const {
        if (i >= size()){
            auto msg = "std::vector index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
            throw std::out_of_range(msg);
        }
    }
public:
    PyVar& operator[](size_t i) {
        _check_index(i);
        return std::vector<PyVar>::operator[](i);
    }

    const PyVar& operator[](size_t i) const {
        _check_index(i);
        return std::vector<PyVar>::operator[](i);
    }

    using std::vector<PyVar>::vector;
};

typedef emhash8::HashMap<Str, PyVar> NameDict;

}

namespace pkpy {
    const int kMaxPoolSize = 10;
    static THREAD_LOCAL std::vector<PyVar*>* _args_pool = new std::vector<PyVar*>[kMaxPoolSize];

    class Args {
        PyVar* _args;
        int _size;

        void _alloc(int n){
            if(n == 0){
                this->_args = nullptr;
                this->_size = 0;
                return;
            }
            if(n >= kMaxPoolSize || _args_pool[n].empty()){
                this->_args = new PyVar[n];
                this->_size = n;
            }else{
                this->_args = _args_pool[n].back();
                this->_size = n;
                _args_pool[n].pop_back();
            }
        }

        void _dealloc(){
            if(_size == 0 || _args == nullptr) return;
            if(_size >= kMaxPoolSize || _args_pool[_size].size() > 32){
                delete[] _args;
            }else{
                for(int i = 0; i < _size; i++) _args[i].reset();
                _args_pool[_size].push_back(_args);
            }
        }

    public:
        Args(int n){ _alloc(n); }

        Args(const Args& other){
            _alloc(other._size);
            for(int i=0; i<_size; i++) _args[i] = other._args[i];
        }

        Args(std::initializer_list<PyVar> a){
            _alloc(a.size());
            int i = 0;
            for(auto& v: a) _args[i++] = v;
        }

        Args(Args&& other) noexcept {
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
        }

        Args(pkpy::List&& other) noexcept {
            _alloc(other.size());
            for(int i=0; i<_size; i++) _args[i] = std::move(other[i]);
            other.clear();
        }

        PyVar& operator[](int i){ return _args[i]; }
        const PyVar& operator[](int i) const { return _args[i]; }

        Args& operator=(Args&& other) noexcept {
            _dealloc();
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
            return *this;
        }

        inline int size() const { return _size; }

        pkpy::List to_list() const {
            pkpy::List ret(_size);
            for(int i=0; i<_size; i++) ret[i] = _args[i];
            return ret;
        }

        void extend_self(const PyVar& self){
            static_assert(std::is_standard_layout_v<PyVar>);
            PyVar* old_args = _args;
            int old_size = _size;
            _alloc(old_size+1);
            _args[0] = self;
            if(old_size == 0) return;

            memcpy((void*)(_args+1), (void*)old_args, sizeof(PyVar)*old_size);
            memset((void*)old_args, 0, sizeof(PyVar)*old_size);
            if(old_size >= kMaxPoolSize || _args_pool[old_size].size() > 32){
                delete[] old_args;
            }else{
                _args_pool[old_size].push_back(old_args);
            }
        }

        ~Args(){ _dealloc(); }
    };

    static const Args _zero(0);
    inline const Args& no_arg() { return _zero; }

    template<typename T>
    Args one_arg(T&& a) {
        Args ret(1);
        ret[0] = std::forward<T>(a);
        return ret;
    }

    template<typename T1, typename T2>
    Args two_args(T1&& a, T2&& b) {
        Args ret(2);
        ret[0] = std::forward<T1>(a);
        ret[1] = std::forward<T2>(b);
        return ret;
    }

    typedef Args Tuple;
}