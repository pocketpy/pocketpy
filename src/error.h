#pragma once

#include "safestl.h"

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
};

struct SourceData {
    const char* source;
    Str filename;
    std::vector<const char*> line_starts;
    CompileMode mode;

    std::pair<const char*,const char*> get_line(int lineno) const {
        if(lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = line_starts.at(lineno);
        const char* i = _start;
        while(*i != '\n' && *i != '\0') i++;
        return {_start, i};
    }

    SourceData(const char* source, Str filename, CompileMode mode) {
        source = strdup(source);
        // Skip utf8 BOM if there is any.
        if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
        this->filename = filename;
        this->source = source;
        line_starts.push_back(source);
        this->mode = mode;
    }

    Str snapshot(int lineno, const char* cursor=nullptr){
        StrStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno << '\n';
        std::pair<const char*,const char*> pair = get_line(lineno);
        Str line = "<?>";
        int removed_spaces = 0;
        if(pair.first && pair.second){
            line = Str(pair.first, pair.second-pair.first).lstrip();
            removed_spaces = pair.second - pair.first - line.size();
            if(line.empty()) line = "<?>";
        }
        ss << "    " << line;
        if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
            auto column = cursor - pair.first - removed_spaces;
            if(column >= 0) ss << "\n    " << std::string(column, ' ') << "^";
        }
        return ss.str();
    }

    ~SourceData() { free((void*)source); }
};

namespace pkpy{
class Exception {
    Str type;
    Str msg;
    std::stack<Str> stacktrace;
public:
    Exception(Str type, Str msg): type(type), msg(msg) {}
    bool match_type(const Str& type) const { return this->type == type;}
    bool is_re = true;

    void st_push(Str snapshot){
        if(stacktrace.size() >= 8) return;
        stacktrace.push(snapshot);
    }

    Str summary() const {
        std::stack<Str> st(stacktrace);
        StrStream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) { ss << st.top() << '\n'; st.pop(); }
        if (msg.compare("") != 0) ss << type << ": " << msg;
        else ss << type;
        return ss.str();
    }
};
}