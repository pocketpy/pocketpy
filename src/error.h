#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include "str.h"

class NeedMoreLines {
public:
    NeedMoreLines(bool isClassDef) : isClassDef(isClassDef) {}
    bool isClassDef;
};

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    SINGLE_MODE
};

struct SourceMetadata {
    const char* source;
    _Str filename;
    std::vector<const char*> lineStarts;
    CompileMode mode;

    _Str getLine(int lineno) const {
        if(lineno == -1) return "<?>";
        const char* _start = lineStarts.at(lineno-1);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return _Str(_start, i-_start);
    }

    SourceMetadata(const char* source, _Str filename, CompileMode mode) {
        // Skip utf8 BOM if there is any.
        if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
        this->filename = filename;
        this->source = source;
        lineStarts.push_back(source);
        this->mode = mode;
    }

    _Str snapshot(int lineno){
        _StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        _Str line = getLine(lineno).__lstrip();
        if(line.empty()) line = "<?>";
        ss << "    " << line << '\n';
        return ss.str();
    }
};

typedef std::shared_ptr<SourceMetadata> _Source;

class _Error : public std::exception {
private:
    _Str _what;
public:
    _Error(_Str type, _Str msg, _Str desc){
        _what = desc + type + ": " + msg;
    }

    const char* what() const noexcept override {
        return _what.c_str();
    }
};

class CompileError : public _Error {
public:
    CompileError(_Str type, _Str msg, _Str snapshot)
        : _Error(type, msg, snapshot) {}
};

class RuntimeError : public _Error {
private:
    static _Str __concat(std::stack<_Str> snapshots){
        _StrStream ss;
        ss << "Traceback (most recent call last):" << '\n';
        while(!snapshots.empty()){
            ss << snapshots.top();
            snapshots.pop();
        }
        return ss.str();
    }
public:
    RuntimeError(_Str type, _Str msg, std::stack<_Str> snapshots)
        : _Error(type, msg, __concat(snapshots)) {}
};