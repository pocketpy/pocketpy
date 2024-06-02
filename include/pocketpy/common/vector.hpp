#pragma once

#include "pocketpy/common/traits.hpp"
#include "pocketpy/common/types.hpp"

#include <cstring>
#include <memory>
#include <cassert>

namespace pkpy{

template<typename T>
struct array{
    T* _data;
    int _size;

    using size_type = int;

    array(): _data(nullptr), _size(0) {}
    array(int size): _data((T*)malloc(sizeof(T) * size)), _size(size) {}
    array(array&& other) noexcept: _data(other._data), _size(other._size) {
        other._data = nullptr;
        other._size = 0;
    }
    array(const array& other) = delete;
    array(explicit_copy_t, const array& other) {
        _data = (T*)malloc(sizeof(T) * other._size);
        _size = other._size;
        for(int i=0; i<_size; i++) _data[i] = other._data[i];
    }
    array(T* data, int size): _data(data), _size(size) {}

    array& operator=(array&& other) noexcept{
        if(_data){
            std::destroy(begin(), end());
            free(_data);
        }
        _data = other._data;
        _size = other._size;
        other._data = nullptr;
        other._size = 0;
        return *this;
    }

    array& operator=(const array& other) = delete;

    T& operator[](int i) {
        assert(i >= 0 && i < _size);
        return _data[i];
    }

    const T& operator[](int i) const {
        assert(i >= 0 && i < _size);
        return _data[i];
    }

    int size() const { return _size; }

    T* begin() const{ return _data; }
    T* end() const{ return _data + _size; }
    T* data() const { return _data; }

    std::pair<T*, int> detach() noexcept {
        std::pair<T*, int> retval(_data, _size);
        _data = nullptr;
        _size = 0;
        return retval;
    }

    ~array() {
        if(_data){
            std::destroy(begin(), end());
            free(_data);
        }
    }
};


template<typename T>
struct vector{
    T* _data;
    int _capacity;
    int _size;

    using size_type = int;

    vector(): _data(nullptr), _capacity(0), _size(0) {}
    vector(int size):
        _data((T*)malloc(sizeof(T) * size)),
        _capacity(size), _size(size) {}
    vector(vector&& other) noexcept:
        _data(other._data), _capacity(other._capacity), _size(other._size) {
        other._data = nullptr;
        other._capacity = 0;
        other._size = 0;
    }
    vector(const vector& other) = delete;
    vector(explicit_copy_t, const vector& other):
        _data((T*)malloc(sizeof(T) * other._size)),
        _capacity(other._size), _size(other._size) {
        for(int i=0; i<_size; i++) _data[i] = other._data[i];
    }

    // allow move
    vector& operator=(vector&& other) noexcept{
        if(_data){
            std::destroy(begin(), end());
            free(_data);
        }
        new (this) vector(std::move(other));
        return *this;
    }
    // disallow copy
    vector& operator=(const vector& other) = delete;

    bool empty() const { return _size == 0; }
    int size() const { return _size; }
    int capacity() const { return _capacity; }
    T& back() { return _data[_size-1]; }

    T* begin() const { return _data; }
    T* end() const { return _data + _size; }
    T* data() const { return _data; }

    void reserve(int cap){
        if(cap < 4) cap = 4;    // minimum capacity
        if(cap <= capacity()) return;
        T* new_data = (T*)malloc(sizeof(T) * cap);
        if constexpr(is_trivially_relocatable_v<T>){
            memcpy(new_data, _data, sizeof(T) * _size);
        }else{
            for(int i=0; i<_size; i++){
                new(&new_data[i]) T(std::move(_data[i]));
                _data[i].~T();
            }
        }
        if(_data) free(_data);
        _data = new_data;
        _capacity = cap;
    }

    void resize(int size){
        reserve(size);
        _size = size;
    }

    void push_back(const T& t){
        if(_size == _capacity) reserve(_capacity * 2);
        new (&_data[_size++]) T(t);
    }

    void push_back(T&& t){
        if(_size == _capacity) reserve(_capacity * 2);
        new(&_data[_size++]) T(std::move(t));
    }

    bool contains(const T& t) const {
        for(int i=0; i<_size; i++){
            if(_data[i] == t) return true;
        }
        return false;
    }

    template<typename... Args>
    void emplace_back(Args&&... args){
        if(_size == _capacity) reserve(_capacity * 2);
        new(&_data[_size++]) T(std::forward<Args>(args)...);
    }

    T& operator[](int i) { return _data[i]; }
    const T& operator[](int i) const { return _data[i]; }

    void extend(T* begin, T* end){
        int n = end - begin;
        reserve(_size + n);
        for(int i=0; i<n; i++) new(&_data[_size++]) T(begin[i]);
    }

    void insert(int index, const T& t){
        if(_size == _capacity) reserve(_capacity * 2);
        for(int i=_size; i>index; i--) _data[i] = std::move(_data[i-1]);
        _data[index] = t;
        _size++;
    }

    void erase(int index){
        for(int i=index; i<_size-1; i++) _data[i] = std::move(_data[i+1]);
        _size--;
    }

    void pop_back(){
        assert(_size > 0);
        _size--;
        if constexpr(!std::is_trivially_destructible_v<T>){
            _data[_size].~T();
        }
    }

    void clear(){
        std::destroy(begin(), end());
        _size = 0;
    }

    std::pair<T*, int> detach() noexcept {
        std::pair<T*, int> retval(_data, _size);
        _data = nullptr;
        _capacity = 0;
        _size = 0;
        return retval;
    }

    void swap(vector& other){
        std::swap(_data, other._data);
        std::swap(_capacity, other._capacity);
        std::swap(_size, other._size);
    }

