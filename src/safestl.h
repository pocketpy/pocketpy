#pragma once

#include "__stl__.h"
#include "memory.h"
#include "str.h"

struct PyObject;
typedef pkpy::shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef PyVar PyVarRef;

class PyVarList: public std::vector<PyVar> {
    PyVar& at(size_t) = delete;

    inline void __checkIndex(size_t i) const {
        if (i >= size()){
            auto msg = "std::vector index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
            throw std::out_of_range(msg);
        }
    }
public:
    PyVar& operator[](size_t i) {
        __checkIndex(i);
        return std::vector<PyVar>::operator[](i);
    }

    const PyVar& operator[](size_t i) const {
        __checkIndex(i);
        return std::vector<PyVar>::operator[](i);
    }

    // define constructors the same as std::vector
    using std::vector<PyVar>::vector;
};

typedef emhash8::HashMap<_Str, PyVar> PyVarDict;

namespace pkpy {
    const uint8_t MAX_POOLING_N = 10;
    static thread_local std::vector<PyVar*>* _poolArgList = new std::vector<PyVar*>[MAX_POOLING_N];

    class ArgList {
        PyVar* _args;
        uint8_t _size;

        void __tryAlloc(size_t n){
            if(n == 0){
                this->_args = nullptr;
                this->_size = 0;
                return;
            }
            if(n > 255) UNREACHABLE();
            if(n >= MAX_POOLING_N || _poolArgList[n].empty()){
                this->_args = new PyVar[n];
                this->_size = n;
            }else{
                this->_args = _poolArgList[n].back();
                this->_size = n;
                _poolArgList[n].pop_back();
            }
        }

        void __tryRelease(){
            if(_size == 0 || _args == nullptr) return;
            if(_size >= MAX_POOLING_N || _poolArgList[_size].size() > 32){
                delete[] _args;
            }else{
                for(uint8_t i = 0; i < _size; i++) _args[i].reset();
                _poolArgList[_size].push_back(_args);
            }
        }

    public:
        ArgList(size_t n){
            __tryAlloc(n);
        }

        ArgList(const ArgList& other){
            __tryAlloc(other._size);
            for(uint8_t i=0; i<_size; i++) _args[i] = other._args[i];
        }

        ArgList(ArgList&& other) noexcept {
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
        }

        ArgList(PyVarList&& other) noexcept {
            __tryAlloc(other.size());
            for(uint8_t i=0; i<_size; i++){
                _args[i] = std::move(other[i]);
            }
            other.clear();
        }

        PyVar& operator[](uint8_t i){ return _args[i]; }
        const PyVar& operator[](uint8_t i) const { return _args[i]; }

        ArgList& operator=(ArgList&& other) noexcept {
            if(this != &other){
                __tryRelease();
                this->_args = other._args;
                this->_size = other._size;
                other._args = nullptr;
                other._size = 0;
            }
            return *this;
        }

        inline uint8_t size() const { return _size; }

        PyVarList toList() const {
            PyVarList ret(_size);
            for(uint8_t i=0; i<_size; i++) ret[i] = _args[i];
            return ret;
        }

        void extend_self(const PyVar& self){
            static_assert(std::is_standard_layout_v<PyVar>);
            PyVar* old_args = _args;
            uint8_t old_size = _size;
            __tryAlloc(old_size+1);
            _args[0] = self;
            if(old_size == 0) return;

            memcpy((void*)(_args+1), (void*)old_args, sizeof(PyVar)*old_size);
            memset((void*)old_args, 0, sizeof(PyVar)*old_size);
            if(old_size >= MAX_POOLING_N || _poolArgList[old_size].size() > 32){
                delete[] old_args;
            }else{
                _poolArgList[old_size].push_back(old_args);
            }
        }

        ~ArgList(){
            __tryRelease();
        }
    };

    const ArgList& noArg(){
        static const ArgList ret(0);
        return ret;
    }

    template<typename T>
    ArgList oneArg(T&& a) {
        ArgList ret(1);
        ret[0] = std::forward<T>(a);
        return ret;
    }

    template<typename T1, typename T2>
    ArgList twoArgs(T1&& a, T2&& b) {
        ArgList ret(2);
        ret[0] = std::forward<T1>(a);
        ret[1] = std::forward<T2>(b);
        return ret;
    }
}