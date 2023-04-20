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
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <random>
#include <initializer_list>
#include <variant>
#include <type_traits>

#define PK_VERSION				"0.9.9"

// debug macros
#define DEBUG_NO_BUILTIN_MODULES	0
#define DEBUG_EXTRA_CHECK			0
#define DEBUG_DIS_EXEC				0
#define DEBUG_CEVAL_STEP			0
#define DEBUG_CEVAL_STEP_MIN		0
#define DEBUG_FULL_EXCEPTION		0
#define DEBUG_MEMORY_POOL			0
#define DEBUG_NO_MEMORY_POOL		0
#define DEBUG_NO_AUTO_GC			0
#define DEBUG_GC_STATS				0

#if (defined(__ANDROID__) && __ANDROID_API__ <= 22) || defined(__EMSCRIPTEN__)
#define PK_ENABLE_FILEIO 			0
#else
#define PK_ENABLE_FILEIO 			0	// TODO: refactor this
#endif

// This is the maximum number of arguments in a function declaration
// including positional arguments, keyword-only arguments, and varargs
#define PK_MAX_CO_VARNAMES			255

#if _MSC_VER
#define PK_ENABLE_COMPUTED_GOTO		0
#define UNREACHABLE()				__assume(0)
#else
#define PK_ENABLE_COMPUTED_GOTO		1
#define UNREACHABLE()				__builtin_unreachable()

#if DEBUG_CEVAL_STEP || DEBUG_CEVAL_STEP_MIN
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
};

template <>
struct NumberTraits<8> {
	using int_t = int64_t;
	using float_t = double;

	template<typename... Args>
	static int_t stoi(Args&&... args) { return std::stoll(std::forward<Args>(args)...); }
	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stod(std::forward<Args>(args)...); }
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
struct Discarded { };

struct Type {
	int index;
	Type(): index(-1) {}
	Type(int index): index(index) {}
	bool operator==(Type other) const noexcept { return this->index == other.index; }
	bool operator!=(Type other) const noexcept { return this->index != other.index; }
	operator int() const noexcept { return this->index; }
};

#define THREAD_LOCAL	// thread_local
#define CPP_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })
#define CPP_NOT_IMPLEMENTED() ([](VM* vm, ArgsView args) { vm->NotImplementedError(); return vm->None; })

#ifdef POCKETPY_H
#define FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

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
inline PyObject* const PY_NULL = (PyObject*)0b000011;
inline PyObject* const PY_BEGIN_CALL = (PyObject*)0b010011;
inline PyObject* const PY_OP_CALL = (PyObject*)0b100011;
inline PyObject* const PY_OP_YIELD = (PyObject*)0b110011;

struct Expr;
typedef std::unique_ptr<Expr> Expr_;

} // namespace pkpy