#pragma once

#include "pocketpy/common/utils.h"
#include "pocketpy/common/str.hpp"
#include "pocketpy/objects/sourcedata.h"

namespace pkpy {

using CompileMode = pkpy_CompileMode;

enum {
    EXEC_MODE = PK_EXEC_MODE,
    EVAL_MODE = PK_EVAL_MODE,
    REPL_MODE = PK_REPL_MODE,
    JSON_MODE = PK_JSON_MODE,
    CELL_MODE = PK_CELL_MODE,
};

struct SourceData : public pkpy_SourceData {
    SourceData(std::string_view source, const Str& filename, pkpy_CompileMode mode) {
        pkpy_SourceData__ctor(this, source.data(), source.size(), &filename, mode);
    }

    std::string_view get_line(int lineno) const {
        const char *st, *ed;
        if (pkpy_SourceData__get_line(this, lineno, &st, &ed)) {
            return std::string_view(st, ed - st);
        }
        return "<?>";
    }

    Str snapshot(int lineno, const char* cursor, std::string_view name) const {
        return pkpy_SourceData__snapshot(this, lineno, cursor, name.empty() ? nullptr : name.data());
    }
};

}  // namespace pkpy
