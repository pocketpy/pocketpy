// emhash5::HashMap for C++11/14/17
// version 2.1.2
// https://github.com/ktprime/ktprime/blob/master/hash_table5.hpp
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2023 Huang Yuanbing & bailuzhou AT 163.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE

#pragma once

#include <cstring>
#include <string>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <cassert>
#include <utility>
#include <cstdint>
#include <functional>
#include <iterator>
#include <algorithm>

#if EMH_WY_HASH
    #include "wyhash.h"
#endif

#ifdef EMH_KEY
    #undef  EMH_KEY
    #undef  EMH_VAL
    #undef  EMH_PKV
    #undef  EMH_BUCKET
    #undef  EMH_NEW
    #undef  EMH_EMPTY
    #undef  EMH_PREVET
#endif

// likely/unlikely
#if (__GNUC__ >= 4 || __clang__)
#    define EMH_LIKELY(condition)   __builtin_expect(condition, 1)
#    define EMH_UNLIKELY(condition) __builtin_expect(condition, 0)
#else
#    define EMH_LIKELY(condition)   condition
#    define EMH_UNLIKELY(condition) condition
#endif

#ifndef EMH_BUCKET_INDEX
    #define EMH_BUCKET_INDEX 1
#endif

#if EMH_BUCKET_INDEX == 0
    #define EMH_KEY(p,n)     p[n].second.first
    #define EMH_VAL(p,n)     p[n].second.second
    #define EMH_BUCKET(p,n)  p[n].first
    #define EMH_PKV(p,n)     p[n].second
    #define EMH_NEW(key, val, bucket) new(_pairs + bucket) PairT(bucket, value_type(key, val)); _num_filled ++
#elif EMH_BUCKET_INDEX == 2
    #define EMH_KEY(p,n)     p[n].first.first
    #define EMH_VAL(p,n)     p[n].first.second
    #define EMH_BUCKET(p,n)  p[n].second
    #define EMH_PREVET(p,n)  *(size_type*)(&p[n].first.first)
    #define EMH_PKV(p,n)     p[n].first
    #define EMH_NEW(key, val, bucket) new(_pairs + bucket) PairT(value_type(key, val), bucket); _num_filled ++
#else
    #define EMH_KEY(p,n)     p[n].first
    #define EMH_VAL(p,n)     p[n].second
    #define EMH_BUCKET(p,n)  p[n].bucket
    #define EMH_PREVET(p,n)  *(size_type*)(&p[n].first)
    #define EMH_PKV(p,n)     p[n]
    #define EMH_NEW(key, val, bucket) new(_pairs + bucket) PairT(key, val, bucket); _num_filled ++
#endif

#define EMH_EMPTY(p, b) (0 > (int)EMH_BUCKET(p, b))

namespace emhash5 {

#if EMH_SIZE_TYPE_64BIT
    typedef uint64_t size_type;
    static constexpr size_type INACTIVE = 0 - 0x1ull;
#else
    typedef uint32_t size_type;
    const constexpr size_type INACTIVE = 0xFFFFFFFF;
#endif

#ifndef EMH_MALIGN
    static constexpr uint32_t EMH_MALIGN = 16;
#endif
static_assert(EMH_MALIGN >= 16 && (EMH_MALIGN & (EMH_MALIGN - 1)) == 0);

template <typename First, typename Second>
struct entry {
    using first_type =  First;
    using second_type = Second;
    entry(const First& key, const Second& val, size_type ibucket)
        :second(val), first(key)
    {
        bucket = ibucket;
    }

    entry(First&& key, Second&& val, size_type ibucket)
        :second(std::move(val)), first(std::move(key))
    {
        bucket = ibucket;
    }

    template<typename K, typename V>
    entry(K&& key, V&& val, size_type ibucket)
        :second(std::forward<V>(val)), first(std::forward<K>(key))
    {
        bucket = ibucket;
    }

    template<typename K, typename V>
    entry(K&& key, std::tuple<V> val, size_type ibucket)
        :second(std::get<1>(val)),
        first(std::forward<K>(key))
    {
        bucket = ibucket;
    }

    entry(const std::pair<First, Second>& pair)
        :second(pair.second), first(pair.first)
    {
        bucket = INACTIVE;
    }

    entry(std::pair<First, Second>&& pair)
        :second(std::move(pair.second)), first(std::move(pair.first))
    {
        bucket = INACTIVE;
    }

    entry(std::tuple<First, Second>&& tup)
        :second(std::move(std::get<2>(tup))), first(std::move(std::get<1>(tup)))
    {
        bucket = INACTIVE;
    }

    entry(const entry& rhs)
        :second(rhs.second), first(rhs.first)
    {
        bucket = rhs.bucket;
    }

    entry(entry&& rhs) noexcept
        :second(std::move(rhs.second)), first(std::move(rhs.first))
    {
        bucket = rhs.bucket;
    }

    entry& operator = (entry&& rhs) noexcept
    {
        second = std::move(rhs.second);
        bucket = rhs.bucket;
        first  = std::move(rhs.first);
        return *this;
    }

    entry& operator = (const entry& rhs)
    {
        second = rhs.second;
        bucket = rhs.bucket;
        first  = rhs.first;
        return *this;
    }

    bool operator == (const entry<First, Second>& p) const
    {
        return first == p.first && second == p.second;
    }

    bool operator == (const std::pair<First, Second>& p) const
    {
        return first == p.first && second == p.second;
    }

    void swap(entry<First, Second>& o)
    {
        std::swap(second, o.second);
        std::swap(first, o.first);
    }

#if EMH_ORDER_KV || EMH_SIZE_TYPE_64BIT
    First first; //long
    size_type bucket;
    Second second;//int
#else
    Second second;
    size_type bucket;
    First first;
#endif
};

/// A cache-friendly hash table with open addressing, linear/qua probing and power-of-two capacity
template <typename KeyT, typename ValueT, typename HashT = std::hash<KeyT>, typename EqT = std::equal_to<KeyT>>
class HashMap
{
#ifndef EMH_DEFAULT_LOAD_FACTOR
    constexpr static float EMH_DEFAULT_LOAD_FACTOR = 0.80f;
    constexpr static float EMH_MIN_LOAD_FACTOR     = 0.25f; //< 0.5
#endif
#if EMH_CACHE_LINE_SIZE < 32
    constexpr static uint32_t EMH_CACHE_LINE_SIZE  = 64;
#endif

public:
    typedef HashMap<KeyT, ValueT, HashT, EqT> htype;
    typedef std::pair<KeyT, ValueT>           value_type;

#if EMH_BUCKET_INDEX == 0
    typedef value_type                        value_pair;
    typedef std::pair<size_type, value_type>  PairT;
#elif EMH_BUCKET_INDEX == 2
    typedef value_type                        value_pair;
    typedef std::pair<value_type, size_type>  PairT;
#else
    typedef entry<KeyT, ValueT>               value_pair;
    typedef entry<KeyT, ValueT>               PairT;
#endif

    typedef KeyT   key_type;
    typedef ValueT val_type;
    typedef ValueT mapped_type;
    typedef HashT  hasher;
    typedef EqT    key_equal;
    typedef PairT&       reference;
    typedef const PairT& const_reference;

    class const_iterator;
    class iterator
    {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef std::ptrdiff_t            difference_type;
        typedef value_pair                value_type;

        typedef value_pair*               pointer;
        typedef value_pair&               reference;

        iterator() { _map = nullptr; _bucket = -1; }
        iterator(const htype* hash_map, size_type bucket) : _map(hash_map), _bucket(bucket) { }

        iterator& operator++()
        {
            goto_next_element();
            return *this;
        }

        iterator operator++(int)
        {
            auto old_index = _bucket;
            goto_next_element();
            return {_map, old_index};
        }

        reference operator*() const { return _map->EMH_PKV(_pairs, _bucket); }
        pointer operator->() const { return &(_map->EMH_PKV(_pairs, _bucket)); }

