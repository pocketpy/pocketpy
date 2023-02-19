#pragma once

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4101)
#define _CRT_NONSTDC_NO_DEPRECATE
#define strdup _strdup
#endif

#include <sstream>
#include <regex>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <string_view>
#include <queue>
#include <iomanip>
#include <memory>
#include <functional>
#include <iostream>
// #include <filesystem>
// namespace fs = std::filesystem;

#include "hash_table8.hpp"

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()!");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

#define PK_VERSION "0.8.9"

#if defined(__EMSCRIPTEN__) || defined(__arm__) || defined(PK_32_BIT)
typedef int32_t i64;
typedef float f64;
const i64 kMinSafeInt = -((i64)1 << 30);
const i64 kMaxSafeInt = ((i64)1 << 30) - 1;
#else
typedef int64_t i64;
typedef double f64;
const i64 kMinSafeInt = -((i64)1 << 62);
const i64 kMaxSafeInt = ((i64)1 << 62) - 1;
#endif

struct Dummy { char _; };
#define DUMMY_VAL Dummy()

struct Type {
	int index;
	Type(): index(-1) {}
	Type(int index): index(index) {}
	inline bool operator==(Type other) const noexcept {
		return this->index == other.index;
	}
	inline bool operator!=(Type other) const noexcept {
		return this->index != other.index;
	}
};

template<typename T>
void* tid() {
	static volatile int8_t _x;
	return (void*)(&_x);
}

//#define THREAD_LOCAL thread_local
#define THREAD_LOCAL

#define RAW(T) std::remove_const_t<std::remove_reference_t<T>>