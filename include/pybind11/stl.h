#include "pybind11.h"

#include <array>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>

#include <map>
#include <unordered_map>

namespace pkbind {

template <typename T, std::size_t N>
struct type_caster<std::array<T, N>> {
    std::array<T, N> data;

    template <typename U>
    static handle cast(U&& src, return_value_policy policy, handle parent) {
        auto list = pkbind::list();
        for(auto&& item: src) {
            list.append(pkbind::cast(std::move(item), policy, parent));
        }
        return list;
    }

    bool load(handle src, bool convert) {
        if(!isinstance<list>(src)) { return false; }
        auto list = src.cast<pkbind::list>();

        if(list.size() != N) { return false; }

        for(int i = 0; i < N; ++i) {
            type_caster<T> caster;
            if(!caster.load(list[i], convert)) { return false; }
            data[i] = std::move(caster.value());
        }

        return true;
    }

    std::array<T, N>& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
constexpr bool is_py_list_like_v = false;

template <typename T, typename Allocator>
constexpr bool is_py_list_like_v<std::vector<T, Allocator>> = true;

template <typename T, typename Allocator>
constexpr bool is_py_list_like_v<std::list<T, Allocator>> = true;

template <typename T, typename Allocator>
constexpr bool is_py_list_like_v<std::deque<T, Allocator>> = true;

template <typename T>
struct type_caster<T, std::enable_if_t<is_py_list_like_v<T>>> {
    T data;

    template <typename U>
    static object cast(U&& src, return_value_policy policy, handle parent) {
        auto list = pkbind::list();
        for(auto&& item: src) {
            if constexpr(std::is_same_v<T, std::vector<bool>>) {
                list.append(pkbind::cast(bool(item), policy, parent));
            } else {
                list.append(pkbind::cast(std::move(item), policy, parent));
            }
        }
        return list;
    }

    bool load(handle src, bool convert) {
        if(!isinstance<list>(src)) { return false; }

        auto list = src.cast<pkbind::list>();

        for(auto item: list) {
            type_caster<typename T::value_type> caster;
            if(!caster.load(item, convert)) { return false; }
            data.push_back(std::move(caster.value()));
        }

        return true;
    }

    T& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

template <typename T>
constexpr bool is_py_map_like_v = false;

template <typename Key, typename T, typename Compare, typename Allocator>
constexpr bool is_py_map_like_v<std::map<Key, T, Compare, Allocator>> = true;

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
constexpr bool is_py_map_like_v<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> = true;

template <typename T>
struct type_caster<T, std::enable_if_t<is_py_map_like_v<T>>> {
    T data;

    template <typename U>
    static object cast(U&& src, return_value_policy policy, handle parent) {
        auto dict = pkbind::dict();
        for(auto&& [key, value]: src) {
            dict[pkbind::cast(std::move(key), policy, parent)] =
                pkbind::cast(std::move(value), policy, parent);
        }
        return dict;
    }

    bool load(handle src, bool convert) {
        if(!isinstance<dict>(src)) { return false; }
        auto dict = src.cast<pkbind::dict>();

        for(auto item: dict) {
            type_caster<typename T::key_type> key_caster;
            if(!key_caster.load(item.first, convert)) { return false; }

            type_caster<typename T::mapped_type> value_caster;
            if(!value_caster.load(item.second, convert)) { return false; }

            data.try_emplace(std::move(key_caster.value()), std::move(value_caster.value()));
        }

        return true;
    }

    T& value() { return data; }

    constexpr inline static bool is_temporary_v = true;
};

}  // namespace pkbind
