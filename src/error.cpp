#include "pocketpy/error.h"

namespace pkpy{

    SourceData::SourceData(std::string_view source, const Str& filename, CompileMode mode): filename(filename), mode(mode) {
        int index = 0;
        // Skip utf8 BOM if there is any.
        if (strncmp(source.data(), "\xEF\xBB\xBF", 3) == 0) index += 3;
        // Drop all '\r'
        SStream ss(source.size() + 1);
        while(index < source.size()){
            if(source[index] != '\r') ss << source[index];
            index++;
        }
        this->source = ss.str();
        if(this->source.size>5 && this->source.sv().substr(0, 5)=="pkpy:"){
            this->is_precompiled = true;
        }else{
            this->is_precompiled = false;
        }
        line_starts.push_back(this->source.c_str());
    }

    SourceData::SourceData(const Str& filename, CompileMode mode): filename(filename), mode(mode) {
        line_starts.push_back(this->source.c_str());
    }

    std::pair<const char*,const char*> SourceData::_get_line(int lineno) const {
        if(is_precompiled || lineno == -1) return {nullptr, nullptr};
        lineno -= 1;
        if(lineno < 0) lineno = 0;
        const char* _start = line_starts[lineno];
        const char* i = _start;
        // max 300 chars
        while(*i != '\n' && *i != '\0' && i-_start < 300) i++;
        return {_start, i};
    }

    std::string_view SourceData::get_line(int lineno) const{
        auto [_0, _1] = _get_line(lineno);
        if(_0 && _1) return std::string_view(_0, _1-_0);
        return "<?>";
    }

    Str SourceData::snapshot(int lineno, const char* cursor, std::string_view name) const{
        SStream ss;
        ss << "  " << "File \"" << filename << "\", line " << lineno;
        if(!name.empty()) ss << ", in " << name;
        if(!is_precompiled){
            ss << '\n';
            std::pair<const char*,const char*> pair = _get_line(lineno);
            Str line = "<?>";
            int removed_spaces = 0;
            if(pair.first && pair.second){
                line = Str(pair.first, pair.second-pair.first).lstrip();
                removed_spaces = pair.second - pair.first - line.length();
                if(line.empty()) line = "<?>";
            }
            ss << "    " << line;
            if(cursor && line != "<?>" && cursor >= pair.first && cursor <= pair.second){
                auto column = cursor - pair.first - removed_spaces;
                if(column >= 0) ss << "\n    " << std::string(column, ' ') << "^";
            }
        }
        return ss.str();
    }

    Str Exception::summary() const {
        stack<ExceptionLine> st(stacktrace);
        SStream ss;
        if(is_re) ss << "Traceback (most recent call last):\n";
        while(!st.empty()) {
            ss << st.top().snapshot() << '\n';
            st.pop();
        }
        // TODO: allow users to override the behavior
        if (!msg.empty()) ss << type.sv() << ": " << msg;
        else ss << type.sv();
        return ss.str();
    }

}   // namespace pkpy