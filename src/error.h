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

struct LineSnapshot {
    _Str filename;
    int lineno;
    _Str source;

    _Str str() const {
        _StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        ss << "    " << source << '\n';
        return ss.str();
    }
};

class CompileError : public _Error {
public:
    CompileError(_Str type, _Str msg, const LineSnapshot& snapshot)
        : _Error(type, msg, snapshot.str()) {}
};

class RuntimeError : public _Error {
private:
    static _Str __concat(std::stack<LineSnapshot> snapshots){
        _StrStream ss;
        ss << "Traceback (most recent call last):" << '\n';
        while(!snapshots.empty()){
            ss << snapshots.top().str();
            snapshots.pop();
        }
        return ss.str();
    }
public:
    RuntimeError(_Str type, _Str msg, std::stack<LineSnapshot> snapshots)
        : _Error(type, msg, __concat(snapshots)) {}
};

class UnexpectedError : public _Error {
public:
    UnexpectedError(_Str msg)
        : _Error("UnexpectedError", msg, "") {}
};