#pragma once

#include "pocketpy/common/utils.h"
#include "pocketpy/common/str.hpp"
#include "pocketpy/objects/sourcedata.h"

namespace pkpy {

struct SourceData {
    pkpy_SourceData *self;

    SourceData(): self(nullptr) {
    }

    SourceData(std::string_view source, const Str& filename, pkpy_CompileMode mode) {
        self = static_cast<pkpy_SourceData*>(std::malloc(sizeof(pkpy_SourceData)));
        pkpy_SourceData__ctor(self, source.data(), source.size(), &filename, mode);
    }

    SourceData(const SourceData& other) {
        self = other.self;
        pkpy_Rcptr__ref(self);
    }

    SourceData& operator=(const SourceData& other) {
        if (this != &other) {
            pkpy_Rcptr__unref(self);
            self = other.self;
            pkpy_Rcptr__ref(self);
        }
        return *this;
    }

    SourceData(SourceData &&other) {
        self = other.self;
        other.self = nullptr;
    }

    SourceData& operator=(SourceData &&other) {
        if (this != &other) {
            pkpy_Rcptr__unref(self);
            self = other.self;
            other.self = nullptr;
        }
        return *this;
    }

    pkpy_SourceData* get() const {
        return self;
    }

    pkpy_SourceData* operator->() const {
        return self;
    }

    std::string_view get_line(int lineno) const {
        const char *st, *ed;
        if (pkpy_SourceData__get_line(self, lineno, &st, &ed)) {
            return std::string_view(st, ed - st);
        }
        return "<?>";
    }

    Str& filename() const {
        return static_cast<Str&>(self->filename);
    }

    Str& source() const {
        return static_cast<Str&>(self->source);
    }

    Str snapshot(int lineno, const char* cursor, std::string_view name) const {
        return pkpy_SourceData__snapshot(self, lineno, cursor, name.data());
    }

    ~SourceData() {
        pkpy_Rcptr__unref(self);
    }
};

}  // namespace pkpy
