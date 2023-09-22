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
#include <random>

#define PK_VERSION				"1.2.2"

#include "config.h"
#include "export.h"

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
	using float_t = float;

	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stof(std::forward<Args>(args)...); }
};

template <>
struct NumberTraits<8> {
	using int_t = int64_t;
	using float_t = double;

	template<typename... Args>
	static float_t stof(Args&&... args) { return std::stod(std::forward<Args>(args)...); }
};

using Number = NumberTraits<sizeof(void*)>;
using i64 = int64_t;		// always 64-bit
using f64 = Number::float_t;

template<size_t T>
union BitsCvtImpl;

template<>
union BitsCvtImpl<4>{
	NumberTraits<4>::int_t _int;
	NumberTraits<4>::float_t _float;

	struct{
		unsigned int sign: 1;
		unsigned int exp: 8;
		uint64_t mantissa: 23;
	} _float_bits;

	static constexpr int C0 = 127;	// 2^7 - 1
	static constexpr int C1 = -62;	// 2 - 2^6
	static constexpr int C2 = 63;	// 2^6 - 1

	BitsCvtImpl(NumberTraits<4>::float_t val): _float(val) {}
	BitsCvtImpl(NumberTraits<4>::int_t val): _int(val) {}
};

template<>
union BitsCvtImpl<8>{
	NumberTraits<8>::int_t _int;
	NumberTraits<8>::float_t _float;

	struct{
		unsigned int sign: 1;
		unsigned int exp: 11;
		uint64_t mantissa: 52;
	} _float_bits;

	static constexpr int C0 = 1023;	// 2^10 - 1
	static constexpr int C1 = -510;	// 2 - 2^9
	static constexpr int C2 = 511;	// 2^9 - 1

	BitsCvtImpl(NumberTraits<8>::float_t val): _float(val) {}
	BitsCvtImpl(NumberTraits<8>::int_t val): _int(val) {}
};

using BitsCvt = BitsCvtImpl<sizeof(void*)>;

static_assert(sizeof(i64) == 8);
static_assert(sizeof(Number::float_t) == sizeof(void*));
static_assert(sizeof(Number::int_t) == sizeof(void*));
static_assert(sizeof(BitsCvt) == sizeof(void*));
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
#define PK_BITS(p) (reinterpret_cast<Number::int_t>(p))

inline PyObject* tag_float(f64 val){
	BitsCvt decomposed(val);
	unsigned int exp_7b = decomposed._float_bits.exp;
	if(exp_7b - BitsCvt::C0 < BitsCvt::C1){
		exp_7b = 0;
		decomposed._float_bits.mantissa = 0;
	}else if(exp_7b - BitsCvt::C0 > BitsCvt::C2){
		exp_7b = BitsCvt::C0;
		if(!std::isnan(val)) decomposed._float_bits.mantissa = 0;
	}
	decomposed._float_bits.exp = exp_7b - BitsCvt::C0 + BitsCvt::C2;
	decomposed._int = (decomposed._int << 1) | 0b01;
	return reinterpret_cast<PyObject*>(decomposed._int);
}

inline f64 untag_float(PyObject* val){
	BitsCvt decomposed(reinterpret_cast<Number::int_t>(val));
	decomposed._int >>= 1;
	unsigned int exp_7b = decomposed._float_bits.exp;
	if(exp_7b == 0) return 0.0f;
	if(exp_7b == BitsCvt::C0){
		decomposed._float_bits.exp = -1;
		return decomposed._float;
	}
	decomposed._float_bits.exp = exp_7b - BitsCvt::C2 + BitsCvt::C0;
	return decomposed._float;
}

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

} // namespace pkpy
