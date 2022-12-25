#pragma once

#ifdef _MSC_VER
#pragma warning (disable:4267)
#pragma warning (disable:4101)
#define _CRT_NONSTDC_NO_DEPRECATE
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
#include <map>

#include <thread>
#include <atomic>
#include <iostream>

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()! This should be a bug, please report it");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

#define PK_VERSION "0.5.2"

//#define PKPY_NO_TYPE_CHECK
//#define PKPY_NO_INDEX_CHECK