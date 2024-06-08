#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <algorithm>

#include "pocketpy/common/traits.hpp"
#include "pocketpy/common/types.hpp"

namespace pkpy {

template <typename T>
struct array {
    T* _data;
    int _size;

    using size_type = int;

    array() : _data(nullptr), _size(0) {}

    array(int size) : _data((T*)std::malloc(sizeof(T) * size)), _size(size) {}

    array(array&& other) noexcept : _data(other._data), _size(other._size) {
        other._data = nullptr;
        other._size = 0;
    }

    array(const array& other) = delete;

    array(explicit_copy_t, const array& other) {
        _data = (T*)std::malloc(sizeof(T) * other._size);
        _size = other._size;
        for(int i = 0; i < _size; i++)
            _data[i] = other._data[i];
    }

    array(T* data, int size) : _data(data), _size(size) {}

    array& operator= (array&& other) noexcept {
        if(_data) {
            std::destroy(begin(), end());
            std::free(_data);
        }
        _data = other._data;
        _size = other._size;
        other._data = nullptr;
        other._size = 0;
        return *this;
    }

    array& operator= (const array& other) = delete;

    T& operator[] (int i) {
        assert(i >= 0 && i < _size);
        return _data[i];
    }

    const T& operator[] (int i) const {
        assert(i >= 0 && i < _size);
        return _data[i];
    }

    int size() const { return _size; }

    T* begin() const { return _data; }

    T* end() const { return _data + _size; }

    T* data() const { return _data; }

    std::pair<T*, int> detach() noexcept {
        std::pair<T*, int> retval(_data, _size);
        _data = nullptr;
        _size = 0;
        return retval;
    }

    ~array() {
        if(_data) {
            std::destroy(begin(), end());
            std::free(_data);
        }
    }
};

template <bool may_alias = false, typename T>
void uninitialized_copy_n(const T* src, int n, T* dest) {
    if(n == 0) return;
    if constexpr(std::is_trivially_copyable_v<T>) {
        if constexpr(may_alias) {
            std::memmove(dest, src, sizeof(T) * n);
        } else {
            std::memcpy(dest, src, sizeof(T) * n);
        }
    } else {
        for(int i = 0; i < n; i++) {
            new (dest + i) T(*(src + i));
        }
    }
}

template <bool may_alias = false, bool backward = false, typename T>
void uninitialized_relocate_n(T* src, int n, T* dest) {
    if(n == 0) return;
    if constexpr(is_trivially_relocatable_v<T>) {
        if constexpr(may_alias) {
            std::memmove(dest, src, sizeof(T) * n);
        } else {
            std::memcpy(dest, src, sizeof(T) * n);
        }
    } else {
        if constexpr(backward) {
            for(int i = n - 1; i >= 0; i--) {
                new (dest + i) T(std::move(*(src + i)));
                (src + i)->~T();
            }
        } else {
            for(int i = 0; i < n; i++) {
                new (dest + i) T(std::move(*(src + i)));
                (src + i)->~T();
            }
        }
    }
}

template <typename T>
struct vector {
    T* _data;
    int _capacity;
    int _size;

    using size_type = int;

    vector() : _data(nullptr), _capacity(0), _size(0) {}

    vector(int size) : _data((T*)std::malloc(sizeof(T) * size)), _capacity(size), _size(size) {}

    vector(vector&& other) noexcept : _data(other._data), _capacity(other._capacity), _size(other._size) {
        other._data = nullptr;
        other._capacity = 0;
        other._size = 0;
    }

    vector(const vector& other) = delete;

    vector(explicit_copy_t, const vector& other) : _capacity(other._size), _size(other._size) {
        _data = (T*)std::malloc(sizeof(T) * _capacity);
        uninitialized_copy_n(other._data, _size, _data);
    }

    // allow move
    vector& operator= (vector&& other) noexcept {
        if(_data) {
            std::destroy(begin(), end());
            std::free(_data);
        }
        new (this) vector(std::move(other));
        return *this;
    }

