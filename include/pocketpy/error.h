#pragma once

#include "namedict.h"
#include "str.h"
#include "tuplelist.h"

namespace pkpy{

struct NeedMoreLines {
    NeedMoreLines(bool is_compiling_class) : is_compiling_class(is_compiling_class) {}
    bool is_compiling_class;
};

struct HandledException {};
struct UnhandledException {};
struct ToBeRaisedException {};

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
    pod_vector<const char*> line_starts;

    bool is_precompiled;
    std::vector<Str> _precompiled_tokens;
    
    SourceData(std::string_view source, const Str& filename, CompileMode mode);
    SourceData(const Str& filename, CompileMode mode);
    std::pair<const char*,const char*> _get_line(int lineno) const;
    std::string_view get_line(int lineno) const;
    Str snapshot(int lineno, const char* cursor, std::string_view name) const;
};

struct ExceptionLine{
    std::shared_ptr<SourceData> src;
    int lineno;
    const char* cursor;
    std::string name;

    Str snapshot() const { return src->snapshot(lineno, cursor, name); }

    ExceptionLine(std::shared_ptr<SourceData> src, int lineno, const char* cursor, std::string_view name):
        src(src), lineno(lineno), cursor(cursor), name(name) {}
};

struct Exception {
    StrName type;
    Str msg;
    bool is_re;

    int _ip_on_error;
    void* _code_on_error;

    PyObject* _self;    // weak reference
    
    stack<ExceptionLine> stacktrace;
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

}   // namespace pkpy