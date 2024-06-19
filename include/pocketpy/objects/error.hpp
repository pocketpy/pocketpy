#pragma once

#include "pocketpy/common/str.hpp"
#include "pocketpy/common/traits.hpp"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/error.h"

namespace pkpy {

struct NeedMoreLines {
    NeedMoreLines(bool is_compiling_class) : is_compiling_class(is_compiling_class) {}

    bool is_compiling_class;
};

enum class InternalExceptionType : int { Null, Handled, Unhandled, ToBeRaised };

struct InternalException final {
    InternalExceptionType type;
    int arg;

    InternalException() : type(InternalExceptionType::Null), arg(-1) {}

    InternalException(InternalExceptionType type, int arg = -1) : type(type), arg(arg) {}
};

struct Exception: pkpy_Exception{
    PK_ALWAYS_PASS_BY_POINTER(Exception)

    Exception(uint16_t type){
        pkpy_Exception__ctor(this, type);
    }

    ~Exception(){
        pkpy_Exception__dtor(this);
    }

    void stpush(pkpy_SourceData_ src, int lineno, const char* cursor, const char* name){
        pkpy_Exception__stpush(this, src, lineno, cursor, name);
    }

    Str summary(){
        return pkpy_Exception__summary(this);
    }
};

struct TopLevelException : std::exception {
    VM* vm;
    Exception* ptr;

    TopLevelException(VM* vm, Exception* ptr) : vm(vm), ptr(ptr) {}

    Str summary() const { return ptr->summary(); }

    const char* what() const noexcept override {
        static Str cached_summary;
        cached_summary = summary();
        return cached_summary.c_str();
    }
};



}  // namespace pkpy