        bool operator==(const iterator& rhs) const { return _bucket == rhs._bucket; }
        bool operator!=(const iterator& rhs) const { return _bucket != rhs._bucket; }

        bool operator==(const const_iterator& rhs) const { return _bucket == rhs._bucket; }
        bool operator!=(const const_iterator& rhs) const { return _bucket != rhs._bucket; }

        size_type bucket() const { return _bucket; }

    private:
        void goto_next_element()
        {
            while ((int)_map->EMH_BUCKET(_pairs, ++_bucket) < 0);
        }

    public:
        const htype* _map;
        size_type _bucket;
    };

    class const_iterator
    {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef std::ptrdiff_t            difference_type;
        typedef value_pair                value_type;

        typedef const value_pair*         pointer;
        typedef const value_pair&         reference;

        //const_iterator() { }
        const_iterator(const iterator& proto) : _map(proto._map), _bucket(proto._bucket) { }
        const_iterator(const htype* hash_map, size_type bucket) : _map(hash_map), _bucket(bucket) { }

        const_iterator& operator++()
        {
            goto_next_element();
            return *this;
        }

        const_iterator operator++(int)
        {
            auto old_index = _bucket;
            goto_next_element();
            return {_map, old_index};
        }

        reference operator*() const { return _map->EMH_PKV(_pairs, _bucket); }
        pointer operator->() const { return &(_map->EMH_PKV(_pairs, _bucket)); }

        bool operator==(const const_iterator& rhs) const { return _bucket == rhs._bucket; }
        bool operator!=(const const_iterator& rhs) const { return _bucket != rhs._bucket; }

        size_type bucket() const { return _bucket; }

    private:
        void goto_next_element()
        {
            while ((int)_map->EMH_BUCKET(_pairs, ++_bucket) < 0);
        }

    public:
        const htype* _map;
        size_type _bucket;
    };

    void init(size_type bucket, float mlf = EMH_DEFAULT_LOAD_FACTOR) noexcept
    {
        _pairs = nullptr;
        _mask  = _num_buckets = 0;
        _num_filled = 0;
#if EMH_HIGH_LOAD
        _ehead = 0;
#endif
        max_load_factor(mlf);
        rehash(bucket);
    }

    HashMap(size_type bucket = 2, float mlf = EMH_DEFAULT_LOAD_FACTOR) noexcept
    {
        init(bucket, mlf);
    }

    HashMap(const HashMap& rhs) noexcept
    {
        if (rhs.load_factor() > EMH_MIN_LOAD_FACTOR) {
            _pairs = alloc_bucket(rhs._num_buckets);
            clone(rhs);
        } else {
            init(rhs._num_filled + 2, EMH_DEFAULT_LOAD_FACTOR);
            for (auto it = rhs.begin(); it != rhs.end(); ++it)
                insert_unique(it->first, it->second);
        }
    }

    HashMap(HashMap&& rhs) noexcept
    {
        init(0);
        *this = std::move(rhs);
    }

    HashMap(std::initializer_list<value_type> ilist) noexcept
    {
        init((size_type)ilist.size());
        for (auto it = ilist.begin(); it != ilist.end(); ++it)
            do_insert(*it);
    }

    template<class InputIt>
    HashMap(InputIt first, InputIt last, size_type bucket_count=4) noexcept
    {
        init(std::distance(first, last) + bucket_count);
        for (; first != last; ++first)
            emplace(*first);
    }

    HashMap& operator=(const HashMap& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        if (rhs.load_factor() < EMH_MIN_LOAD_FACTOR) {
            clear();
#if EMH_SMALL_SIZE
            if (_pairs != (PairT*)_small)
#endif
            free(_pairs);
            _pairs = nullptr;

            rehash(rhs._num_filled + 2);
            for (auto it = rhs.begin(); it != rhs.end(); ++it)
                insert_unique(it->first, it->second);
            return *this;
        }

        clearkv();
        if (_num_buckets < rhs._num_buckets || _num_buckets > 2 * rhs._num_buckets) {
#if EMH_SMALL_SIZE
            if (_pairs != (PairT*)_small)
#endif
            free(_pairs);
            _pairs = alloc_bucket(rhs._num_buckets);
        }

        clone(rhs);
        return *this;
    }

    HashMap& operator=(HashMap&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

#if EMH_SMALL_SIZE
        if (_pairs == (PairT*)_small || rhs._pairs == (PairT*)rhs._small) {
            clear();
            if (rhs.empty())
                return *this;
            if (_pairs != (PairT*)_small)
                free(_pairs);
            if (rhs._num_buckets > EMH_SMALL_SIZE)
                _pairs = alloc_bucket(rhs._num_buckets);
            clone(rhs);
            rhs.clear();
            return *this;
        }
#endif
        swap(rhs);
        rhs.clear();
        return *this;
    }

    template<typename Con>
    bool operator == (const Con& rhs) const noexcept
    {
        if (size() != rhs.size())
            return false;

        for (auto it = begin(), last = end(); it != last; ++it) {
            auto oi = rhs.find(it->first);
            if (oi == rhs.end() || it->second != oi->second)
                return false;
        }
        return true;
    }

    template<typename Con>
    bool operator != (const Con& rhs) const { return !(*this == rhs); }

    ~HashMap() noexcept
    {
        clearkv();
#if EMH_SMALL_SIZE
        if (_pairs != (PairT*)_small)
#endif
        free(_pairs);
    }

    void clone(const HashMap& rhs) noexcept
    {
        _hasher      = rhs._hasher;
//        _eq          = rhs._eq;
        _num_buckets = rhs._num_buckets;
        _num_filled  = rhs._num_filled;
        _mlf         = rhs._mlf;
        _last        = rhs._last;
        _mask        = rhs._mask;
#if EMH_HIGH_LOAD
        _ehead       = rhs._ehead;
#endif

        auto opairs  = rhs._pairs;

        if (is_copy_trivially())
            memcpy(_pairs, opairs, (_num_buckets + 2) * sizeof(PairT));
        else {
            for (size_type bucket = 0; bucket < _num_buckets; bucket++) {
                auto next_bucket = EMH_BUCKET(_pairs, bucket) = EMH_BUCKET(opairs, bucket);
                if ((int)next_bucket >= 0)
                    new(_pairs + bucket) PairT(opairs[bucket]);
#if EMH_HIGH_LOAD
                else if (next_bucket != INACTIVE)
                    EMH_PREVET(_pairs, bucket) = EMH_PREVET(opairs, bucket);
#endif
            }
            memcpy(_pairs + _num_buckets, opairs + _num_buckets, sizeof(PairT) * 2);
        }
    }

    void swap(HashMap& rhs) noexcept
    {
#if EMH_SMALL_SIZE
        if (_pairs == (PairT*)_small || rhs._pairs == (PairT*)rhs._small) {
            if (is_copy_trivially()) {
                char tmp[(EMH_SMALL_SIZE + 2) * sizeof(PairT)];
                memcpy(tmp,  _small, sizeof(tmp));
                memcpy(_small, rhs._small, sizeof(tmp));
                memcpy(rhs._small, tmp,  sizeof(tmp)); //copy once if only one small map
            } else {
                HashMap tmp(*this);
                *this = rhs;
                rhs = tmp;
                return;
            }
        }
#endif
        //      std::swap(_eq, rhs._eq);
        std::swap(_hasher, rhs._hasher);
        std::swap(_pairs, rhs._pairs);
        std::swap(_num_buckets, rhs._num_buckets);
        std::swap(_num_filled, rhs._num_filled);
        std::swap(_mask, rhs._mask);
        std::swap(_mlf, rhs._mlf);
        std::swap(_last, rhs._last);
#if EMH_HIGH_LOAD
        std::swap(_ehead, rhs._ehead);
#endif
    }

    // -------------------------------------------------------------
    iterator begin() noexcept
    {
        if (_num_filled == 0)
            return end();

        size_type bucket = 0;
        while (EMH_EMPTY(_pairs, bucket)) {
            ++bucket;
        }
        return {this, bucket};
    }

#if 0
    iterator last() noexcept
    {
        if (_num_filled == 0)
            return end();

        size_type bucket = _num_buckets - 1;
        while (EMH_EMPTY(_pairs, bucket)) bucket--;
        return {this, bucket};
    }
#endif

