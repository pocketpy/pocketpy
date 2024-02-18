#pragma once

#include "common.h"
#include "memory.h"

namespace pkpy{

template<typename T>
struct pod_vector{
    static constexpr int SizeT = sizeof(T);
    static constexpr int N = 64 / SizeT;

    // static_assert(64 % SizeT == 0);
    static_assert(is_pod<T>::value);
    static_assert(N >= 4);

    int _size;
    int _capacity;
    T* _data;

    using size_type = int;

    pod_vector(): _size(0), _capacity(N) {
        _data = (T*)pool64_alloc(_capacity * SizeT);
    }

    // support initializer list
    pod_vector(std::initializer_list<T> il): _size(il.size()), _capacity(std::max(N, _size)) {
        _data = (T*)pool64_alloc(_capacity * SizeT);
        for(int i=0; i<_size; i++) _data[i] = *(il.begin() + i);
    }

    pod_vector(int size): _size(size), _capacity(std::max(N, size)) {
        _data = (T*)pool64_alloc(_capacity * SizeT);
    }

    pod_vector(const pod_vector& other): _size(other._size), _capacity(other._capacity) {
        _data = (T*)pool64_alloc(_capacity * SizeT);
        memcpy(_data, other._data, SizeT * _size);
    }

    pod_vector(pod_vector&& other) noexcept {
        _size = other._size;
        _capacity = other._capacity;
        _data = other._data;
        other._data = nullptr;
    }

    pod_vector& operator=(pod_vector&& other) noexcept {
        if(_data!=nullptr) pool64_dealloc(_data);
        _size = other._size;
        _capacity = other._capacity;
        _data = other._data;
        other._data = nullptr;
        return *this;
    }

    // remove copy assignment
    pod_vector& operator=(const pod_vector& other) = delete;

    template<typename __ValueT>
    void push_back(__ValueT&& t) {
        if (_size == _capacity) reserve(_capacity*2);
        _data[_size++] = std::forward<__ValueT>(t);
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (_size == _capacity) reserve(_capacity*2);
        new (&_data[_size++]) T(std::forward<Args>(args)...);
    }

    void reserve(int cap){
        if(cap <= _capacity) return;
        _capacity = cap;
        T* old_data = _data;
        _data = (T*)pool64_alloc(_capacity * SizeT);
        if(old_data != nullptr){
            memcpy(_data, old_data, SizeT * _size);
            pool64_dealloc(old_data);
        }
    }

    void pop_back() { _size--; }
    T popx_back() { T t = std::move(_data[_size-1]); _size--; return t; }
    
    void extend(const pod_vector& other){
        for(int i=0; i<other.size(); i++) push_back(other[i]);
    }

    void extend(const T* begin, const T* end){
        for(auto it=begin; it!=end; it++) push_back(*it);
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

    void reverse(){
        std::reverse(_data, _data+_size);
    }

    void resize(int size){
        if(size > _capacity) reserve(size);
        _size = size;
    }

    std::pair<T*, int> detach() noexcept {
        T* p = _data;
        int size = _size;
        _data = nullptr;
        _size = 0;
        return {p, size};
    }

    ~pod_vector() {
        if(_data != nullptr) pool64_dealloc(_data);
    }
};


template <typename T, typename Container=std::vector<T>>
class stack{
	Container vec;
public:
	void push(const T& t){ vec.push_back(t); }
	void push(T&& t){ vec.push_back(std::move(t)); }
    template<typename... Args>
    void emplace(Args&&... args){
        vec.emplace_back(std::forward<Args>(args)...);
    }
	void pop(){ vec.pop_back(); }
	void clear(){ vec.clear(); }
	bool empty() const { return vec.empty(); }
	typename Container::size_type size() const { return vec.size(); }
	T& top(){ return vec.back(); }
	const T& top() const { return vec.back(); }
	T popx(){ T t = std::move(vec.back()); vec.pop_back(); return t; }
    void reserve(int n){ vec.reserve(n); }
	Container& data() { return vec; }
};

template <typename T, typename Container=std::vector<T>>
class stack_no_copy: public stack<T, Container>{
public:
    stack_no_copy() = default;
    stack_no_copy(const stack_no_copy& other) = delete;
    stack_no_copy& operator=(const stack_no_copy& other) = delete;
    stack_no_copy(stack_no_copy&& other) noexcept = default;
    stack_no_copy& operator=(stack_no_copy&& other) noexcept = default;
};

} // namespace pkpy


namespace pkpy {

// explicitly mark a type as trivially relocatable for better performance
    template <typename T> struct TriviallyRelocatable {
        constexpr static bool value =
                std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;
    };

    template <typename T>
    constexpr inline bool is_trivially_relocatable_v =
            TriviallyRelocatable<T>::value;

    template<typename T>
    struct TriviallyRelocatable<std::shared_ptr<T>>{
        constexpr static bool value = true;
    };

