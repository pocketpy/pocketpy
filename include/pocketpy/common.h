#pragma once

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
#include <initializer_list>
#include <variant>
#include <type_traits>

#define PK_VERSION				"1.0.9"

#include "config.h"
#include "export.h"

/*******************************************************************************/
#if PK_ENABLE_STD_FUNCTION
#include <functional>
#endif
/*******************************************************************************/

#if PK_ENABLE_THREAD
#define THREAD_LOCAL thread_local
#include <mutex>

struct GIL {
	inline static std::mutex _mutex;
    explicit GIL() { _mutex.lock(); }
    ~GIL() { _mutex.unlock(); }
};
#define PK_GLOBAL_SCOPE_LOCK() auto _lock = GIL();

#else
#define THREAD_LOCAL
#define PK_GLOBAL_SCOPE_LOCK()
#endif

/*******************************************************************************/

#define PK_UNUSED(x) (void)(x)

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

#define PK_LAMBDA(x) ([](VM* vm, ArgsView args) { return x; })
#define PK_VAR_LAMBDA(x) ([](VM* vm, ArgsView args) { return VAR(x); })
#define PK_ACTION(x) ([](VM* vm, ArgsView args) { x; return vm->None; })

#ifdef POCKETPY_H
#define FATAL_ERROR() throw std::runtime_error( "L" + std::to_string(__LINE__) + " FATAL_ERROR()!");
#else
#define FATAL_ERROR() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " FATAL_ERROR()!");
#endif

#define PK_ASSERT(x) if(!(x)) FATAL_ERROR();

struct PyObject;
#define PK_BITS(p) (reinterpret_cast<i64>(p))
inline bool is_tagged(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) != 0b00; }
inline bool is_int(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b01; }
inline bool is_float(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b10; }
inline bool is_special(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b11; }

inline bool is_both_int_or_float(PyObject* a, PyObject* b) noexcept {
    return is_tagged(a) && is_tagged(b);
}

inline bool is_both_int(PyObject* a, PyObject* b) noexcept {
    return is_int(a) && is_int(b);
}

inline bool is_both_float(PyObject* a, PyObject* b) noexcept {
	return is_float(a) && is_float(b);
}

// special singals, is_tagged() for them is true
inline PyObject* const PY_NULL = (PyObject*)0b000011;		// tagged null
inline PyObject* const PY_OP_CALL = (PyObject*)0b100011;
inline PyObject* const PY_OP_YIELD = (PyObject*)0b110011;

#define ADD_MODULE_PLACEHOLDER(name) namespace pkpy { inline void add_module_##name(void* vm) { (void)vm; } }

} // namespace pkpy

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#elif __EMSCRIPTEN__

#include <emscripten.h>

#elif __unix__

#include <dlfcn.h>

#endif