    const_iterator cbegin() const noexcept
    {
        if (_num_filled == 0)
            return end();

        size_type bucket = 0;
        while (EMH_EMPTY(_pairs, bucket)) {
            ++bucket;
        }
        return {this, bucket};
    }
    inline const_iterator begin() const noexcept { return cbegin(); }

    inline iterator end() noexcept { return {this, _num_buckets}; }
    inline const_iterator end()  const noexcept { return cend(); }
    inline const_iterator cend() const noexcept { return {this, _num_buckets}; }

    inline size_type size() const noexcept { return _num_filled; }
    inline bool empty() const noexcept { return _num_filled == 0; }
    inline size_type bucket_count() const noexcept { return _num_buckets; }

    inline HashT hash_function() const noexcept { return static_cast<const HashT&>(_hasher); }
    inline EqT key_eq() const noexcept { return static_cast<const EqT&>(_eq); }

    inline float load_factor() const noexcept { return static_cast<float>(_num_filled) / _num_buckets; }
    inline float max_load_factor() const noexcept { return (1 << 27) / (float)_mlf; }
    void max_load_factor(float ml) noexcept
    {
        if (ml < 0.991f && ml > EMH_MIN_LOAD_FACTOR)
            _mlf = (uint32_t)((1 << 27) / ml);
    }

    inline constexpr size_type max_size() const { return 1ull << (sizeof(size_type) * 8 - 1); }
    inline constexpr size_type max_bucket_count() const { return max_size(); }

#if EMH_STATIS
    //Returns the bucket number where the element with key k is located.
    size_type bucket_slot(const KeyT& key) const
    {
        const auto bucket = key_to_bucket(key);
        const auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0)
            return 0;
        else if (bucket == next_bucket)
            return bucket + 1;

        return hash_main(bucket) + 1;
    }

    //Returns the number of elements in bucket n.
    size_type bucket_size(const size_type bucket) const
    {
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0)
            return 0;

        next_bucket = hash_main(bucket);
        size_type ibucket_size = 1;

        //iterator each item in current main bucket
        while (true) {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (nbucket == next_bucket) {
                break;
            }
            ibucket_size ++;
            next_bucket = nbucket;
        }
        return ibucket_size;
    }

    size_type get_main_bucket(const uint32_t bucket) const
    {
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0)
            return INACTIVE;

        return hash_main(bucket);
    }

    size_type get_diss(uint32_t bucket, uint32_t next_bucket, const uint32_t slots) const
    {
        auto pbucket = reinterpret_cast<uint64_t>(&_pairs[bucket]);
        auto pnext   = reinterpret_cast<uint64_t>(&_pairs[next_bucket]);
        if (pbucket / EMH_CACHE_LINE_SIZE == pnext / EMH_CACHE_LINE_SIZE)
            return 0;
        uint32_t diff = pbucket > pnext ? (pbucket - pnext) : (pnext - pbucket);
        if (diff / EMH_CACHE_LINE_SIZE < slots - 1)
            return diff / EMH_CACHE_LINE_SIZE + 1;
        return slots - 1;
    }

    int get_bucket_info(const uint32_t bucket, uint32_t steps[], const uint32_t slots) const
    {
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0)
            return -1;

        const auto main_bucket = hash_main(bucket);
        if (next_bucket == main_bucket)
            return 1;
        else if (main_bucket != bucket)
            return 0;

        steps[get_diss(bucket, next_bucket, slots)] ++;
        uint32_t ibucket_size = 2;
        //find a new empty and linked it to tail
        while (true) {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (nbucket == next_bucket)
                break;

            steps[get_diss(nbucket, next_bucket, slots)] ++;
            ibucket_size ++;
            next_bucket = nbucket;
        }
        return ibucket_size;
    }

    void dump_statics() const
    {
        const int slots = 128;
        uint32_t buckets[slots + 1] = {0};
        uint32_t steps[slots + 1]   = {0};
        for (uint32_t bucket = 0; bucket < _num_buckets; ++bucket) {
            auto bsize = get_bucket_info(bucket, steps, slots);
            if (bsize > 0)
                buckets[bsize] ++;
        }

        uint32_t sumb = 0, collision = 0, sumc = 0, finds = 0, sumn = 0;
        puts("============== buckets size ration =========");
        for (uint32_t i = 0; i < sizeof(buckets) / sizeof(buckets[0]); i++) {
            const auto bucketsi = buckets[i];
            if (bucketsi == 0)
                continue;
            sumb += bucketsi;
            sumn += bucketsi * i;
            collision += bucketsi * (i - 1);
            finds += bucketsi * i * (i + 1) / 2;
            printf("  %2u  %8u  %2.2lf|  %.2lf\n", i, bucketsi, bucketsi * 100.0 * i / _num_filled, sumn * 100.0 / _num_filled);
        }

        puts("========== collision miss ration ===========");
        for (uint32_t i = 0; i < sizeof(steps) / sizeof(steps[0]); i++) {
            sumc += steps[i];
            if (steps[i] <= 2)
                continue;
            printf("  %2u  %8u  %.2lf  %.2lf\n", i, steps[i], steps[i] * 100.0 / collision, sumc * 100.0 / collision);
        }

        if (sumb == 0)  return;
        printf("    _num_filled/bucket_size/packed collision/cache_miss/hit_find = %u/%.2lf/%d/ %.2lf%%/%.2lf%%/%.2lf\n",
                _num_filled, _num_filled * 1.0 / sumb, int(sizeof(PairT)), (collision * 100.0 / _num_filled), (collision - steps[0]) * 100.0 / _num_filled, finds * 1.0 / _num_filled);
        assert(sumc == collision);
        assert(sumn == _num_filled);
        puts("============== buckets size end =============");
    }
#endif

    // ------------------------------------------------------------
    template<typename K=KeyT>
    inline iterator find(const K& key) noexcept
    {
        return {this, find_filled_bucket(key)};
    }

    template<typename K=KeyT>
    inline const_iterator find(const K& key) const noexcept
    {
        return {this, find_filled_bucket(key)};
    }

    template<typename K=KeyT>
    inline iterator find(const K& key, size_type key_hash) noexcept
    {
        return {this, find_hash_bucket(key, key_hash)};
    }

    template<typename K=KeyT>
    inline const_iterator find(const K& key, size_type key_hash) const noexcept
    {
        return {this, find_hash_bucket(key, key_hash)};
    }

    template<typename K=KeyT>
    inline ValueT& at(const K& key)
    {
        const auto bucket = find_filled_bucket(key);
        //throw
        return EMH_VAL(_pairs, bucket);
    }

    template<typename K=KeyT>
    inline const ValueT& at(const K& key) const
    {
        const auto bucket = find_filled_bucket(key);
        return EMH_VAL(_pairs, bucket);
    }

    template<typename K=KeyT>
    ValueT& at(const K& key, size_type key_hash)
    {
        const auto bucket = find_hash_bucket(key, key_hash);
        return EMH_VAL(_pairs, bucket);
    }

    template<typename K=KeyT>
    const ValueT& at(const K& key, size_type key_hash) const
    {
        const auto bucket = find_hash_bucket(key, key_hash);
        return EMH_VAL(_pairs, bucket);
    }

    template<typename K=KeyT>
    inline bool contains(const K& key) const noexcept
    {
        return find_filled_bucket(key) != _num_buckets;
    }

    template<typename K=KeyT>
    inline bool contains(const K& key, size_type key_hash) const noexcept
    {
        return find_hash_bucket(key, key_hash) != _num_buckets;
    }

    template<typename K=KeyT>
    inline size_type count(const K& key) const noexcept
    {
        return find_filled_bucket(key) == _num_buckets ? 0 : 1;
    }

    template<typename K=KeyT>
    inline size_type count(const K& key, size_type key_hash) const noexcept
    {
        return find_hash_bucket(key, key_hash) == _num_buckets ? 0 : 1;
    }

    template<typename K=KeyT>
    std::pair<iterator, iterator> equal_range(const K& key) noexcept
    {
        const auto found = find(key);
        if (found.bucket() == _num_buckets)
            return { found, found };
        else
            return { found, std::next(found) };
    }

    template<typename K=KeyT>
    std::pair<const_iterator, const_iterator> equal_range(const K& key) const
    {
        const auto found = find(key);
        if (found.bucket() == _num_buckets)
            return { found, found };
        else
            return { found, std::next(found) };
    }

    void merge(HashMap& rhs)
    {
        if (_num_filled == 0) {
            *this = std::move(rhs);
            return;
        }

        for (auto rit = rhs.begin(); rit != rhs.end(); ) {
            auto fit = find(rit->first);
            if (fit.bucket() == _num_buckets) {
                insert_unique(rit->first, std::move(rit->second));
                rit = rhs.erase(rit);
            } else {
                ++rit;
            }
        }
    }