    template<typename T>
    struct TriviallyRelocatable<std::vector<T>>{
        constexpr static bool value = true;
    };

// the implementation of small_vector
    template <typename T, std::size_t N> class small_vector {
    public:
        union Internal {
            T *begin;
            alignas(T) char buffer[sizeof(T) * N];

        } m_internal;

        int m_capacity;
        int m_size;

    public:
        using value_type = T;
        using size_type = int;
        using difference_type = int;
        using reference = T &;
        using const_reference = const T &;
        using pointer = T *;
        using const_pointer = const T *;
        using iterator = T *;
        using const_iterator = const T *;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        bool is_small() const { return m_capacity == N; }

        size_type size() const { return m_size; }

        size_type capacity() const { return m_capacity; }

        bool empty() const { return m_size == 0; }

        pointer data() {
            return is_small() ? reinterpret_cast<T *>(m_internal.buffer)
                              : m_internal.begin;
        }

        const_pointer data() const {
            return is_small() ? reinterpret_cast<const T *>(m_internal.buffer)
                              : m_internal.begin;
        }

        reference operator[](size_type index) { return data()[index]; }

        const_reference operator[](size_type index) const { return data()[index]; }

        reference front() { return data()[0]; }

        const_reference front() const { return data()[0]; }

        reference back() { return data()[m_size - 1]; }

        const_reference back() const { return data()[m_size - 1]; }

        iterator begin() { return data(); }

        const_iterator begin() const { return data(); }

        const_iterator cbegin() const { return data(); }

        iterator end() { return data() + m_size; }

        const_iterator end() const { return data() + m_size; }

        const_iterator cend() const { return data() + m_size; }

        reverse_iterator rbegin() { return reverse_iterator(end()); }

        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const {
            return const_reverse_iterator(end());
        }

        reverse_iterator rend() { return reverse_iterator(begin()); }

        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const {
            return const_reverse_iterator(begin());
        }

    private:
        static void uninitialized_copy_n(const void *src, size_type n, void *dest) {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(dest, src, sizeof(T) * n);
            } else {
                for (size_type i = 0; i < n; i++) {
                    ::new ((T *)dest + i) T(*((const T *)src + i));
                }
            }
        }

        static void uninitialized_relocate_n(void *src, size_type n, void *dest) {
            if constexpr (is_trivially_relocatable_v<T>) {
                std::memcpy(dest, src, sizeof(T) * n);
            } else {
                for (size_type i = 0; i < n; i++) {
                    ::new ((T *)dest + i) T(std::move(*((T *)src + i)));
                    ((T *)src + i)->~T();
                }
            }
        }

    public:
        small_vector() : m_capacity(N), m_size(0) {}

        small_vector(const small_vector &other) noexcept
                : m_capacity(other.m_capacity), m_size(other.m_size) {
            if (other.is_small()) {
                uninitialized_copy_n(other.m_internal.buffer, other.m_size,
                                     m_internal.buffer);
            } else {
                m_internal.begin = (pointer)std::malloc(sizeof(T) * m_capacity);
                uninitialized_copy_n(other.m_internal.begin, other.m_size,
                                     m_internal.begin);
            }
        }

        small_vector(small_vector &&other) noexcept
                : m_capacity(other.m_capacity), m_size(other.m_size) {
            if (other.is_small()) {
                uninitialized_relocate_n(other.m_internal.buffer, other.m_size,
                                         m_internal.buffer);
            } else {
                m_internal.begin = other.m_internal.begin;
                other.m_capacity = N;
            }
            other.m_size = 0;
        }

        small_vector &operator=(const small_vector &other) noexcept {
            if (this != &other) {
                ~small_vector();
                if (other.is_small()) {
                    uninitialized_copy_n(other.m_internal.buffer, other.m_size,
                                         m_internal.buffer);
                } else {
                    m_internal.begin = (pointer)std::malloc(sizeof(T) * other.m_capacity);
                    uninitialized_copy_n(other.m_internal.begin, other.m_size,
                                         m_internal.begin);
                }
                m_capacity = other.m_capacity;
                m_size = other.m_size;
            }
            return *this;
        }

        small_vector &operator=(small_vector &&other) noexcept {
            if (this != &other) {
                ~small_vector();
                if (other.is_small()) {
                    uninitialized_relocate_n(other.m_internal.buffer, other.m_size,
                                             m_internal.buffer);
                } else {
                    m_internal.begin = other.m_internal.begin;
                }
                m_capacity = other.m_capacity;
                m_size = other.m_size;
                other.m_capacity = N;
                other.m_size = 0;
            }
            return *this;
        }

        ~small_vector() {
            std::destroy_n(data(), m_size);
            if (!is_small()) {
                std::free(m_internal.begin);
            }
        }

        template <typename... Args> void emplace_back(Args &&...args) noexcept {
            if (m_size == m_capacity) {
                auto new_capacity = m_capacity * 2;
                if (!is_small()) {
                    if constexpr (is_trivially_relocatable_v<T>) {
                        m_internal.begin =
                                (pointer)std::realloc(m_internal.begin, sizeof(T) * new_capacity);
                    } else {
                        auto new_data = (pointer)std::malloc(sizeof(T) * new_capacity);
                        uninitialized_relocate_n(m_internal.begin, m_size, new_data);
                        std::free(m_internal.begin);
                        m_internal.begin = new_data;
                    }
                } else {
                    auto new_data = (pointer)std::malloc(sizeof(T) * new_capacity);
                    uninitialized_relocate_n(m_internal.buffer, m_size, new_data);
                    m_internal.begin = new_data;
                }
                m_capacity = new_capacity;
            }
            ::new (data() + m_size) T(std::forward<Args>(args)...);
            m_size++;
        }

        void push_back(const T &value) { emplace_back(value); }

        void push_back(T &&value) { emplace_back(std::move(value)); }

        void pop_back() {
            m_size--;
            if constexpr (!std::is_trivially_destructible_v<T>) {
                (data() + m_size)->~T();
            }
        }

        void clear() {
            std::destroy_n(data(), m_size);
            m_size = 0;
        }
    };
} // namespace pkpy