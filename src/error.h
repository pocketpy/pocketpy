#pragma once

#include "safestl.h"

class NeedMoreLines {
public:
    NeedMoreLines(bool isClassDef) : isClassDef(isClassDef) {}
    bool isClassDef;
};

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    SINGLE_MODE,     // for REPL
    JSON_MODE,
};

struct SourceMetadata {
    const char* source;
    _Str filename;
    std::vector<const char*> lineStarts;
    CompileMode mode;

    std::pair<const char*,const char*> getLine(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = lineStarts.at(lineno);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return {_start, i};
    }

    SourceMetadata(const char* source, _Str filename, CompileMode mode) {
        source = strdup(source);
        // Skip utf8 BOM if there is any.
        if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
        this->filename = filename;
        this->source = source;
        lineStarts.push_back(source);
        this->mode = mode;
    }

    _Str snapshot(int lineno, const char* cursor=nullptr){
        _StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        std::pair<const char*,const char*> pair = getLine(lineno);
        _Str line = "<?>";
        int removedSpaces = 0;
        if(pair.first && pair.second){
            line = _Str(pair.first, pair.second-pair.first).__lstrip();
            removedSpaces = pair.second - pair.first - line.size();
            if(line.empty()) line = "<?>";
        }
        ss << "    " << line << '\n';
        if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
            int column = cursor - pair.first - removedSpaces;
            if(column >= 0){
                ss << "    " << std::string(column, ' ') << "^\n";
            }
        }
        return ss.str();
    }

    ~SourceMetadata(){
        free((void*)source);
    }
};

typedef pkpy::shared_ptr<SourceMetadata> _Source;

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
    RuntimeError(_Str type, _Str msg, const std::stack<_Str>& snapshots)
        : _Error(type, msg, __concat(snapshots)) {}
};