#ifdef EMH_EXT
    /// Return the old value or ValueT() if it didn't exist.
    ValueT set_get(const KeyT& key, const ValueT& val)
    {
        check_expand_need();
        const auto bucket = find_or_allocate(key);

        if (EMH_EMPTY(_pairs, bucket)) {
            EMH_NEW(key, val, bucket);
            return ValueT();
        } else {
            ValueT old_value(val);
            std::swap(EMH_VAL(_pairs, bucket), old_value);
            return old_value;
        }
    }

    /// Returns the matching ValueT or nullptr if k isn't found.
    bool try_get(const KeyT& key, ValueT& val) const
    {
        const auto bucket = find_filled_bucket(key);
        const auto found = bucket != _num_buckets;
        if (found) {
            val = EMH_VAL(_pairs, bucket);
        }
        return found;
    }

    /// Returns the matching ValueT or nullptr if k isn't found.
    inline ValueT* try_get(const KeyT& key)
    {
        const auto bucket = find_filled_bucket(key);
        return bucket != _num_buckets ? &EMH_VAL(_pairs, bucket) : nullptr;
    }

    /// Const version of the above
    inline ValueT* try_get(const KeyT& key) const
    {
        const auto bucket = find_filled_bucket(key);
        return bucket != _num_buckets ? &EMH_VAL(_pairs, bucket) : nullptr;
    }

    /// set value if key exist
    bool try_set(const KeyT& key, const ValueT& val)
    {
        const auto bucket = find_filled_bucket(key);
        if (bucket == _num_buckets)
            return false;

        EMH_VAL(_pairs, bucket) = val;
        return true;
    }

    /// set value if key exist
    bool try_set(const KeyT& key, ValueT&& val)
    {
        const auto bucket = find_filled_bucket(key);
        if (bucket == _num_buckets)
            return false;

        EMH_VAL(_pairs, bucket) = std::move(val);
        return true;
    }

    /// Convenience function.
    ValueT get_or_return_default(const KeyT& key) const
    {
        const auto bucket = find_filled_bucket(key);
        return bucket == _num_buckets ? ValueT() : EMH_VAL(_pairs, bucket);
    }
#endif

    // -----------------------------------------------------
#if EMH_BUCKET_INDEX == 1
    std::pair<iterator, bool> do_insert(const value_pair& value)
    {
        const auto bucket = find_or_allocate(value.first);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
            EMH_NEW(value.first, value.second, bucket);
        }
        return { {this, bucket}, bempty };
    }
#endif

    std::pair<iterator, bool> do_insert(const value_type& value) noexcept
    {
        const auto bucket = find_or_allocate(value.first);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
            EMH_NEW(value.first, value.second, bucket);
        }
        return { {this, bucket}, bempty };
    }

    std::pair<iterator, bool> do_insert(value_type&& value) noexcept
    {
        const auto bucket = find_or_allocate(value.first);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
            EMH_NEW(std::move(value.first), std::move(value.second), bucket);
        }
        return { {this, bucket}, bempty };
    }

    template<typename K, typename V>
    std::pair<iterator, bool> do_insert(K&& key, V&& val) noexcept
    {
        const auto bucket = find_or_allocate(key);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
            EMH_NEW(std::forward<K>(key), std::forward<V>(val), bucket);
        }
        return { {this, bucket}, bempty };
    }

    template<typename K, typename V>
    std::pair<iterator, bool> do_assign(K&& key, V&& val) noexcept
    {
        check_expand_need();
        const auto bucket = find_or_allocate(key);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
            EMH_NEW(std::forward<K>(key), std::forward<V>(val), bucket);
        } else {
            EMH_VAL(_pairs, bucket) = std::forward<V>(val);
        }
        return { {this, bucket}, bempty };
    }

    std::pair<iterator, bool> insert(const value_type& value) noexcept
    {
        check_expand_need();
        return do_insert(value);
    }

    std::pair<iterator, bool> insert(value_type&& value) noexcept
    {
        check_expand_need();
        return do_insert(std::move(value));
    }

    template< typename P >
    std::pair<iterator, bool> insert(P&& value) noexcept
    {
        check_expand_need();
        return do_insert(std::forward<P>(value));
    }

    iterator insert(const_iterator hint, const value_type& value)
    {
        if (hint.bucket() != _num_buckets && hint->first == value.first) {
            return {this, hint.bucket()};
        }

        check_expand_need();
        return do_insert(value).first;
    }

    iterator insert(const_iterator hint, value_type&& value)
    {
        if (hint.bucket() != _num_buckets && hint->first == value.first) {
            return {this, hint.bucket()};
        }

        check_expand_need();
        return do_insert(std::move(value)).first;
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        reserve(ilist.size() + _num_filled);
        for (auto it = ilist.begin(); it != ilist.end(); ++it)
            do_insert(*it);
    }

    template <typename Iter>
    void insert(Iter first, Iter last)
    {
        reserve(std::distance(first, last) + _num_filled);
        for (; first != last; ++first)
            emplace(*first);
    }

    //assert(bucket < _num_buckets)
    ValueT* find_hint(const KeyT& key, size_t bucket)
    {
        if (!EMH_EMPTY(_pairs, bucket) && EMH_KEY(_pairs, bucket) == key)
            return &EMH_VAL(_pairs, bucket);
        return nullptr;
    }

#if 0
    template <typename Iter>
    void insert_unique(Iter begin, Iter end) noexcept
    {
        reserve(std::distance(begin, end) + _num_filled);
        for (; begin != end; ++begin) {
            insert_unique(*begin);
        }
    }
