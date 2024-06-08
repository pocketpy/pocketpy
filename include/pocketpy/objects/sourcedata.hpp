#pragma once

#include "pocketpy/common/utils.hpp"
#include "pocketpy/common/str.hpp"

namespace pkpy {

enum CompileMode { EXEC_MODE, EVAL_MODE, REPL_MODE, JSON_MODE, CELL_MODE };

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
    pair<const char*, const char*> _get_line(int lineno) const;
    std::string_view get_line(int lineno) const;
    Str snapshot(int lineno, const char* cursor, std::string_view name) const;
};

}  // namespace pkpy
