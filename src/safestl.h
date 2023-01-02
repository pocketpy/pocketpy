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
#ifndef PKPY_NO_INDEX_CHECK
        if (i >= size()){
            auto msg = "std::vector index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
            throw std::out_of_range(msg);
        }
#endif
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
        PyVar* _args = nullptr;
        uint8_t _size = 0;

        inline void __checkIndex(uint8_t i) const {
#ifndef PKPY_NO_INDEX_CHECK
            if (i >= _size){
                auto msg = "pkpy:ArgList index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
                throw std::out_of_range(msg);
            }
#endif
        }

        void __tryAlloc(size_t n){
            if(n > 255) UNREACHABLE();
            if(n >= MAX_POOLING_N || _poolArgList[n].empty()){
                this->_size = n;
                this->_args = new PyVar[n];
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
            if(n != 0) __tryAlloc(n);
        }

        ArgList(const ArgList& other){
            __tryAlloc(other._size);
            for(uint8_t i=0; i<_size; i++){
                _args[i] = other._args[i];
            }
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

        PyVar& operator[](uint8_t i){
            __checkIndex(i);
            return _args[i];
        }

        const PyVar& operator[](uint8_t i) const {
            __checkIndex(i);
            return _args[i];
        }

        inline PyVar& _index(uint8_t i){
            return _args[i];
        }

        inline const PyVar& _index(uint8_t i) const {
            return _args[i];
        }

        // overload = for &&
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

        inline uint8_t size() const {
            return _size;
        }

        ArgList subList(uint8_t start) const {
            if(start >= _size) return ArgList(0);
            ArgList ret(_size - start);
            for(uint8_t i=start; i<_size; i++){
                ret[i-start] = _args[i];
            }
            return ret;
        }

        PyVarList toList() const {
            PyVarList ret(_size);
            for(uint8_t i=0; i<_size; i++){
                ret[i] = _args[i];
            }
            return ret;
        }

        ~ArgList(){
            __tryRelease();
        }
    };

    const ArgList& noArg(){
        static const ArgList ret(0);
        return ret;
    }

    ArgList oneArg(PyVar&& a) {
        ArgList ret(1);
        ret[0] = std::move(a);
        return ret;
    }

    ArgList oneArg(const PyVar& a) {
        ArgList ret(1);
        ret[0] = a;
        return ret;
    }

    ArgList twoArgs(PyVar&& a, PyVar&& b) {
        ArgList ret(2);
        ret[0] = std::move(a);
        ret[1] = std::move(b);
        return ret;
    }

    ArgList twoArgs(const PyVar& a, const PyVar& b) {
        ArgList ret(2);
        ret[0] = a;
        ret[1] = b;
        return ret;
    }
}