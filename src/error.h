#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <stdarg.h>

#include "parser.h"

class NeedMoreLines : public std::exception {};

class SyntaxError : public std::exception {
private:
    _Str _what;

public:
    char message[100];
    _Str path;
    int lineno;

    SyntaxError(const _Str& path, Token tk, const char* msg, ...) {
        va_list args;
        va_start(args, msg);
        vsnprintf(message, 100, msg, args);
        va_end(args);

        this->path = path;
        lineno = tk.line;

        _StrStream ss;
        ss << "  File '" << path << "', line " << std::to_string(lineno) << std::endl;
        ss << _Str("SyntaxError: ") << message;
        _what = ss.str();
    }

    const char* what() const noexcept override {
        return _what.str().c_str();
    }
};