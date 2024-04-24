#pragma once

#include <cmath>
#include <cstring>
#include <ctime>

#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>
#include <string_view>
#include <memory>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <variant>
#include <type_traits>
#include <deque>
#include <typeindex>
#include <initializer_list>

#define PK_VERSION				"1.4.6"

#include "config.h"
#include "export.h"

#include "_generated.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

/*******************************************************************************/
#if PK_ENABLE_STD_FUNCTION
#include <functional>
#endif
/*******************************************************************************/

#if PK_ENABLE_THREAD
#define PK_THREAD_LOCAL thread_local
#include <mutex>

struct GIL {
	inline static std::mutex _mutex;
    explicit GIL() { _mutex.lock(); }
    ~GIL() { _mutex.unlock(); }
};
#define PK_GLOBAL_SCOPE_LOCK() GIL _lock;

#else
#define PK_THREAD_LOCAL
#define PK_GLOBAL_SCOPE_LOCK()
#endif

/*******************************************************************************/

#define PK_UNUSED(x) (void)(x)

#define PK_LOCAL_STATIC static

namespace pkpy{

namespace std = ::std;

template <size_t T>
struct NumberTraits;

template <>
struct NumberTraits<4> {
	using int_t = int32_t;
	static constexpr int_t kMaxSmallInt = (1 << 28) - 1;
	static constexpr int_t kMinSmallInt = 0;
};

template <>
struct NumberTraits<8> {
	using int_t = int64_t;
	static constexpr int_t kMaxSmallInt = (1ll << 60) - 1;
	static constexpr int_t kMinSmallInt = 0;
};

using Number = NumberTraits<sizeof(void*)>;
using i64 = int64_t;		// always 64-bit
using f64 = double;			// always 64-bit

static_assert(sizeof(i64) == 8);

struct Dummy { }; // for special objects: True, False, None, Ellipsis, etc.
struct DummyInstance { };
struct DummyModule { };
struct NoReturn { };
struct Discarded { };

struct Type {
	int index;
	constexpr Type(): index(-1) {}
	constexpr Type(int index): index(index) {}
	bool operator==(Type other) const { return this->index == other.index; }
	bool operator!=(Type other) const { return this->index != other.index; }
	operator int() const { return this->index; }
};

#define PK_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })
#define PK_VAR_LAMBDA(x) ([](VM* vm, ArgsView args) { return VAR(x); })
#define PK_ACTION(x) ([](VM* vm, ArgsView args) { x; return vm->None; })

#ifdef POCKETPY_H
#define PK_FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define PK_FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

#define PK_ASSERT(x) if(!(x)) PK_FATAL_ERROR();

#if PK_DEBUG_EXTRA_CHECK
#define PK_DEBUG_ASSERT(x) if(!(x)) PK_FATAL_ERROR();
#else
#define PK_DEBUG_ASSERT(x)
#endif

struct PyObject;
#define PK_BITS(p) (reinterpret_cast<i64>(p))
#define PK_SMALL_INT(val) (reinterpret_cast<PyObject*>(val << 2 | 0b10))

// is_pod<> for c++17 and c++20
template<typename T>
struct is_pod {
	static constexpr bool value = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;
};

#define PK_ALWAYS_PASS_BY_POINTER(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete; \
	T(T&&) = delete; \
	T& operator=(T&&) = delete;

inline const char* kPlatformStrings[] = {
    "win32",        // 0
    "emscripten",   // 1
    "ios",          // 2
    "darwin",       // 3
    "android",      // 4
    "linux",        // 5
    "unknown"       // 6
};

#define PK_SLICE_LOOP(i, start, stop, step) for(int i=start; step>0?i<stop:i>stop; i+=step)

template<typename T>
inline constexpr bool is_integral_v = std::is_same_v<T, char>
        || std::is_same_v<T, short>
        || std::is_same_v<T, int>
        || std::is_same_v<T, long>
        || std::is_same_v<T, long long>
        || std::is_same_v<T, unsigned char>
        || std::is_same_v<T, unsigned short>
        || std::is_same_v<T, unsigned int>
        || std::is_same_v<T, unsigned long>
        || std::is_same_v<T, unsigned long long>
		|| std::is_same_v<T, signed char>;		// for imgui

template<typename T>
inline constexpr bool is_floating_point_v = std::is_same_v<T, float> || std::is_same_v<T, double>;

inline const char* PK_HEX_TABLE = "0123456789abcdef";

} // namespace pkpy