    ~vector(){
        if(_data){
            std::destroy(begin(), end());
            free(_data);
        }
    }
};

template <typename T, typename Container=vector<T>>
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
	Container& container() { return vec; }
    const Container& container() const { return vec; }
};

template <typename T, typename Container=vector<T>>
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
    template<typename T, std::size_t N>
    class small_vector
    {
        alignas(T) char m_buffer[sizeof(T) * N];
        T* m_begin;
        T* m_end;
        T* m_max;

    public:
        using value_type = T;
        using size_type = int;
        using difference_type = int;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        [[nodiscard]] bool is_small() const { return m_begin == reinterpret_cast<const T*>(m_buffer); }
        [[nodiscard]] size_type size() const { return m_end - m_begin; }
        [[nodiscard]] size_type capacity() const { return m_max - m_begin; }
        [[nodiscard]] bool empty() const { return m_begin == m_end; }

        pointer data() { return m_begin; }
        const_pointer data() const { return m_begin; }
        reference operator[](size_type index) { return m_begin[index]; }
        const_reference operator[](size_type index) const { return m_begin[index]; }
        iterator begin() { return m_begin; }
        const_iterator begin() const { return m_begin; }
        iterator end() { return m_end; }
        const_iterator end() const { return m_end; }
        reference front() { return *begin(); }
        const_reference front() const { return *begin(); }
        reference back() { return *(end() - 1); }
        const_reference back() const { return *(end() - 1); }
        reverse_iterator rbegin() { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(end());
        }
        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(begin());
        }
    private:
        static void uninitialized_copy_n(const void* src, size_type n, void* dest)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                memcpy(dest, src, sizeof(T) * n);
            }
            else
            {
                for (size_type i = 0; i < n; i++)
                {
                    ::new((T*) dest + i) T(*((const T*) src + i));
                }
            }
        }

        static void uninitialized_relocate_n(void* src, size_type n, void* dest)
        {
            if constexpr (is_trivially_relocatable_v<T>)
            {
                memcpy(dest, src, sizeof(T) * n);
            }
            else
            {
                for (size_type i = 0; i < n; i++)
                {
                    ::new((T*) dest + i) T(std::move(*((T*) src + i)));
                    ((T*) src + i)->~T();
                }
            }
        }

    public:
        small_vector() : m_begin(reinterpret_cast<T*>(m_buffer)), m_end(m_begin), m_max(m_begin + N) {}

        small_vector(const small_vector& other) noexcept
        {
            const auto size = other.size();
            const auto capacity = other.capacity();
            m_begin = reinterpret_cast<T*>(other.is_small() ? m_buffer : std::malloc(sizeof(T) * capacity));
            uninitialized_copy_n(other.m_begin, size, this->m_begin);
            m_end = m_begin + size;
            m_max = m_begin + capacity;
        }

        small_vector(small_vector&& other) noexcept
        {
            if(other.is_small())
            {
                m_begin = reinterpret_cast<T*>(m_buffer);
                uninitialized_relocate_n(other.m_buffer, other.size(), m_buffer);
                m_end = m_begin + other.size();
                m_max = m_begin + N;
            }
            else
            {
                m_begin = other.m_begin;
                m_end = other.m_end;
                m_max = other.m_max;
            }
            other.m_begin = reinterpret_cast<T*>(other.m_buffer);
            other.m_end = other.m_begin;
            other.m_max = other.m_begin + N;
        }

        small_vector& operator=(const small_vector& other) noexcept
        {
            if (this != &other)
            {
                ~small_vector();
                ::new (this) small_vector(other);
            }
            return *this;
        }

        small_vector& operator=(small_vector&& other) noexcept
        {
            if (this != &other)
            {
                ~small_vector();
                :: new (this) small_vector(std::move(other));
            }
            return *this;
        }

        ~small_vector()
        {
            std::destroy(m_begin, m_end);
            if (!is_small()) std::free(m_begin);
        }

        template<typename... Args>
        void emplace_back(Args&& ...args) noexcept
        {
            if (m_end == m_max)
            {
                const auto new_capacity = capacity() * 2;
                const auto size = this->size();
                if (!is_small())
                {
                    if constexpr (is_trivially_relocatable_v<T>)
                    {
                        m_begin = (pointer)std::realloc(m_begin, sizeof(T) * new_capacity);
                    }
                    else
                    {
                        auto new_data = (pointer) std::malloc(sizeof(T) * new_capacity);
                        uninitialized_relocate_n(m_begin, size, new_data);
                        std::free(m_begin);
                        m_begin = new_data;
                    }
                }
                else
                {
                    auto new_data = (pointer) std::malloc(sizeof(T) * new_capacity);
                    uninitialized_relocate_n(m_buffer, size, new_data);
                    m_begin = new_data;
                }
                m_end = m_begin + size;
                m_max = m_begin + new_capacity;
            }
            ::new(m_end) T(std::forward<Args>(args)...);
            m_end++;
        }

        void push_back(const T& value) { emplace_back(value); }
        void push_back(T&& value) { emplace_back(std::move(value)); }

        void pop_back()
        {
            m_end--;
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                m_end->~T();
            }
        }

        void clear()
        {
            std::destroy(m_begin, m_end);
            m_end = m_begin;
        }
    };

    template<typename T, std::size_t N>
    class small_vector_2: public small_vector<T, N>
    {
    public:
        small_vector_2() = default;
        small_vector_2(const small_vector_2& other) = delete;
        small_vector_2& operator=(const small_vector_2& other) = delete;
        small_vector_2(small_vector_2&& other) = delete;
        small_vector_2& operator=(small_vector_2&& other) = delete;
    };
} // namespace pkpy