#endif

    template<typename K, typename V>
    size_type insert_unique(K&& key, V&& val) noexcept
    {
        check_expand_need();
        auto bucket = find_unique_bucket(key);
        EMH_NEW(std::forward<K>(key), std::forward<V>(val), bucket);
        return bucket;
    }

    inline size_type insert_unique(value_type&& value) noexcept
    {
        return insert_unique(std::move(value.first), std::move(value.second));
    }

    inline size_type insert_unique(const value_type& value)
    {
        return insert_unique(value.first, value.second);
    }

    template <class... Args>
    inline size_type emplace_unique(Args&&... args) noexcept
    {
        return insert_unique(std::forward<Args>(args)...);
    }

    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        check_expand_need();
        return do_insert(std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args)
    {
        value_type value(std::forward<Args>(args)...);
        auto bucket = hint.bucket();
        if (bucket != _num_buckets && hint->first == value.first) {
            return {this, bucket};
        }

        check_expand_need();
        return do_insert(std::move(value)).first;
    }

    //TODO: fix tuple
    template<class... Args>
    std::pair<iterator, bool> try_emplace(const KeyT& key, Args&&... args)
    {
        check_expand_need();
        const auto bucket = find_or_allocate(key);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
//            EMH_NEW(key, std::forward_as_tuple(std::forward<Args>(args)...), bucket);
        }
        return { {this, bucket}, bempty };
    }

    template<class... Args>
    std::pair<iterator, bool> try_emplace(KeyT&& key, Args&&... args)
    {
        check_expand_need();
        const auto bucket = find_or_allocate(key);
        const auto bempty = EMH_EMPTY(_pairs, bucket);
        if (bempty) {
//            EMH_NEW(std::move(key), std::forward_as_tuple(std::forward<Args>(args)...), bucket);
        }
        return { {this, bucket}, bempty };
    }

    template<class... Args>
    iterator try_emplace(const_iterator hint, const KeyT& key, Args&&... args)
    {
        (void)hint;
        return try_emplace(key, std::forward<Args>(args)...).first;
    }

    template<class... Args>
    iterator try_emplace(const_iterator hint, KeyT&& key, Args&&... args)
    {
        (void)hint;
        return try_emplace(std::move(key), std::forward<Args>(args)...).first;
    }

    template <class M>
    std::pair<iterator, bool> insert_or_assign(const KeyT& key, M&& val) noexcept { return do_assign(key, std::forward<M>(val)); }
    template <class M>
    std::pair<iterator, bool> insert_or_assign(KeyT&& key, M&& val) noexcept { return do_assign(std::move(key), std::forward<M>(val)); }

    template <class M>
    iterator insert_or_assign(const_iterator hint, const KeyT& key, M&& val) {
        auto bucket = hint.bucket();
        if (bucket != _num_buckets && hint->first == key) {
            hint->second = std::forward<M>(val);
            return {this, bucket};
        }

        return do_assign(key, std::forward<M>(val)).first;
    }

    template <class M>
    iterator insert_or_assign(const_iterator hint, KeyT&& key, M&& val) {
        auto bucket = hint.bucket();
        if (bucket != _num_buckets && hint->first == key) {
            EMH_VAL(_pairs, bucket) = std::forward<M>(val);
            return {this, bucket};
        }

        return do_assign(std::move(key), std::forward<M>(val)).first;
    }

    /// Like std::map<KeyT, ValueT>::operator[].
    ValueT& operator[](const KeyT& key) noexcept
    {
        check_expand_need();
        const auto bucket = find_or_allocate(key);
        if (EMH_EMPTY(_pairs, bucket)) {
            /* Check if inserting a new value rather than overwriting an old entry */
            EMH_NEW(key, std::move(ValueT()), bucket);
        }

        return EMH_VAL(_pairs, bucket);
    }

    ValueT& operator[](KeyT&& key) noexcept
    {
        check_expand_need();
        const auto bucket = find_or_allocate(key);
        if (EMH_EMPTY(_pairs, bucket)) {
            EMH_NEW(std::move(key), std::move(ValueT()), bucket);
        }

        return EMH_VAL(_pairs, bucket);
    }

    // -------------------------------------------------------
    /// return 0 if not erase
#if 0
    size_type erase_node(const KeyT& key, const size_type slot)
    {
        if (slot < _num_buckets && _pairs[slot].second != INACTIVE && _pairs[slot].first == key) {
            erase_bucket(slot);
            return 1;
        }
        return erase(key);
    }
#endif

    /// Erase an element from the hash table.
    /// return 0 if element was not found
    size_type erase(const KeyT& key) noexcept
    {
        const auto bucket = erase_key(key);
        if (bucket == INACTIVE)
            return 0;

        clear_bucket(bucket);
        return 1;
    }

#if 0
    template <typename K=KeyT>
    size_type erase(K&& key)
    {
        const auto bucket = erase_key(key);
        if ((int)bucket < 0)
            return 0;

        clear_bucket(bucket);
        return 1;
    }
#endif

    //iterator erase(const_iterator begin_it, const_iterator end_it)
    iterator erase(const_iterator cit) noexcept
    {
        const auto bucket = erase_bucket(cit._bucket);
        clear_bucket(bucket);

        iterator it(this, cit._bucket);
        //erase from main bucket, return main bucket as next
        return (bucket == it._bucket) ? ++it : it;
    }

    void _erase(const_iterator it) noexcept
    {
        const auto bucket = erase_bucket(it._bucket);
        clear_bucket(bucket);
    }

    void _erase(iterator it) noexcept
    {
        const auto bucket = erase_bucket(it._bucket);
        clear_bucket(bucket);
    }

    template<typename Pred>
    size_type erase_if(Pred pred)
    {
        auto old_size = size();
        for (auto it = begin(), last = end(); it != last; ) {
            if (pred(*it))
                it = erase(it);
            else
                ++it;
        }
        return old_size - size();
    }

    static constexpr bool is_triviall_destructable()
    {
#if __cplusplus >= 201402L || _MSC_VER > 1600
        return !(std::is_trivially_destructible<KeyT>::value && std::is_trivially_destructible<ValueT>::value);
#else
        return !(std::is_pod<KeyT>::value && std::is_pod<ValueT>::value);
#endif
    }

    static constexpr bool is_copy_trivially()
    {
#if __cplusplus >= 201103L || _MSC_VER > 1600
        return (std::is_trivially_copyable<KeyT>::value && std::is_trivially_copyable<ValueT>::value);
#else
        return (std::is_pod<KeyT>::value && std::is_pod<ValueT>::value);
#endif
    }

    void clearkv() noexcept
    {
        if (is_triviall_destructable()) {
            for (size_type bucket = 0; _num_filled > 0; ++bucket) {
                if (!EMH_EMPTY(_pairs, bucket))
                    clear_bucket(bucket, false);
            }
        }
    }

#if EMH_FIND_HIT
    void reset_bucket(size_type bucket)
    {
        if constexpr (std::is_integral<KeyT>::value) {
            auto& key = EMH_KEY(_pairs, bucket); key = KeyT(0-2);
//            if (bucket != _zero_index)                 return;
            while (key_to_bucket(key) == bucket) key += 1610612741;
        }
    }
