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

#include "hash_table8.hpp"

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()! This should be a bug, please report it");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

#define PK_VERSION "0.8.4"

typedef int64_t i64;
typedef double f64;
#define DUMMY_VAL (char)1
#define DUMMY_VAL_TP char

template<typename T>
void* tid() {
	static volatile int8_t _x;
	return (void*)(&_x);
}

// This does not ensure to be unique when the pointer of obj->type is deleted & reused.
// But it is good enough for now.
template<typename T>
void* obj_tid(void* alt){
    if constexpr(std::is_same_v<T, DUMMY_VAL_TP>) return alt;
    return tid<T>();
}