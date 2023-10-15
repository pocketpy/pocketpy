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
    std::string source;  // assume '\0' terminated
    Str filename;
    std::vector<const char*> line_starts;
    CompileMode mode;

    SourceData(const SourceData&) = delete;
    SourceData& operator=(const SourceData&) = delete;

    SourceData(const Str& source, const Str& filename, CompileMode mode);
    std::pair<const char*,const char*> get_line(int lineno) const;
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
    
    stack<ExceptionLine> stacktrace;

    Exception(StrName type, Str msg): 
        type(type), msg(msg), is_re(true), _ip_on_error(-1), _code_on_error(nullptr) {}
    bool match_type(StrName t) const { return this->type == t;}

    template<typename... Args>
    void st_push(Args&&... args){
        if(stacktrace.size() >= 8) return;
        stacktrace.emplace(std::forward<Args>(args)...);
    }

    Str summary() const;
};

}   // namespace pkpy