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
	Container& container() { return vec; }
    const Container& container() const { return vec; }
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


namespace pkpy
{

// explicitly mark a type as trivially relocatable for better performance
    template<typename T>
    struct TriviallyRelocatable
    {
        constexpr static bool value =
                std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;
    };

    template<typename T>
    constexpr inline bool is_trivially_relocatable_v =
            TriviallyRelocatable<T>::value;

    template<typename T>
    struct TriviallyRelocatable<std::shared_ptr<T>>
    {
        constexpr static bool value = true;
    };


// the implementation of small_vector
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
                std::memcpy(dest, src, sizeof(T) * n);
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
                std::memcpy(dest, src, sizeof(T) * n);
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
            uninitialized_copy_n(other.begin, size, this->m_begin);
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

// small_vector_no_copy_and_move

    template<typename T, std::size_t N>
    class small_vector_no_copy_and_move: public small_vector<T, N>
    {
    public:
        small_vector_no_copy_and_move() = default;
        small_vector_no_copy_and_move(const small_vector_no_copy_and_move& other) = delete;
        small_vector_no_copy_and_move& operator=(const small_vector_no_copy_and_move& other) = delete;
        small_vector_no_copy_and_move(small_vector_no_copy_and_move&& other) = delete;
        small_vector_no_copy_and_move& operator=(small_vector_no_copy_and_move&& other) = delete;
    };
} // namespace pkpy