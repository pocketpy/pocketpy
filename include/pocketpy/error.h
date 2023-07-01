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
    std::string source;
    Str filename;
    std::vector<const char*> line_starts;
    CompileMode mode;

    SourceData(const SourceData&) = delete;
    SourceData& operator=(const SourceData&) = delete;

    SourceData(const Str& source, const Str& filename, CompileMode mode);
    std::pair<const char*,const char*> get_line(int lineno) const;
    Str snapshot(int lineno, const char* cursor=nullptr);
};

struct Exception {
    StrName type;
    Str msg;
    bool is_re;
    stack<Str> stacktrace;

    Exception(StrName type, Str msg): type(type), msg(msg), is_re(true) {}
    bool match_type(StrName t) const { return this->type == t;}
    void st_push(Str snapshot);
    Str summary() const;
};

}   // namespace pkpy