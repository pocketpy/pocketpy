#pragma once

#include "common.h"

struct PyObject;

namespace pkpy{
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
    class shared_ptr {
        int* counter;

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

        template <typename __VAL>
        inline __VAL cast() const {
            static_assert(std::is_same_v<T, PyObject>, "T must be PyObject");
            return reinterpret_cast<__VAL>(counter);
        }

        inline bool is_tagged() const {
            if constexpr(!std::is_same_v<T, PyObject>) return false;
            return (reinterpret_cast<i64>(counter) & 0b11) != 0b00;
        }
        inline bool is_tag_00() const { return (reinterpret_cast<i64>(counter) & 0b11) == 0b00; }
        inline bool is_tag_01() const { return (reinterpret_cast<i64>(counter) & 0b11) == 0b01; }
        inline bool is_tag_10() const { return (reinterpret_cast<i64>(counter) & 0b11) == 0b10; }
        inline bool is_tag_11() const { return (reinterpret_cast<i64>(counter) & 0b11) == 0b11; }
    };

#undef _t
#undef _inc_counter
#undef _dec_counter

    template <typename T, typename U, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
        static_assert(std::has_virtual_destructor_v<T>, "T must have virtual destructor");
        static_assert(!std::is_same_v<T, PyObject> || (!std::is_same_v<U, i64> && !std::is_same_v<U, f64>));
        int* p = SpAllocator<T>::template alloc<U>(); *p = 1;
        new(p+1) U(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        int* p = SpAllocator<T>::template alloc<T>(); *p = 1;
        new(p+1) T(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }
};

static_assert(sizeof(i64) == sizeof(pkpy::shared_ptr<PyObject>));
static_assert(sizeof(f64) == sizeof(pkpy::shared_ptr<PyObject>));