    // disallow copy
    vector& operator= (const vector& other) = delete;

    bool empty() const { return _size == 0; }

    int size() const { return _size; }

    int capacity() const { return _capacity; }

    T& back() { return _data[_size - 1]; }

    T* begin() const { return _data; }

    T* end() const { return _data + _size; }

    T* data() const { return _data; }

    T& operator[] (int i) { return _data[i]; }

    const T& operator[] (int i) const { return _data[i]; }

    void clear() {
        std::destroy(begin(), end());
        _size = 0;
    }

    void reserve(int cap) {
        if(cap < 4) cap = 4;  // minimum capacity
        if(cap <= capacity()) return;
        T* new_data = (T*)std::malloc(sizeof(T) * cap);
        uninitialized_relocate_n(_data, _size, new_data);
        if(_data) std::free(_data);
        _data = new_data;
        _capacity = cap;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        if(_size == _capacity) reserve(_capacity * 2);
        new (_data + _size) T(std::forward<Args>(args)...);
        _size++;
    }

    void push_back(const T& t) { emplace_back(t); }

    void push_back(T&& t) { emplace_back(std::move(t)); }

    bool contains(const T& t) const {
        for(int i = 0; i < _size; i++) {
            if(_data[i] == t) return true;
        }
        return false;
    }

    void extend(const T* begin, const T* end) {
        int n = end - begin;
        reserve(_size + n);
        uninitialized_copy_n(begin, n, _data + _size);
        _size += n;
    }

    void insert(const T* it, const T& t) {
        assert(it >= begin() && it <= end());
        int pos = it - begin();
        if(_size == _capacity) {
            int new_capacity = (_capacity == 0) ? 4 : _capacity * 2;
            T* new_data = (T*)std::malloc(sizeof(T) * new_capacity);
            uninitialized_relocate_n(_data, pos, new_data);
            new (new_data + pos) T(t);
            uninitialized_relocate_n(_data + pos, _size - pos, new_data + pos + 1);
            if(_data) std::free(_data);
            _data = new_data;
            _capacity = new_capacity;
        } else {
            uninitialized_relocate_n<true, true>(_data + pos, _size - pos, _data + pos + 1);
            new (_data + pos) T(t);
        }
        _size++;
    }

    void erase(T* it) {
        assert(it >= begin() && it < end());
        int pos = it - begin();
        _data[pos].~T();
        uninitialized_relocate_n<true>(_data + pos + 1, _size - pos, _data + pos);
        _size--;
    }

    void pop_back() {
        assert(_size > 0);
        _size--;
        _data[_size].~T();
    }

    [[nodiscard]] T popx_back() {
        T retval = std::move(back());
        pop_back();
        return retval;
    }

    std::pair<T*, int> detach() noexcept {
        std::pair<T*, int> retval(_data, _size);
        _data = nullptr;
        _capacity = 0;
        _size = 0;
        return retval;
    }

    void swap(vector& other) {
        std::swap(_data, other._data);
        std::swap(_capacity, other._capacity);
        std::swap(_size, other._size);
    }

    ~vector() {
        if(_data) {
            std::destroy(begin(), end());
            std::free(_data);
        }
    }
};

template <typename T, std::size_t N>
struct small_vector {
    alignas(T) char _buffer[sizeof(T) * N];
    T* _begin;
    T* _end;
    T* _capacity;

    [[nodiscard]] bool is_small() const { return _begin == reinterpret_cast<const T*>(_buffer); }

    [[nodiscard]] int size() const { return _end - _begin; }

    [[nodiscard]] int capacity() const { return _capacity - _begin; }

    [[nodiscard]] bool empty() const { return _begin == _end; }

    [[nodiscard]] T* data() const { return _begin; }

    [[nodiscard]] T* begin() const { return _begin; }

    [[nodiscard]] T* end() const { return _end; }

    [[nodiscard]] T& back() const { return *(end() - 1); }

    [[nodiscard]] T& operator[] (int index) { return _begin[index]; }

    [[nodiscard]] const T& operator[] (int index) const { return _begin[index]; }