#endif

    /// Remove all elements, keeping full capacity.
    void clear() noexcept
    {
#if EMH_HIGH_LOAD
        if (_ehead > 0)
            clear_empty();
        clearkv();
#else
        if (is_triviall_destructable())
            clearkv();
        else if (_num_filled)
            memset((char*)_pairs, INACTIVE, sizeof(_pairs[0]) * _num_buckets);
#endif
#if EMH_FIND_HIT
        if constexpr (std::is_integral<KeyT>::value)
        reset_bucket(hash_main(0));
#endif

        _last = _num_filled = 0;
    }

    void shrink_to_fit(const float min_factor = EMH_DEFAULT_LOAD_FACTOR / 4)
    {
        if (load_factor() < min_factor) //safe guard
            rehash(_num_filled + 1);
    }

    /// Make room for this many elements
    bool reserve(uint64_t num_elems)
    {
#if EMH_HIGH_LOAD < 1000
#if EMH_PACK_TAIL
        const auto required_buckets = 1 + (size_type)(num_elems * _mlf >> 27);
        if (EMH_LIKELY(required_buckets < _num_buckets))
#else
        const auto required_buckets = (size_type)(num_elems * _mlf >> 27);
        if (EMH_LIKELY(required_buckets < _mask))
#endif
            return false;
#else
        const auto required_buckets = (size_type)(num_elems + num_elems * 1 / 9);
        if (EMH_LIKELY(required_buckets < _mask))
            return false;

        else if (_num_buckets < 16 && _num_filled < _num_buckets)
            return false;

        else if (_num_buckets > EMH_HIGH_LOAD) {
            if (_ehead == 0) {
                set_empty();
                return false;
            } else if (/*_num_filled + 100 < _num_buckets && */EMH_BUCKET(_pairs, _ehead) != 0-_ehead) {
                return false;
            }
        }
#endif

#if EMH_STATIS
        if (_num_filled > EMH_STATIS) dump_statics();
#endif

        //assert(required_buckets < max_size());
        rehash(required_buckets + 2);
        return true;
    }

    void rehash(uint64_t required_buckets)
    {
        if (required_buckets < _num_filled)
            return;

#if EMH_SMALL_SIZE
        uint64_t buckets = _num_filled > (1u << 16) ? (1u << 16) : EMH_SMALL_SIZE;
        static_assert(EMH_SMALL_SIZE >= 2 && EMH_SMALL_SIZE < 1024);
        static_assert((EMH_SMALL_SIZE & (EMH_SMALL_SIZE - 1)) == 0);
#else
        uint64_t buckets = _num_filled > (1u << 16) ? (1u << 16) : 2;
#endif
        while (buckets < required_buckets) { buckets *= 2; }

        // no need alloc too many bucket for small key.
        // if maybe fail set small load_factor and then call reserve() TODO:
        if (sizeof(KeyT) < sizeof(size_type) && buckets >= (1ul << (2 * 8)))
            buckets = 2ul << (sizeof(KeyT) * 8);

        assert(buckets < max_size() && buckets > _num_filled);

        auto num_buckets = (size_type)buckets;
        auto old_num_filled  = _num_filled;
        auto* old_pairs   = _pairs;
        auto old_buckets = _num_buckets;

#if EMH_REHASH_LOG
        auto omask = _mask;
        auto last = _last;
        size_type collision = 0;
#endif
#if EMH_HIGH_LOAD
        _ehead = 0;
#endif

        _num_filled  = 0;
        _mask        = num_buckets - 1;
        _last        = num_buckets / 4;

#if EMH_PACK_TAIL > 1 && EMH_PACK_TAIL <= 100
        _last = num_buckets;
        num_buckets += num_buckets * EMH_PACK_TAIL / 100; //add more 5-10%
#endif
        _num_buckets = num_buckets;

#if EMH_SMALL_SIZE
        if (num_buckets <= EMH_SMALL_SIZE && old_pairs != (PairT*)_small)
            _pairs = (PairT*)_small;
        else
#endif
        _pairs = (PairT*)alloc_bucket(num_buckets);
        memset((char*)_pairs, INACTIVE, sizeof(_pairs[0]) * num_buckets);
        memset((char*)(_pairs + num_buckets), 0, sizeof(PairT) * 2);

#if EMH_FIND_HIT
        if constexpr (std::is_integral<KeyT>::value)
        reset_bucket(hash_main(0));
#endif

        (void)old_buckets;
        if (0 && is_copy_trivially() && old_num_filled && num_buckets >= 2 * old_buckets) {
            memcpy((char*)_pairs, old_pairs, old_buckets * sizeof(PairT));
            for (size_type src_bucket = 0; src_bucket < old_buckets; src_bucket++) {
                if (EMH_EMPTY(_pairs, src_bucket))
                    continue;

                _num_filled ++;
                auto nbucket = hash_main(src_bucket);
                if (nbucket < old_buckets)
                    continue;

                auto bucket = move_unique_bucket(src_bucket, nbucket);
                _pairs[bucket] = std::move(_pairs[src_bucket]);
                erase_bucket(src_bucket);
                if ((int)EMH_BUCKET(_pairs, src_bucket) >= 0)
                    src_bucket --;

                EMH_BUCKET(_pairs, bucket) = bucket;
            }
        } else {
            //for (size_type src_bucket = 0; _num_filled < old_num_filled; src_bucket++) {
            for (size_type src_bucket = old_buckets - 1; _num_filled < old_num_filled; src_bucket--) {
                if (EMH_EMPTY(old_pairs, src_bucket))
                    continue;
#if EMH_REHASH_LOG
                else if (src_bucket != EMH_BUCKET(old_pairs, src_bucket))
                    collision ++;
#endif

                const auto& key = EMH_KEY(old_pairs, src_bucket);
                const auto bucket = find_unique_bucket(key);
                new(_pairs + bucket) PairT(std::move(old_pairs[src_bucket])); _num_filled ++;
                EMH_BUCKET(_pairs, bucket) = bucket;
                if (is_triviall_destructable())
                    old_pairs[src_bucket].~PairT();
            }
        }

#if EMH_REHASH_LOG
        if (_num_filled > EMH_REHASH_LOG) {
            auto mbucket = _num_filled - collision;
            char buff[255] = {0};
            sprintf(buff, "    _num_filled/aver_size/K.V/pack/collision|last = %u/%.2lf/%s.%s/%zd|%.2lf%%,%.2lf%%",
                    _num_filled, double (_num_filled) / mbucket, typeid(KeyT).name(), typeid(ValueT).name(),
                    sizeof(_pairs[0]), collision * 100.0 / _num_filled, last * 100.0 / omask);
#ifdef EMH_LOG
            static uint32_t ihashs = 0; EMH_LOG() << "hash_nums = " << ihashs ++ << "|" <<__FUNCTION__ << "|" << buff << endl;
#else
            puts(buff);
#endif
        }
#endif
#if EMH_SMALL_SIZE
        if (old_pairs != (PairT*)_small)
#endif
        free(old_pairs);
        assert(old_num_filled == _num_filled);
    }

private:

    static PairT* alloc_bucket(size_type num_buckets)
    {
        //TODO: call realloc
#ifdef EMH_ALLOC
        auto* new_pairs = (PairT*)aligned_alloc(EMH_MALIGN, (2 + num_buckets) * sizeof(PairT));
#else
        auto* new_pairs = (PairT*)malloc((2 + num_buckets) * sizeof(PairT));
#endif
        return new_pairs;
    }

#if EMH_HIGH_LOAD
    void set_empty()
    {
        auto prev = 0;
        for (int32_t bucket = 1; bucket < _num_buckets; ++bucket) {
            if (EMH_EMPTY(_pairs, bucket)) {
                if (prev != 0) {
                    EMH_PREVET(_pairs, bucket) = prev;
                    EMH_BUCKET(_pairs, prev) = -bucket;
                }
                else
                    _ehead = bucket;
                prev = bucket;
            }
        }

        EMH_PREVET(_pairs, _ehead) = prev;
        EMH_BUCKET(_pairs, prev) = 0-_ehead;
        _ehead = 0-EMH_BUCKET(_pairs, _ehead);
    }

    void clear_empty()
    {
        auto prev = EMH_PREVET(_pairs, _ehead);
        while (prev != _ehead) {
            EMH_BUCKET(_pairs, prev) = INACTIVE;
            prev = EMH_PREVET(_pairs, prev);
        }
        EMH_BUCKET(_pairs, _ehead) = INACTIVE;
        _ehead = 0;
    }

    //prev-ehead->next
    size_type pop_empty(const size_type bucket)
    {
        const auto prev_bucket = EMH_PREVET(_pairs, bucket);
        int next_bucket = (int)(0-EMH_BUCKET(_pairs, bucket));
//        assert(next_bucket > 0 && _ehead > 0);
//        assert(next_bucket <= _mask && prev_bucket <= _mask);

        EMH_PREVET(_pairs, next_bucket) = prev_bucket;
        EMH_BUCKET(_pairs, prev_bucket) = -next_bucket;

        _ehead = next_bucket;
        return bucket;
    }

    //ehead->bucket->next
    void push_empty(const int32_t bucket)
    {
        const int next_bucket = 0-EMH_BUCKET(_pairs, _ehead);
        assert(next_bucket > 0);

        EMH_PREVET(_pairs, bucket) = _ehead;
        EMH_BUCKET(_pairs, bucket) = -next_bucket;

        EMH_PREVET(_pairs, next_bucket) = bucket;
        EMH_BUCKET(_pairs, _ehead) = -bucket;
        //        _ehead = bucket;
    }
