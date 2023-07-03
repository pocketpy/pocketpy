#include "pocketpy/lexer.h"

namespace pkpy{

    bool Lexer::match_n_chars(int n, char c0){
        const char* c = curr_char;
        for(int i=0; i<n; i++){
            if(*c == '\0') return false;
            if(*c != c0) return false;
            c++;
        }
        for(int i=0; i<n; i++) eatchar_include_newline();
        return true;
    }

    bool Lexer::match_string(const char* s){
        int s_len = strlen(s);
        bool ok = strncmp(curr_char, s, s_len) == 0;
        if(ok) for(int i=0; i<s_len; i++) eatchar_include_newline();
        return ok;
    }

    int Lexer::eat_spaces(){
        int count = 0;
        while (true) {
            switch (peekchar()) {
                case ' ' : count+=1; break;
                case '\t': count+=4; break;
                default: return count;
            }
            eatchar();
        }
    }

    bool Lexer::eat_indentation(){
        if(brackets_level > 0) return true;
        int spaces = eat_spaces();
        if(peekchar() == '#') skip_line_comment();
        if(peekchar() == '\0' || peekchar() == '\n') return true;
        // https://docs.python.org/3/reference/lexical_analysis.html#indentation
        if(spaces > indents.top()){
            indents.push(spaces);
            nexts.push_back(Token{TK("@indent"), token_start, 0, current_line, brackets_level});
        } else if(spaces < indents.top()){
            while(spaces < indents.top()){
                indents.pop();
                nexts.push_back(Token{TK("@dedent"), token_start, 0, current_line, brackets_level});
            }
            if(spaces != indents.top()){
                return false;
            }
        }
        return true;
    }

    char Lexer::eatchar() {
        char c = peekchar();
        if(c == '\n') throw std::runtime_error("eatchar() cannot consume a newline");
        curr_char++;
        return c;
    }

    char Lexer::eatchar_include_newline() {
        char c = peekchar();
        curr_char++;
        if (c == '\n'){
            current_line++;
            src->line_starts.push_back(curr_char);
        }
        return c;
    }

    int Lexer::eat_name() {
        curr_char--;
        while(true){
            unsigned char c = peekchar();
            int u8bytes = utf8len(c, true);
            if(u8bytes == 0) return 1;
            if(u8bytes == 1){
                if(isalpha(c) || c=='_' || isdigit(c)) {
                    curr_char++;
                    continue;
                }else{
                    break;
                }
            }
            // handle multibyte char
            std::string u8str(curr_char, u8bytes);
            if(u8str.size() != u8bytes) return 2;
            uint32_t value = 0;
            for(int k=0; k < u8bytes; k++){
                uint8_t b = u8str[k];
                if(k==0){
                    if(u8bytes == 2) value = (b & 0b00011111) << 6;
                    else if(u8bytes == 3) value = (b & 0b00001111) << 12;
                    else if(u8bytes == 4) value = (b & 0b00000111) << 18;
                }else{
                    value |= (b & 0b00111111) << (6*(u8bytes-k-1));
                }
            }
            if(is_unicode_Lo_char(value)) curr_char += u8bytes;
            else break;
        }

        int length = (int)(curr_char - token_start);
        if(length == 0) return 3;
        std::string_view name(token_start, length);

        if(src->mode == JSON_MODE){
            if(name == "true"){
                add_token(TK("True"));
            } else if(name == "false"){
                add_token(TK("False"));
            } else if(name == "null"){
                add_token(TK("None"));
            } else {
                return 4;
            }
            return 0;
        }

        if(kTokenKwMap.count(name)){
            add_token(kTokenKwMap.at(name));
        } else {
            add_token(TK("@id"));
        }
        return 0;
    }

    void Lexer::skip_line_comment() {
        char c;
        while ((c = peekchar()) != '\0') {
            if (c == '\n') return;
            eatchar();
        }
    }
    
    bool Lexer::matchchar(char c) {
        if (peekchar() != c) return false;
        eatchar_include_newline();
        return true;
    }

