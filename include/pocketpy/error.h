#pragma once

#include "namedict.h"
#include "str.h"
#include "tuplelist.h"

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

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    REPL_MODE,
    JSON_MODE,
    CELL_MODE
};

struct SourceData {
    PK_ALWAYS_PASS_BY_POINTER(SourceData)

    Str filename;
    CompileMode mode;

    Str source;
    vector<const char*> line_starts;

    bool is_precompiled;
    vector<Str> _precompiled_tokens;
    
    SourceData(std::string_view source, const Str& filename, CompileMode mode);
    SourceData(const Str& filename, CompileMode mode);
    std::pair<const char*,const char*> _get_line(int lineno) const;
    std::string_view get_line(int lineno) const;
    Str snapshot(int lineno, const char* cursor, std::string_view name) const;
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
        PK_ASSERT(_self != nullptr);
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
    Exception* ptr;
    TopLevelException(Exception* ptr): ptr(ptr) {}

    PyObject* self() const { return ptr->self(); }
    Str summary() const { return ptr->summary(); }

    const char* what() const noexcept override {
        static Str cached_summary(summary());
        return cached_summary.c_str();
    }
};

}   // namespace pkpy