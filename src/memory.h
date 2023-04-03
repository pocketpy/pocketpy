#pragma once

#include "common.h"

namespace pkpy{

template <typename T>
struct shared_ptr {
    int* counter;

    T* _t() const noexcept { return (T*)(counter + 1); }
    void _inc_counter() { if(counter) ++(*counter); }
    void _dec_counter() { if(counter && --(*counter) == 0) {((T*)(counter + 1))->~T(); free(counter);} }

public:
    shared_ptr() : counter(nullptr) {}
    shared_ptr(int* counter) : counter(counter) {}
    shared_ptr(const shared_ptr& other) : counter(other.counter) {
        _inc_counter();
    }
    shared_ptr(shared_ptr&& other) noexcept : counter(other.counter) {
        other.counter = nullptr;
    }
    ~shared_ptr() { _dec_counter(); }

    bool operator==(const shared_ptr& other) const { return counter == other.counter; }
    bool operator!=(const shared_ptr& other) const { return counter != other.counter; }
    bool operator<(const shared_ptr& other) const { return counter < other.counter; }
    bool operator>(const shared_ptr& other) const { return counter > other.counter; }
    bool operator<=(const shared_ptr& other) const { return counter <= other.counter; }
    bool operator>=(const shared_ptr& other) const { return counter >= other.counter; }
    bool operator==(std::nullptr_t) const { return counter == nullptr; }
    bool operator!=(std::nullptr_t) const { return counter != nullptr; }

    shared_ptr& operator=(const shared_ptr& other) {
        _dec_counter();
        counter = other.counter;
        _inc_counter();
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        _dec_counter();
        counter = other.counter;
        other.counter = nullptr;
        return *this;
    }

    T& operator*() const { return *_t(); }
    T* operator->() const { return _t(); }
    T* get() const { return _t(); }

    int use_count() const { 
        return counter ? *counter : 0;
    }

    void reset(){
        _dec_counter();
        counter = nullptr;
    }
};

template <typename T, typename... Args>
shared_ptr<T> make_sp(Args&&... args) {
    int* p = (int*)malloc(sizeof(int) + sizeof(T));
    *p = 1;
    new(p+1) T(std::forward<Args>(args)...);
    return shared_ptr<T>(p);
}

template<typename T, int __Bucket, bool __ZeroInit>
struct FreeListA {
    std::vector<T*> buckets[__Bucket+1];

    T* alloc(int n){
        static_assert(std::is_standard_layout_v<T>);
        T* p;
        if(n > __Bucket || buckets[n].empty()){
            p = (T*)malloc(sizeof(T) * n);
        }else{
            p = buckets[n].back();
            buckets[n].pop_back();
        }
        if constexpr(__ZeroInit){
            // the constructor of T should be equivalent to zero initialization
            memset((void*)p, 0, sizeof(T) * n);
        }
        return p;
    }

    void dealloc(T* p, int n){
        if(p == nullptr) return;
        if(n > __Bucket || buckets[n].size() >= 80){
            free(p);
        }else{
            buckets[n].push_back(p);
        }
    }

    ~FreeListA(){
        for(int i=0; i<=__Bucket; i++){
            for(T* p : buckets[i]) free(p);
        }
    }
};

};  // namespace pkpy
