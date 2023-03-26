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
#include <map>
#include <set>
#include <algorithm>
#include <random>
#include <chrono>

#define PK_VERSION				"0.9.5"
#define PK_EXTRA_CHECK 			1

#if (defined(__ANDROID__) && __ANDROID_API__ <= 22) || defined(__EMSCRIPTEN__)
#define PK_ENABLE_FILEIO 		0
#else
#define PK_ENABLE_FILEIO 		1
#endif

#if defined(__EMSCRIPTEN__) || defined(__arm__) || defined(__i386__)
typedef int32_t i64;
typedef float f64;
#define S_TO_INT std::stoi
#define S_TO_FLOAT std::stof
#else
typedef int64_t i64;
typedef double f64;
#define S_TO_INT std::stoll
#define S_TO_FLOAT std::stod
#endif

namespace pkpy{

namespace std = ::std;

struct Dummy {  };
struct DummyInstance {  };
struct DummyModule { };
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

//#define THREAD_LOCAL thread_local
#define THREAD_LOCAL
#define CPP_LAMBDA(x) ([](VM* vm, Args& args) { return x; })
#define CPP_NOT_IMPLEMENTED() ([](VM* vm, Args& args) { vm->NotImplementedError(); return vm->None; })

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()!");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

const float kLocalsLoadFactor = 0.67f;
const float kInstAttrLoadFactor = 0.67f;
const float kTypeAttrLoadFactor = 0.5f;
} // namespace pkpy