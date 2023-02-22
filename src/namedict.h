#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

    struct NameDictNode{
        StrName first;
        PyVar second;
        inline bool empty() const { return first.empty(); }
    };

    struct NameDict {
        int _capacity;
        int _size;
        float _load_factor;
        NameDictNode* _a;

        NameDict(int capacity=4, float load_factor=0.67):
            _capacity(capacity), _size(0), _load_factor(load_factor) {
            _a = new NameDictNode[_capacity];
        }

        NameDict(const NameDict& other) {
            this->_capacity = other._capacity;
            this->_size = other._size;
            this->_a = new NameDictNode[_capacity];
            for(int i=0; i<_capacity; i++) _a[i] = other._a[i];
        }
        
        NameDict& operator=(const NameDict& other){
            delete[] _a;
            this->_capacity = other._capacity;
            this->_size = other._size;
            this->_a = new NameDictNode[_capacity];
            for(int i=0; i<_capacity; i++) _a[i] = other._a[i];
            return *this;
        }

        NameDict(NameDict&&) = delete;
        NameDict& operator=(NameDict&&) = delete;

        int size() const { return _size; }

    //https://github.com/python/cpython/blob/main/Objects/dictobject.c#L175
    #define HASH_PROBE(key, ok, i) \
        int i = (key).index % _capacity; \
        bool ok = false; \
        while(!_a[i].empty()) { \
            if(_a[i].first == (key)) { ok = true; break; } \
            i = (5*i + 1) % _capacity; \
        }

    #define HASH_PROBE_OVERRIDE(key, ok, i) \
        i = (key).index % _capacity; \
        ok = false; \
        while(!_a[i].empty()) { \
            if(_a[i].first == (key)) { ok = true; break; } \
            i = (5*i + 1) % _capacity; \
        }

        const PyVar& operator[](StrName key) const {
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found");
            return _a[i].second;
        }

        [[nodiscard]] PyVar& operator[](StrName key){
            HASH_PROBE(key, ok, i);
            if(!ok) {
                _a[i].first = key;
                _size++;
                if(_size > _capacity * _load_factor){
                    _rehash_2x();
                    HASH_PROBE_OVERRIDE(key, ok, i);
                }
            }
            return _a[i].second;
        }

        void _rehash_2x(){
            NameDictNode* old_a = _a;
            int old_capacity = _capacity;
            _capacity *= 2;
            _size = 0;
            _a = new NameDictNode[_capacity];
            for(int i=0; i<old_capacity; i++){
                if(old_a[i].empty()) continue;
                HASH_PROBE(old_a[i].first, ok, j);
                if(ok) UNREACHABLE();
                _a[j].first = old_a[i].first;
                _a[j].second = std::move(old_a[i].second);
                _size++;
            }
            delete[] old_a;
        }

        inline PyVar* try_get(StrName key){
            HASH_PROBE(key, ok, i);
            if(!ok) return nullptr;
            return &_a[i].second;
        }

        inline bool contains(StrName key) const {
            HASH_PROBE(key, ok, i);
            return ok;
        }

        ~NameDict(){ delete[] _a;}

        struct iterator {
            const NameDict* _dict;
            int i;
            iterator() = default;
            iterator(const NameDict* dict, int i): _dict(dict), i(i) { _skip_empty(); }
            inline void _skip_empty(){ while(i < _dict->_capacity && _dict->_a[i].empty()) i++;}
            inline iterator& operator++(){ i++; _skip_empty(); return *this;}

            inline bool operator!=(const iterator& other) const { return i != other.i; }
            inline bool operator==(const iterator& other) const { return i == other.i; }

            inline NameDictNode* operator->() const { return &_dict->_a[i]; }
        };

        template<typename T>
        void emplace(StrName key, T&& value){
            HASH_PROBE(key, ok, i);
            if(!ok) {
                _a[i].first = key;
                _size++;
                if(_size > _capacity * _load_factor){
                    _rehash_2x();
                    HASH_PROBE_OVERRIDE(key, ok, i);
                }
            }
            _a[i].second = std::forward<T>(value);
        }

        void insert(iterator begin, iterator end){
            for(auto it = begin; it != end; ++it){
                emplace(it->first, it->second);
            }
        }

        iterator find(StrName key) const{
            HASH_PROBE(key, ok, i);
            if(!ok) return end();
            return iterator(this, i);
        }

        void erase(StrName key){
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found");
            _a[i] = NameDictNode();
            _size--;
        }

        inline iterator begin() const { return iterator(this, 0); }
        inline iterator end() const { return iterator(this, _capacity); }

    #undef HASH_PROBE
    #undef HASH_PROBE_OVERRIDE
    };

} // namespace pkpy