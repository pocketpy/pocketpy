#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include "str.h"

class NeedMoreLines : public std::exception {
public:
    NeedMoreLines(bool isClassDef) : isClassDef(isClassDef) {}
    bool isClassDef;
};

struct SourceMetadata {
    _Str filename;
    const char* source;
    std::vector<const char*> lineStarts;

    _Str getLine(int lineno) const {
        if(lineno == -1) return "<?>";
        const char* _start = lineStarts.at(lineno-1);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return _Str(_start, i-_start);
    }

    SourceMetadata(_Str filename, const char* source) {
        // Skip utf8 BOM if there is any.
        if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
        this->filename = filename;
        this->source = source;
        lineStarts.push_back(source);
    }

    _Str snapshot(int lineno){
        _StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        ss << "    " << getLine(lineno) << '\n';
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
        return _what;
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

class UnexpectedError : public _Error {
public:
    UnexpectedError(_Str msg)
        : _Error("UnexpectedError", msg, "") {}
};