#pragma once

#include "safestl.h"

struct NeedMoreLines {
    NeedMoreLines(bool isClassDef) : isClassDef(isClassDef) {}
    bool isClassDef;
};

struct HandledException {};
struct UnhandledException {};
struct ToBeRaisedException {};

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    SINGLE_MODE,     // for REPL
    JSON_MODE,
};

struct SourceData {
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

    SourceData(const char* source, _Str filename, CompileMode mode) {
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
            line = _Str(pair.first, pair.second-pair.first).lstrip();
            removedSpaces = pair.second - pair.first - line.size();
            if(line.empty()) line = "<?>";
        }
        ss << "    " << line;
        if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
            auto column = cursor - pair.first - removedSpaces;
            if(column >= 0) ss << "\n    " << std::string(column, ' ') << "^";
        }
        return ss.str();
    }

    ~SourceData(){
        free((void*)source);
    }
};

class _Exception {
    _Str type;
    _Str msg;
    std::stack<_Str> stacktrace;
public:
    _Exception(_Str type, _Str msg): type(type), msg(msg) {}
    bool match_type(const _Str& type) const { return this->type == type;}
    bool is_re = true;

    void st_push(_Str snapshot){
        if(stacktrace.size() >= 8) return;
        stacktrace.push(snapshot);
    }

    _Str summary() const {
        std::stack<_Str> st(stacktrace);
        _StrStream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) { ss << st.top() << '\n'; st.pop(); }
        ss << type << ": " << msg;
        return ss.str();
    }
};