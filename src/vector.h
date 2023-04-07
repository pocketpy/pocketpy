#pragma once

#include "common.h"

namespace pkpy{

template<typename T, int N>
struct small_vector{
    int _size;
    int _capacity;
    T* _data;
    T _buffer[N];

    small_vector(): _size(0), _capacity(N) {
        static_assert(std::is_pod_v<T>);
        _data = _buffer;
    }

    small_vector(int size): _size(0), _capacity(N){
        _data = _buffer;
        reserve(size);
        _size = size;
    }

    small_vector(const small_vector& other): _size(other._size), _capacity(other._capacity) {
        if(other.is_small()){
            _data = _buffer;
            memcpy(_buffer, other._buffer, sizeof(T) * _size);
        } else {
            _data = (T*)malloc(sizeof(T) * _capacity);
            memcpy(_data, other._data, sizeof(T) * _size);
        }
    }

    small_vector(small_vector&& other) noexcept {
        _size = other._size;
        _capacity = other._capacity;
        if(other.is_small()){
            _data = _buffer;
            memcpy(_buffer, other._buffer, sizeof(T) * _size);
        } else {
            _data = other._data;
            other._data = other._buffer;
        }
    }

    small_vector& operator=(small_vector&& other) noexcept {
        if (!is_small()) free(_data);
        _size = other._size;
        _capacity = other._capacity;
        if(other.is_small()){
            _data = _buffer;
            memcpy(_buffer, other._buffer, sizeof(T) * _size);
        } else {
            _data = other._data;
            other._data = other._buffer;
        }
        return *this;
    }

    // remove copy assignment
    small_vector& operator=(const small_vector& other) = delete;

    template<typename __ValueT>
    void push_back(__ValueT&& t) {
        if (_size == _capacity) reserve(_capacity*2);
        _data[_size++] = std::forward<__ValueT>(t);
    }

    void reserve(int cap){
        if(cap < _capacity) return;
        _capacity = cap;
        if (is_small()) {
            _data = (T*)malloc(sizeof(T) * _capacity);
            memcpy(_data, _buffer, sizeof(T) * _size);
        } else {
            _data = (T*)realloc(_data, sizeof(T) * _capacity);
        }
    }

    void pop_back() { _size--; }
    void extend(const small_vector& other){
        for(int i=0; i<other.size(); i++) push_back(other[i]);
    }

    T& operator[](int index) { return _data[index]; }
    const T& operator[](int index) const { return _data[index]; }

    T* begin() { return _data; }
    T* end() { return _data + _size; }
    const T* begin() const { return _data; }
    const T* end() const { return _data + _size; }
    T& back() { return _data[_size - 1]; }
    const T& back() const { return _data[_size - 1]; }

    bool empty() const { return _size == 0; }
    int size() const { return _size; }
    T* data() { return _data; }
    const T* data() const { return _data; }
    bool is_small() const { return _data == _buffer; }
    void pop_back_n(int n) { _size -= n; }
    void clear() { _size=0; }

    template<typename __ValueT>
    void insert(int i, __ValueT&& val){
        if (_size == _capacity) reserve(_capacity*2);
        for(int j=_size; j>i; j--) _data[j] = _data[j-1];
        _data[i] = std::forward<__ValueT>(val);
        _size++;
    }

    void erase(int i){
        for(int j=i; j<_size-1; j++) _data[j] = _data[j+1];
        _size--;
    }

    ~small_vector() {
        if (!is_small()) free(_data);
    }
};


template <typename T, typename Container=std::vector<T>>
class stack{
	Container vec;
public:
	void push(const T& t){ vec.push_back(t); }
	void push(T&& t){ vec.push_back(std::move(t)); }
	void pop(){ vec.pop_back(); }
	void clear(){ vec.clear(); }
	bool empty() const { return vec.empty(); }
	size_t size() const { return vec.size(); }
	T& top(){ return vec.back(); }
	const T& top() const { return vec.back(); }
	T popx(){ T t = std::move(vec.back()); vec.pop_back(); return t; }
	const Container& data() const { return vec; }
};

template <typename T, int N>
using small_stack = stack<T, small_vector<T, N>>;
} // namespace pkpy