    void Lexer::add_token(TokenIndex type, TokenValue value) {
        switch(type){
            case TK("{"): case TK("["): case TK("("): brackets_level++; break;
            case TK(")"): case TK("]"): case TK("}"): brackets_level--; break;
        }
        auto token = Token{
            type,
            token_start,
            (int)(curr_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
            brackets_level,
            value
        };
        // handle "not in", "is not", "yield from"
        if(!nexts.empty()){
            auto& back = nexts.back();
            if(back.type == TK("not") && type == TK("in")){
                back.type = TK("not in");
                return;
            }
            if(back.type == TK("is") && type == TK("not")){
                back.type = TK("is not");
                return;
            }
            if(back.type == TK("yield") && type == TK("from")){
                back.type = TK("yield from");
                return;
            }
            nexts.push_back(token);
        }
    }

    void Lexer::add_token_2(char c, TokenIndex one, TokenIndex two) {
        if (matchchar(c)) add_token(two);
        else add_token(one);
    }

    Str Lexer::eat_string_until(char quote, bool raw) {
        bool quote3 = match_n_chars(2, quote);
        std::vector<char> buff;
        while (true) {
            char c = eatchar_include_newline();
            if (c == quote){
                if(quote3 && !match_n_chars(2, quote)){
                    buff.push_back(c);
                    continue;
                }
                break;
            }
            if (c == '\0'){
                if(quote3 && src->mode == REPL_MODE){
                    throw NeedMoreLines(false);
                }
                SyntaxError("EOL while scanning string literal");
            }
            if (c == '\n'){
                if(!quote3) SyntaxError("EOL while scanning string literal");
                else{
                    buff.push_back(c);
                    continue;
                }
            }
            if (!raw && c == '\\') {
                switch (eatchar_include_newline()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    case 'x': {
                        char hex[3] = {eatchar(), eatchar(), '\0'};
                        size_t parsed;
                        char code;
                        try{
                            code = (char)Number::stoi(hex, &parsed, 16);
                        }catch(...){
                            SyntaxError("invalid hex char");
                        }
                        if (parsed != 2) SyntaxError("invalid hex char");
                        buff.push_back(code);
                    } break;
                    default: SyntaxError("invalid escape char");
                }
            } else {
                buff.push_back(c);
            }
        }
        return Str(buff.data(), buff.size());
    }

    void Lexer::eat_string(char quote, StringType type) {
        Str s = eat_string_until(quote, type == RAW_STRING);
        if(type == F_STRING){
            add_token(TK("@fstr"), s);
        }else{
            add_token(TK("@str"), s);
        }
    }

    void Lexer::eat_number() {
        static const std::regex pattern("^(0x)?[0-9a-fA-F]+(\\.[0-9]+)?(L)?");
        std::smatch m;

        const char* i = token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(token_start, i);

        bool ok = std::regex_search(s, m, pattern);
        PK_ASSERT(ok);
        // here is m.length()-1, since the first char was eaten by lex_token()
        for(int j=0; j<m.length()-1; j++) eatchar();

        if(m[3].matched){
            add_token(TK("@long"));
            return;
        }

        if(m[1].matched && m[2].matched){
            SyntaxError("hex literal should not contain a dot");
        }

        try{
            int base = 10;
            size_t size;
            if (m[1].matched) base = 16;
            if (m[2].matched) {
                PK_ASSERT(base == 10);
                add_token(TK("@num"), Number::stof(m[0], &size));
            } else {
                add_token(TK("@num"), Number::stoi(m[0], &size, base));
            }
            PK_ASSERT((int)size == (int)m.length());
        }catch(...){
            SyntaxError("invalid number literal");
        }
    }

    bool Lexer::lex_one_token() {
        while (peekchar() != '\0') {
            token_start = curr_char;
            char c = eatchar_include_newline();
            switch (c) {
                case '\'': case '"': eat_string(c, NORMAL_STRING); return true;
                case '#': skip_line_comment(); break;
                case '{': add_token(TK("{")); return true;
                case '}': add_token(TK("}")); return true;
                case ',': add_token(TK(",")); return true;
                case ':': add_token(TK(":")); return true;
                case ';': add_token(TK(";")); return true;
                case '(': add_token(TK("(")); return true;
                case ')': add_token(TK(")")); return true;
                case '[': add_token(TK("[")); return true;
                case ']': add_token(TK("]")); return true;
                case '@': add_token(TK("@")); return true;
                case '$': {
                    for(int i=TK("$goto"); i<=TK("$label"); i++){
                        // +1 to skip the '$'
                        if(match_string(TK_STR(i) + 1)){
                            add_token((TokenIndex)i);
                            return true;
                        }
                    }
                    SyntaxError("invalid special token");
                } return false;
                case '%': add_token_2('=', TK("%"), TK("%=")); return true;
                case '&': add_token_2('=', TK("&"), TK("&=")); return true;
                case '|': add_token_2('=', TK("|"), TK("|=")); return true;
                case '^': add_token_2('=', TK("^"), TK("^=")); return true;
                case '?': add_token(TK("?")); return true;
                case '.': {
                    if(matchchar('.')) {
                        if(matchchar('.')) {
                            add_token(TK("..."));
                        } else {
                            SyntaxError("invalid token '..'");
                        }
                    } else {
                        add_token(TK("."));
                    }
                    return true;
                }
                case '=': add_token_2('=', TK("="), TK("==")); return true;
                case '+':
                    if(matchchar('+')){
                        add_token(TK("++"));
                    }else{
                        add_token_2('=', TK("+"), TK("+="));
                    }
                    return true;
                case '>': {
                    if(matchchar('=')) add_token(TK(">="));
                    else if(matchchar('>')) add_token_2('=', TK(">>"), TK(">>="));
                    else add_token(TK(">"));
                    return true;
                }
                case '<': {
                    if(matchchar('=')) add_token(TK("<="));
                    else if(matchchar('<')) add_token_2('=', TK("<<"), TK("<<="));
                    else add_token(TK("<"));
                    return true;
                }
                case '-': {
                    if(matchchar('-')){
                        add_token(TK("--"));
                    }else{
                        if(matchchar('=')) add_token(TK("-="));
                        else if(matchchar('>')) add_token(TK("->"));
                        else add_token(TK("-"));
                    }
                    return true;
                }
                case '!':
                    if(matchchar('=')) add_token(TK("!="));
                    else SyntaxError("expected '=' after '!'");
                    break;
                case '*':
                    if (matchchar('*')) {
                        add_token(TK("**"));  // '**'
                    } else {
                        add_token_2('=', TK("*"), TK("*="));
                    }
                    return true;
                case '/':
                    if(matchchar('/')) {
                        add_token_2('=', TK("//"), TK("//="));
                    } else {
                        add_token_2('=', TK("/"), TK("/="));
                    }
                    return true;
                case ' ': case '\t': eat_spaces(); break;
                case '\n': {
                    add_token(TK("@eol"));
                    if(!eat_indentation()) IndentationError("unindent does not match any outer indentation level");
                    return true;
                }
                default: {
                    if(c == 'f'){
                        if(matchchar('\'')) {eat_string('\'', F_STRING); return true;}
                        if(matchchar('"')) {eat_string('"', F_STRING); return true;}
                    }else if(c == 'r'){
                        if(matchchar('\'')) {eat_string('\'', RAW_STRING); return true;}
                        if(matchchar('"')) {eat_string('"', RAW_STRING); return true;}
                    }
                    if (c >= '0' && c <= '9') {
                        eat_number();
                        return true;
                    }
                    switch (eat_name())
                    {
                        case 0: break;
                        case 1: SyntaxError("invalid char: " + std::string(1, c)); break;
                        case 2: SyntaxError("invalid utf8 sequence: " + std::string(1, c)); break;
                        case 3: SyntaxError("@id contains invalid char"); break;
                        case 4: SyntaxError("invalid JSON token"); break;
                        default: FATAL_ERROR();
                    }
                    return true;
                }
            }
        }

        token_start = curr_char;
        while(indents.size() > 1){
            indents.pop();
            add_token(TK("@dedent"));
            return true;
        }
        add_token(TK("@eof"));
        return false;
    }

    void Lexer::throw_err(Str type, Str msg){
        int lineno = current_line;
        const char* cursor = curr_char;
        if(peekchar() == '\n'){
            lineno--;
            cursor--;
        }
        throw_err(type, msg, lineno, cursor);
    }

    void Lexer::throw_err(Str type, Str msg, int lineno, const char* cursor){
        auto e = Exception(type, msg);
        e.st_push(src->snapshot(lineno, cursor));
        throw e;
    }

    Lexer::Lexer(shared_ptr<SourceData> src) {
        this->src = src;
        this->token_start = src->source.c_str();
        this->curr_char = src->source.c_str();
        this->nexts.push_back(Token{TK("@sof"), token_start, 0, current_line, brackets_level});
        this->indents.push(0);
    }

    std::vector<Token> Lexer::run() {
        if(used) FATAL_ERROR();
        used = true;
        while (lex_one_token());
        return std::move(nexts);
    }

}   // namespace pkpy