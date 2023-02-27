#pragma once

#include "common.h"

namespace pkpy{

struct PyObject;

template<typename T>
struct SpAllocator {
    template<typename U>
    inline static int* alloc(){
        return (int*)malloc(sizeof(int) + sizeof(U));
    }

    inline static void dealloc(int* counter){
        ((T*)(counter + 1))->~T();
        free(counter);
    }
};

template <typename T>
struct shared_ptr {
    union {
        int* counter;
        i64 bits;
    };

#define _t() (T*)(counter + 1)
#define _inc_counter() if(!is_tagged() && counter) ++(*counter)
#define _dec_counter() if(!is_tagged() && counter && --(*counter) == 0) SpAllocator<T>::dealloc(counter)

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
        if(is_tagged()) return 1;
        return counter ? *counter : 0;
    }

    void reset(){
        _dec_counter();
        counter = nullptr;
    }

    inline constexpr bool is_tagged() const {
        if constexpr(!std::is_same_v<T, PyObject>) return false;
        return (bits & 0b11) != 0b00;
    }
    inline bool is_tag_00() const { return (bits & 0b11) == 0b00; }
    inline bool is_tag_01() const { return (bits & 0b11) == 0b01; }
    inline bool is_tag_10() const { return (bits & 0b11) == 0b10; }
    inline bool is_tag_11() const { return (bits & 0b11) == 0b11; }
};

#undef _t
#undef _inc_counter
#undef _dec_counter

    template <typename T, typename U, typename... Args>
    shared_ptr<T> make_sp(Args&&... args) {
        static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
        static_assert(std::has_virtual_destructor_v<T>, "T must have virtual destructor");
        static_assert(!std::is_same_v<T, PyObject> || (!std::is_same_v<U, i64> && !std::is_same_v<U, f64>));
        int* p = SpAllocator<T>::template alloc<U>(); *p = 1;
        new(p+1) U(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T, typename... Args>
    shared_ptr<T> make_sp(Args&&... args) {
        int* p = SpAllocator<T>::template alloc<T>(); *p = 1;
        new(p+1) T(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

static_assert(sizeof(i64) == sizeof(int*));
static_assert(sizeof(f64) == sizeof(int*));
static_assert(sizeof(shared_ptr<PyObject>) == sizeof(int*));
static_assert(std::numeric_limits<float>::is_iec559);
static_assert(std::numeric_limits<double>::is_iec559);

template<typename T, int __Bucket, int __BucketSize=32>
struct SmallArrayPool {
    std::vector<T*> buckets[__Bucket+1];

    T* alloc(int n){
        if(n == 0) return nullptr;
        if(n > __Bucket || buckets[n].empty()){
            return new T[n];
        }else{
            T* p = buckets[n].back();
            buckets[n].pop_back();
            return p;
        }
    }

    void dealloc(T* p, int n){
        if(n == 0) return;
        if(n > __Bucket || buckets[n].size() >= __BucketSize){
            delete[] p;
        }else{
            buckets[n].push_back(p);
        }
    }

    ~SmallArrayPool(){
        for(int i=1; i<=__Bucket; i++){
            for(auto p: buckets[i]) delete[] p;
        }
    }
};


typedef shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef PyVar PyVarRef;

};  // namespace pkpy
