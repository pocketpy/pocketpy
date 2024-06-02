#pragma once

#include "pocketpy/common/str.hpp"
#include "pocketpy/objects/sourcedata.hpp"

namespace pkpy{

struct NeedMoreLines {
    NeedMoreLines(bool is_compiling_class) : is_compiling_class(is_compiling_class) {}
    bool is_compiling_class;
};

enum class InternalExceptionType: int{
    Null, Handled, Unhandled, ToBeRaised
};

struct InternalException final{
    InternalExceptionType type;
    int arg;
    InternalException(): type(InternalExceptionType::Null), arg(-1) {}
    InternalException(InternalExceptionType type, int arg=-1): type(type), arg(arg) {}
};

struct Exception {
    StrName type;
    Str msg;
    bool is_re;

    int _ip_on_error;
    void* _code_on_error;

    PyObject* _self;    // weak reference

    struct Frame{
        std::shared_ptr<SourceData> src;
        int lineno;
        const char* cursor;
        std::string name;

        Str snapshot() const { return src->snapshot(lineno, cursor, name); }

        Frame(std::shared_ptr<SourceData> src, int lineno, const char* cursor, std::string_view name):
            src(src), lineno(lineno), cursor(cursor), name(name) {}
    };

    stack<Frame> stacktrace;
    Exception(StrName type): type(type), is_re(true), _ip_on_error(-1), _code_on_error(nullptr), _self(nullptr) {}

    PyObject* self() const{
        assert(_self != nullptr);
        return _self;
    }

    template<typename... Args>
    void st_push(Args&&... args){
        if(stacktrace.size() >= 7) return;
        stacktrace.emplace(std::forward<Args>(args)...);
    }

    Str summary() const;
};

struct TopLevelException: std::exception{
    VM* vm;
    Exception* ptr;
    TopLevelException(VM* vm, Exception* ptr): vm(vm), ptr(ptr) {}

    Str summary() const { return ptr->summary(); }

    const char* what() const noexcept override {
        static Str cached_summary;
        cached_summary = summary();
        return cached_summary.c_str();
    }
};

}   // namespace pkpy