#endif

    // Can we fit another element?
    inline bool check_expand_need()
    {
        return reserve(_num_filled);
    }

    void clear_bucket(size_type bucket, bool bclear = true) noexcept
    {
        if (is_triviall_destructable()) {
            //EMH_BUCKET(_pairs, bucket) = INACTIVE; //loop call in destructor
            _pairs[bucket].~PairT();
        }
        EMH_BUCKET(_pairs, bucket) = INACTIVE; //the status is reset by destructor by some compiler
        _num_filled--;

#if EMH_HIGH_LOAD
        if (_ehead && bclear) {
            if (10 * _num_filled < 8 * _num_buckets)
                clear_empty();
            else if (bucket)
                push_empty(bucket);
        }
#endif
#if EMH_FIND_HIT
        reset_bucket(bucket);
#endif
    }

    template <typename K=KeyT>
    size_type erase_key(const K& key) noexcept
    {
        const auto bucket = key_to_bucket(key);
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if (EMH_UNLIKELY((int)next_bucket < 0))
            return INACTIVE;

        const auto equalk = _eq(key, EMH_KEY(_pairs, bucket));
#if 1
        if (next_bucket == bucket)
            return equalk ? bucket : INACTIVE;
        else if (equalk) {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            EMH_PKV(_pairs, bucket) = std::move(EMH_PKV(_pairs, next_bucket));
            EMH_BUCKET(_pairs, bucket) = (nbucket == next_bucket) ? bucket : nbucket;
            return next_bucket;
        }/* else if (EMH_UNLIKELY(bucket != hash_main(bucket)))
            return INACTIVE;
        */
#else
        if (equalk) {
            if (next_bucket != bucket) {
                const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
                EMH_PKV(_pairs, bucket) = std::move(EMH_PKV(_pairs, next_bucket));
                EMH_BUCKET(_pairs, bucket) = (nbucket == next_bucket) ? bucket : nbucket;
            }
            return next_bucket;
        } else if (next_bucket == bucket)
            return INACTIVE;
#endif

        auto prev_bucket = bucket;
        while (true) {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (_eq(key, EMH_KEY(_pairs, next_bucket))) {
#ifndef EMH_RNEXT
                EMH_BUCKET(_pairs, prev_bucket) = (nbucket == next_bucket) ? prev_bucket : nbucket;
                return next_bucket;
#else
                if (nbucket == next_bucket) {
                    EMH_BUCKET(_pairs, prev_bucket) = prev_bucket;
                    return next_bucket;
                }

                const auto last = EMH_BUCKET(_pairs, nbucket);
                EMH_PKV(_pairs, next_bucket) = std::move(EMH_PKV(_pairs, nbucket));
                EMH_BUCKET(_pairs, next_bucket) = (nbucket == last) ? next_bucket : last;
                return nbucket;
#endif
            }

            if (nbucket == next_bucket)
                break;
            prev_bucket = next_bucket;
            next_bucket = nbucket;
        }

        return INACTIVE;
    }

    size_type erase_bucket(const size_type bucket) noexcept
    {
#if 1
        const auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if (EMH_LIKELY(next_bucket == bucket)) {
            const auto main_bucket = hash_main(bucket);
            if (main_bucket != bucket) {
                const auto prev_bucket = find_prev_bucket(main_bucket, bucket);
                EMH_BUCKET(_pairs, prev_bucket) = prev_bucket;
            }
        } else {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            EMH_PKV(_pairs, bucket) = std::move(EMH_PKV(_pairs, next_bucket));
            EMH_BUCKET(_pairs, bucket) = (nbucket == next_bucket) ? bucket : nbucket;
        }

        return next_bucket;
#else
        const auto next_bucket = EMH_BUCKET(_pairs, bucket);
        const auto main_bucket = hash_main(bucket);
        if (bucket == main_bucket) {
            if (bucket != next_bucket) {
                const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
                EMH_PKV(_pairs, bucket) = std::move(EMH_PKV(_pairs, next_bucket));
                EMH_BUCKET(_pairs, bucket) = (nbucket == next_bucket) ? bucket : nbucket;
            }
            return next_bucket;
        }

        const auto prev_bucket = find_prev_bucket(main_bucket, bucket);
        EMH_BUCKET(_pairs, prev_bucket) = (bucket == next_bucket) ? prev_bucket : next_bucket;
        return bucket;
#endif
    }

    template<typename K=KeyT>
    inline size_type find_filled_bucket(const K& key) const noexcept
    {
        return find_hash_bucket(key, hash_key(key));
    }

    // Find the bucket with this key, or return bucket size
    template<typename K=KeyT>
    size_type find_hash_bucket(const K& key, size_type key_hash) const noexcept
    {
        const auto bucket = key_hash & _mask;
        auto next_bucket = EMH_BUCKET(_pairs, bucket);

#if EMH_FIND_HIT == 0
        if ((int)next_bucket < 0)
            return _num_buckets;
        else if (_eq(key, EMH_KEY(_pairs, bucket)))
            return bucket;
#else
        if constexpr (std::is_integral<KeyT>::value) {
            if (_eq(key, EMH_KEY(_pairs, bucket)))
                return bucket;
            else if ((int)next_bucket < 0)
                return _num_buckets;
        } else {
            if ((int)next_bucket < 0)
                return _num_buckets;
            else if (_eq(key, EMH_KEY(_pairs, bucket)))
                return bucket;
        }
#endif

        if (next_bucket == bucket)
            return _num_buckets;
//        else if (key_to_bucket(EMH_KEY(_pairs, bucket)) != bucket)
//            return _num_buckets;

        while (true) {
            if (_eq(key, EMH_KEY(_pairs, next_bucket)))
                return next_bucket;

            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (nbucket == next_bucket)
                return _num_buckets;
            next_bucket = nbucket;
        }

        return 0;
    }

    //kick out bucket and find empty to occpuy
    //it will break the orgin link and relnik again.
    //before: main_bucket-->prev_bucket --> kbucket   --> next_bucket
    //after : main_bucket-->prev_bucket --> (removed)--> new_bucket(kbucket)--> next_bucket
    size_type kickout_bucket(const size_type kmain, const size_type kbucket) noexcept
    {
        const auto next_bucket = EMH_BUCKET(_pairs, kbucket);
        const auto new_bucket  = find_empty_bucket(next_bucket, 2);
        const auto prev_bucket = find_prev_bucket(kmain, kbucket);
        EMH_BUCKET(_pairs, prev_bucket) = new_bucket;
        new(_pairs + new_bucket) PairT(std::move(_pairs[kbucket]));
        if (next_bucket == kbucket)
            EMH_BUCKET(_pairs, new_bucket) = new_bucket;

        clear_bucket(kbucket, false);
        _num_filled ++;
        return kbucket;
    }

/*
** inserts a new key into a hash table; first, check whether key's main
** bucket/position is free. If not, check whether colliding node/bucket is in its main
** position or not: if it is not, move colliding bucket to an empty place and
** put new key in its main position; otherwise (colliding bucket is in its main
** position), new key goes to an empty position.
*/
    template<typename K=KeyT>
    size_type find_or_allocate(const K& key) noexcept
    {
        const auto bucket = key_to_bucket(key);
        const auto& bucket_key = EMH_KEY(_pairs, bucket);
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0) {
#if EMH_HIGH_LOAD
            if (next_bucket != INACTIVE)
                pop_empty(bucket);
#endif
            return bucket;
        } else if (_eq(key, bucket_key))
            return bucket;

        //check current bucket_key is in main bucket or not
        const auto kmain = key_to_bucket(bucket_key);
        if (kmain != bucket)
            return kickout_bucket(kmain, bucket);
        else if (next_bucket == bucket)
            return EMH_BUCKET(_pairs, next_bucket) = find_empty_bucket(next_bucket, 1);

        int csize = 0;
#if EMH_LRU_SET
        auto prev_bucket = bucket;
