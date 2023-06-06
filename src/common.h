#pragma once

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4101)
#pragma warning (disable:4244)
#define _CRT_NONSTDC_NO_DEPRECATE
#define strdup _strdup
#endif

#include <cmath>
#include <cstring>

#include <sstream>
#include <regex>
#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>
#include <string_view>
#include <iomanip>
#include <memory>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <random>
#include <initializer_list>
#include <variant>
#include <type_traits>

#define PK_VERSION				"1.0.2"

// debug macros
#define DEBUG_NO_BUILTIN_MODULES    0
#define DEBUG_EXTRA_CHECK           0
#define DEBUG_DIS_EXEC              0
#define DEBUG_CEVAL_STEP            0
#define DEBUG_FULL_EXCEPTION        0
#define DEBUG_MEMORY_POOL           0
#define DEBUG_NO_MEMORY_POOL        0
#define DEBUG_NO_AUTO_GC            0
#define DEBUG_GC_STATS              0

// config macros
#ifndef PK_ENABLE_OS

#ifdef __ANDROID__
#include <android/ndk-version.h>

#if __NDK_MAJOR__ <= 22
#define PK_ENABLE_OS 			0
#else
#define PK_ENABLE_OS 			1
#endif

#else
#define PK_ENABLE_OS 			1
#endif

#endif

#if PK_ENABLE_THREAD
#define THREAD_LOCAL thread_local
#else
#define THREAD_LOCAL
#endif

/*******************************************************************************/

// This is the maximum number of arguments in a function declaration
// including positional arguments, keyword-only arguments, and varargs
#define PK_MAX_CO_VARNAMES			255

#if _MSC_VER
#define PK_ENABLE_COMPUTED_GOTO		0
#define UNREACHABLE()				__assume(0)
#else
#define PK_ENABLE_COMPUTED_GOTO		1
#define UNREACHABLE()				__builtin_unreachable()

#if DEBUG_CEVAL_STEP
#undef PK_ENABLE_COMPUTED_GOTO
#endif

#endif

namespace pkpy{

namespace std = ::std;

template <size_t T>
struct NumberTraits;

template <>
struct NumberTraits<4> {
	using int_t = int32_t;
	using float_t = float;

	template<typename... Args>
	static int_t stoi(Args&&... args) { return std::stoi(std::forward<Args>(args)...); }
	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stof(std::forward<Args>(args)...); }

	static constexpr int_t c0 = 0b00000000011111111111111111111100;
	static constexpr int_t c1 = 0b11111111111111111111111111111100;
	static constexpr int_t c2 = 0b00000000000000000000000000000011;
};

template <>
struct NumberTraits<8> {
	using int_t = int64_t;
	using float_t = double;

	template<typename... Args>
	static int_t stoi(Args&&... args) { return std::stoll(std::forward<Args>(args)...); }
	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stod(std::forward<Args>(args)...); }

	static constexpr int_t c0 = 0b0000000000001111111111111111111111111111111111111111111111111100;
	static constexpr int_t c1 = 0b1111111111111111111111111111111111111111111111111111111111111100;
	static constexpr int_t c2 = 0b0000000000000000000000000000000000000000000000000000000000000011;
};

using Number = NumberTraits<sizeof(void*)>;
using i64 = Number::int_t;
using f64 = Number::float_t;

static_assert(sizeof(i64) == sizeof(void*));
static_assert(sizeof(f64) == sizeof(void*));
static_assert(std::numeric_limits<f64>::is_iec559);

struct Dummy { };
struct DummyInstance { };
struct DummyModule { };
struct NoReturn { };
struct Discarded { };

struct Type {
	int index;
	Type(): index(-1) {}
	Type(int index): index(index) {}
	bool operator==(Type other) const noexcept { return this->index == other.index; }
	bool operator!=(Type other) const noexcept { return this->index != other.index; }
	operator int() const noexcept { return this->index; }
};

#define CPP_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })

#ifdef POCKETPY_H
#define FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

#define PK_ASSERT(x) if(!(x)) FATAL_ERROR();

inline const float kInstAttrLoadFactor = 0.67f;
inline const float kTypeAttrLoadFactor = 0.5f;

struct PyObject;
#define BITS(p) (reinterpret_cast<i64>(p))
inline bool is_tagged(PyObject* p) noexcept { return (BITS(p) & 0b11) != 0b00; }
inline bool is_int(PyObject* p) noexcept { return (BITS(p) & 0b11) == 0b01; }
inline bool is_float(PyObject* p) noexcept { return (BITS(p) & 0b11) == 0b10; }
inline bool is_special(PyObject* p) noexcept { return (BITS(p) & 0b11) == 0b11; }

inline bool is_both_int_or_float(PyObject* a, PyObject* b) noexcept {
    return is_tagged(a) && is_tagged(b);
}

inline bool is_both_int(PyObject* a, PyObject* b) noexcept {
    return is_int(a) && is_int(b);
}

// special singals, is_tagged() for them is true
inline PyObject* const PY_NULL = (PyObject*)0b000011;		// tagged null
inline PyObject* const PY_OP_CALL = (PyObject*)0b100011;
inline PyObject* const PY_OP_YIELD = (PyObject*)0b110011;

#ifdef _WIN32
    inline const char kPlatformSep = '\\';
#else
    inline const char kPlatformSep = '/';
#endif

} // namespace pkpy