    small_vector() : _begin(reinterpret_cast<T*>(_buffer)), _end(_begin), _capacity(_begin + N) {}

    small_vector(const small_vector& other) noexcept {
        const auto size = other.size();
        const auto capacity = other.capacity();
        _begin = reinterpret_cast<T*>(other.is_small() ? _buffer : std::malloc(sizeof(T) * capacity));
        uninitialized_copy_n(other._begin, size, this->_begin);
        _end = _begin + size;
        _capacity = _begin + capacity;
    }

    small_vector(small_vector&& other) noexcept {
        if(other.is_small()) {
            _begin = reinterpret_cast<T*>(_buffer);
            uninitialized_relocate_n((T*)other._buffer, other.size(), (T*)_buffer);
            _end = _begin + other.size();
            _capacity = _begin + N;
        } else {
            _begin = other._begin;
            _end = other._end;
            _capacity = other._capacity;
        }
        other._begin = reinterpret_cast<T*>(other._buffer);
        other._end = other._begin;
        other._capacity = other._begin + N;
    }

    small_vector& operator= (const small_vector& other) = delete;

    small_vector& operator= (small_vector&& other) noexcept {
        if(this != &other) {
            ~small_vector();
            ::new (this) small_vector(std::move(other));
        }
        return *this;
    }

    ~small_vector() {
        std::destroy(_begin, _end);
        if(!is_small()) std::free(_begin);
    }

    template <typename... Args>
    void emplace_back(Args&&... args) noexcept {
        if(_end == _capacity) {
            const auto new_capacity = capacity() * 2;
            const auto size = this->size();
            auto new_data = (T*)std::malloc(sizeof(T) * new_capacity);
            uninitialized_relocate_n(_begin, size, new_data);
            if(!is_small()) std::free(_begin);
            _begin = new_data;
            _end = _begin + size;
            _capacity = _begin + new_capacity;
        }
        ::new (_end) T(std::forward<Args>(args)...);
        _end++;
    }

    void push_back(const T& value) { emplace_back(value); }

    void push_back(T&& value) { emplace_back(std::move(value)); }

    void pop_back() {
        _end--;
        _end->~T();
    }

    void clear() {
        std::destroy(_begin, _end);
        _end = _begin;
    }
};

template <typename T, std::size_t N>
class small_vector_2 : public small_vector<T, N> {
public:
    small_vector_2() = default;
    small_vector_2(const small_vector_2& other) = delete;
    small_vector_2& operator= (const small_vector_2& other) = delete;
    small_vector_2(small_vector_2&& other) = delete;
    small_vector_2& operator= (small_vector_2&& other) = delete;
};

template <typename K, typename V>
struct small_map {
    struct Item {
        K first;
        V second;

        bool operator< (const K& other) const { return first < other; }

        bool operator< (const Item& other) const { return first < other.first; }
    };

    vector<Item> _data;

    small_map() = default;

    using size_type = int;

    int size() const { return _data.size(); }

    bool empty() const { return _data.empty(); }

    Item* begin() const { return _data.begin(); }

    Item* end() const { return _data.end(); }

    Item* data() const { return _data.data(); }

    void insert(const K& key, const V& value) {
        Item* it = std::lower_bound(_data.begin(), _data.end(), key);
        assert(it == _data.end() || it->first != key);
        _data.insert(it, {key, value});
    }

    V* try_get(const K& key) const {
        auto it = std::lower_bound(_data.begin(), _data.end(), key);
        if(it == _data.end() || it->first != key) return nullptr;
        return &it->second;
    }

    V get(const K& key, V default_value) const {
        static_assert(is_pod_v<V>);
        auto it = try_get(key);
        return it ? *it : default_value;
    }

    bool contains(const K& key) const { return try_get(key) != nullptr; }

    void clear() { _data.clear(); }

    const V& operator[] (const K& key) const {
        auto it = try_get(key);
        assert(it != nullptr);
        return *it;
    }
};

}  // namespace pkpy
