#pragma once

#include "__stl__.h"

namespace pkpy{
    template <typename T>
    class shared_ptr {
        int* count;
        T* ptr;

    inline void _delete(){
        delete count;
        delete ptr;
    }

    public:
        shared_ptr() : count(nullptr), ptr(nullptr) {}
        shared_ptr(T* ptr) : count(new int(1)), ptr(ptr) {}
        ~shared_ptr() {
            if (count && --(*count) == 0) _delete();
        }

        bool operator==(const shared_ptr& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const shared_ptr& other) const {
            return ptr != other.ptr;
        }

        bool operator==(std::nullptr_t) const {
            return ptr == nullptr;
        }

        bool operator!=(std::nullptr_t) const {
            return ptr != nullptr;
        }

        shared_ptr& operator=(const shared_ptr& other) {
            if (this != &other) {
                if (count && --(*count) == 0) _delete();
                count = other.count;
                ptr = other.ptr;
                if (count) ++(*count);  
            }
            return *this;
        }
        
        shared_ptr& operator=(shared_ptr&& other) {
            if (this != &other) {
                if (count && --(*count) == 0) _delete();
                count = other.count;
                ptr = other.ptr;
                other.count = nullptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        T& operator*() const {
            return *ptr;
        }
        T* operator->() const {
            return ptr;
        }
        T* get() const {
            return ptr;
        }
        int use_count() const {
            return count ? *count : 0;
        }
    };

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        return shared_ptr<T>(new T(std::forward<Args>(args)...));
    }
};