#endif
        //find next linked bucket and check key
        while (true) {
            if (_eq(key, EMH_KEY(_pairs, next_bucket))) {
#if EMH_LRU_SET
                EMH_PKV(_pairs, next_bucket).swap(EMH_PKV(_pairs, prev_bucket));
                return prev_bucket;
#else
                return next_bucket;
#endif
            }

#if EMH_LRU_SET
            prev_bucket = next_bucket;
#endif

            csize += 1;
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (nbucket == next_bucket)
                break;
            next_bucket = nbucket;
        }

        //find a new empty and link it to tail
        const auto new_bucket = find_empty_bucket(next_bucket, csize);
        return EMH_BUCKET(_pairs, next_bucket) = new_bucket;
    }

    template<typename K=KeyT>
    size_type find_unique_bucket(const K& key) noexcept
    {
        const auto bucket = key_to_bucket(key);
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0) {
#if EMH_HIGH_LOAD
            if (next_bucket != INACTIVE)
                pop_empty(bucket);
#endif
            return bucket;
        }

        //check current bucket_key is in main bucket or not
        const auto kmain = hash_main(bucket);
        if (EMH_UNLIKELY(kmain != bucket))
            return kickout_bucket(kmain, bucket);
        else if (EMH_UNLIKELY(next_bucket != bucket))
            next_bucket = find_last_bucket(next_bucket);

        //find a new empty and link it to tail
        return EMH_BUCKET(_pairs, next_bucket) = find_unique_empty(next_bucket);
    }

    size_type move_unique_bucket(size_type old_bucket, size_type bucket) noexcept
    {
        (void)old_bucket;
        auto next_bucket = EMH_BUCKET(_pairs, bucket);
        if ((int)next_bucket < 0)
            return bucket;

        next_bucket = find_last_bucket(next_bucket);

        //find a new empty and link it to tail
        return EMH_BUCKET(_pairs, next_bucket) = find_unique_empty(next_bucket);
    }

/***
  Different probing techniques usually provide a trade-off between memory locality and avoidance of clustering.
Since Robin Hood hashing is relatively resilient to clustering (both primary and secondary), linear probing the most cache-friendly alternative is typically used.

    It's the core algorithm of this hash map with highly optimization/benchmark.
normaly linear probing is inefficient with high load factor, it use a new 3-way linear
probing strategy to search empty slot. from benchmark even the load factor > 0.9, it's more 2-3 timer fast than
one-way search strategy.

1. linear or quadratic probing a few cache line for less cache miss from input slot "bucket_from".
2. the first  search  slot from member variant "_last", init with 0
3. the second search slot from calculated pos "(_num_filled + _last) & _mask", it's like a rand value
*/
    // key is not in this map. Find a place to put it.
    size_type find_empty_bucket(const size_type bucket_from, uint32_t csize) noexcept
    {
#if EMH_HIGH_LOAD
        if (_ehead)
            return pop_empty(_ehead);
#endif

        auto bucket = bucket_from;
        if (EMH_EMPTY(_pairs, ++bucket) || EMH_EMPTY(_pairs, ++bucket))
            return bucket;

#ifndef _MSC_VER
        //__builtin_prefetch(static_cast<const void*>(_pairs + bucket + 1), 0, 1);
#endif
        constexpr auto linear_probe_length = 5;//2-3 cache line miss
        for (size_type step = 2, slot = bucket + 1 + csize / 2; ; slot += step++) {
            if (step < linear_probe_length) {
                auto bucket1 = slot & _mask;
                if (EMH_EMPTY(_pairs, bucket1) || EMH_EMPTY(_pairs, ++bucket1))
                    return bucket1;
            } else { //if (step++ > 5) {
                if (EMH_EMPTY(_pairs, ++_last))// || EMH_EMPTY(_pairs, _last++))
                    return _last;

                _last &= _mask;
#if EMH_PACK_TAIL
                auto tail = _num_buckets - _last;
                if (EMH_EMPTY(_pairs, tail) || EMH_EMPTY(_pairs, ++tail))
                    return tail;
#else
                auto medium = (_num_buckets / 2 + _last) & _mask;
                if (EMH_EMPTY(_pairs, medium))// && EMH_EMPTY(_pairs, ++medium))
                    return _last = medium;
#endif
            }
        }

        return 0;
    }

    size_type find_unique_empty(const size_type bucket_from) noexcept
    {
        auto bucket = bucket_from;
        if (EMH_EMPTY(_pairs, ++bucket) || EMH_EMPTY(_pairs, ++bucket))
            return bucket;

        for (size_type slot = bucket + 2, step = 2; ; slot += ++step) {
            auto nbucket = slot & _mask;
            if (EMH_EMPTY(_pairs, nbucket) || EMH_EMPTY(_pairs, ++nbucket))
                return nbucket;
        }

        return 0;
    }

    size_type find_last_bucket(size_type main_bucket) const noexcept
    {
        auto next_bucket = EMH_BUCKET(_pairs, main_bucket);
        if (next_bucket == main_bucket)
            return main_bucket;

        while (true) {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (nbucket == next_bucket)
                return next_bucket;
            next_bucket = nbucket;
        }
    }

    size_type find_prev_bucket(const size_type main_bucket, const size_type bucket) const noexcept
    {
        auto next_bucket = EMH_BUCKET(_pairs, main_bucket);
        if (next_bucket == bucket)
            return main_bucket;

        while (true) {
            const auto nbucket = EMH_BUCKET(_pairs, next_bucket);
            if (nbucket == bucket)
                return next_bucket;
            next_bucket = nbucket;
        }
    }

    template<typename K=KeyT>
    inline size_type key_to_bucket(const K& key) const noexcept
    {
        return (size_type)hash_key(key) & _mask;
    }

    inline size_type hash_main(const size_type bucket) const noexcept
    {
        return (size_type)hash_key(EMH_KEY(_pairs, bucket)) & _mask;
    }

#if EMH_INT_HASH
    static constexpr uint64_t KC = UINT64_C(11400714819323198485);
    inline uint64_t hash64(uint64_t key)
    {
#if __SIZEOF_INT128__ && EMH_INT_HASH == 1
        __uint128_t r = key; r *= KC;
        return (uint64_t)(r >> 64) + (uint64_t)r;
#elif EMH_INT_HASH == 2
        //MurmurHash3Mixer
        uint64_t h = key;
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccd;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53;
        h ^= h >> 33;
        return h;
#elif _WIN64 && EMH_INT_HASH == 1
        uint64_t high;
        return _umul128(key, KC, &high) + high;
#elif EMH_INT_HASH == 3
        auto ror  = (key >> 32) | (key << 32);
        auto low  = key * 0xA24BAED4963EE407ull;
        auto high = ror * 0x9FB21C651E98DF25ull;
        auto mix  = low + high;
        return mix;
#elif EMH_INT_HASH == 1
        uint64_t r = key * UINT64_C(0xca4bcaa75ec3f625);
        return (r >> 32) + r;
#elif EMH_WYHASH64
        return wyhash64(key, KC);
#else
        uint64_t x = key;
        x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
        x = x ^ (x >> 31);
        return x;
#endif
    }
#endif

    template<typename UType, typename std::enable_if<std::is_integral<UType>::value, size_type>::type = 0>
    inline size_type hash_key(const UType key) const
    {
#if EMH_INT_HASH
        return hash64(key);
#elif EMH_IDENTITY_HASH
        return key + (key >> 24);
#else
        return (size_type)_hasher(key);
#endif
    }

    template<typename UType, typename std::enable_if<std::is_same<UType, std::string>::value, size_type>::type = 0>
    inline size_type hash_key(const UType& key) const
    {
#if EMH_WY_HASH
        return (size_type)wyhash(key.data(), key.size(), 0);
#else
        return (size_type)_hasher(key);
#endif
    }

    template<typename UType, typename std::enable_if<!std::is_integral<UType>::value && !std::is_same<UType, std::string>::value, size_type>::type = 0>
    inline size_type hash_key(const UType& key) const
    {
        return (size_type)_hasher(key);
    }

private:
    PairT*    _pairs;
#if EMH_SMALL_SIZE
    char      _small[(EMH_SMALL_SIZE + 2) * sizeof(PairT)];
#endif

    HashT     _hasher;
    EqT       _eq;
    uint32_t  _mlf;
    size_type _mask;
    size_type _num_buckets;
    size_type _num_filled;
    size_type _last;
#if EMH_HIGH_LOAD
    size_type _ehead;
#endif
};
} // namespace emhash

//#define ehmap emhash5::HashMap
#if __cplusplus > 199711
//template <class Key, class Val> using emhash5 = ehmap<Key, Val, std::hash<Key>>;
#endif