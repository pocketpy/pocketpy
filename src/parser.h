#pragma once

#include "error.h"
#include "obj.h"

namespace pkpy{

typedef uint8_t TokenIndex;

constexpr const char* kTokens[] = {
    "@error", "@eof", "@eol", "@sof",
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}", "%", "::",
    "+", "-", "*", "/", "//", "**", "=", ">", "<", "...", "->",
    "<<", ">>", "&", "|", "^", "?", "@",
    "==", "!=", ">=", "<=",
    "+=", "-=", "*=", "/=", "//=", "%=", "&=", "|=", "^=", ">>=", "<<=",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del", "from", "with", "yield",
    "None", "in", "is", "and", "or", "not", "True", "False", "global", "try", "except", "finally",
    "goto", "label",      // extended keywords, not available in cpython
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise",
    /** KW_END **/
    "is not", "not in",
    "@id", "@num", "@str", "@fstr",
    "@indent", "@dedent"
};

const TokenIndex kTokenCount = sizeof(kTokens) / sizeof(kTokens[0]);

constexpr TokenIndex TK(const char token[]) {
    for(int k=0; k<kTokenCount; k++){
        const char* i = kTokens[k];
        const char* j = token;
        while(*i && *j && *i == *j) { i++; j++;}
        if(*i == *j) return k;
    }
    UNREACHABLE();
}

#define TK_STR(t) kTokens[t]
const TokenIndex kTokenKwBegin = TK("class");
const TokenIndex kTokenKwEnd = TK("raise");

const std::map<std::string_view, TokenIndex> kTokenKwMap = [](){
    std::map<std::string_view, TokenIndex> map;
    for(int k=kTokenKwBegin; k<=kTokenKwEnd; k++) map[kTokens[k]] = k;
    return map;
}();


struct Token{
  TokenIndex type;

  const char* start;
  int length;
  int line;
  PyVar value;

  Str str() const { return Str(start, length);}

  Str info() const {
    StrStream ss;
    Str raw = str();
    if (raw == Str("\n")) raw = "\\n";
    ss << line << ": " << TK_STR(type) << " '" << raw << "'";
    return ss.str();
  }
};

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,    // =
  PREC_COMMA,         // ,
  PREC_TERNARY,       // ?:
  PREC_LOGICAL_OR,    // or
  PREC_LOGICAL_AND,   // and
  PREC_EQUALITY,      // == !=
  PREC_TEST,          // in is
  PREC_COMPARISION,   // < > <= >=
  PREC_BITWISE_OR,    // |
  PREC_BITWISE_XOR,   // ^
  PREC_BITWISE_AND,   // &
  PREC_BITWISE_SHIFT, // << >>
  PREC_TERM,          // + -
  PREC_FACTOR,        // * / % //
  PREC_UNARY,         // - not
  PREC_EXPONENT,      // **
  PREC_CALL,          // ()
  PREC_SUBSCRIPT,     // []
  PREC_ATTRIB,        // .index
  PREC_PRIMARY,
};

// The context of the parsing phase for the compiler.
struct Parser {
    shared_ptr<SourceData> src;

    const char* token_start;
    const char* curr_char;
    int current_line = 1;
    Token prev, curr;
    std::queue<Token> nexts;
    std::stack<int> indents;

    int brackets_level = 0;

    Token next_token(){
        if(nexts.empty()){
            return Token{TK("@error"), token_start, (int)(curr_char - token_start), current_line};
        }
        Token t = nexts.front();
        if(t.type == TK("@eof") && indents.size()>1){
            nexts.pop();
            indents.pop();
            return Token{TK("@dedent"), token_start, 0, current_line};
        }
        nexts.pop();
        return t;
    }

    inline char peekchar() const{ return *curr_char; }

    bool match_n_chars(int n, char c0){
        const char* c = curr_char;
        for(int i=0; i<n; i++){
            if(*c == '\0') return false;
            if(*c != c0) return false;
            c++;
        }
        for(int i=0; i<n; i++) eatchar_include_newline();
        return true;
    }

    int eat_spaces(){
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

    bool eat_indentation(){
        if(brackets_level > 0) return true;
        int spaces = eat_spaces();
        if(peekchar() == '#') skip_line_comment();
        if(peekchar() == '\0' || peekchar() == '\n' || peekchar() == '\r') return true;
        // https://docs.python.org/3/reference/lexical_analysis.html#indentation
        if(spaces > indents.top()){
            indents.push(spaces);
            nexts.push(Token{TK("@indent"), token_start, 0, current_line});
        } else if(spaces < indents.top()){
            while(spaces < indents.top()){
                indents.pop();
                nexts.push(Token{TK("@dedent"), token_start, 0, current_line});
            }
            if(spaces != indents.top()){
                return false;
            }
        }
        return true;
    }

    char eatchar() {
        char c = peekchar();
        if(c == '\n') throw std::runtime_error("eatchar() cannot consume a newline");
        curr_char++;
        return c;
    }

    char eatchar_include_newline() {
        char c = peekchar();
        curr_char++;
        if (c == '\n'){
            current_line++;
            src->line_starts.push_back(curr_char);
        }
        return c;
    }

    int eat_name() {
        curr_char--;
        while(true){
            uint8_t c = peekchar();
            int u8bytes = 0;
            if((c & 0b10000000) == 0b00000000) u8bytes = 1;
            else if((c & 0b11100000) == 0b11000000) u8bytes = 2;
            else if((c & 0b11110000) == 0b11100000) u8bytes = 3;
            else if((c & 0b11111000) == 0b11110000) u8bytes = 4;
            else return 1;
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
                set_next_token(TK("True"));
            } else if(name == "false"){
                set_next_token(TK("False"));
            } else if(name == "null"){
                set_next_token(TK("None"));
            } else {
                return 4;
            }
            return 0;
        }

        if(kTokenKwMap.count(name)){
            if(name == "not"){
                if(strncmp(curr_char, " in", 3) == 0){
                    curr_char += 3;
                    set_next_token(TK("not in"));
                    return 0;
                }
            }else if(name == "is"){
                if(strncmp(curr_char, " not", 4) == 0){
                    curr_char += 4;
                    set_next_token(TK("is not"));
                    return 0;
                }
            }
            set_next_token(kTokenKwMap.at(name));
        } else {
            set_next_token(TK("@id"));
        }
        return 0;
    }

    void skip_line_comment() {
        char c;
        while ((c = peekchar()) != '\0') {
            if (c == '\n') return;
            eatchar();
        }
    }
    
    bool matchchar(char c) {
        if (peekchar() != c) return false;
        eatchar_include_newline();
        return true;
    }

    void set_next_token(TokenIndex type, PyVar value=nullptr) {
        switch(type){
            case TK("{"): case TK("["): case TK("("): brackets_level++; break;
            case TK(")"): case TK("]"): case TK("}"): brackets_level--; break;
        }
        nexts.push( Token{
            type,
            token_start,
            (int)(curr_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
            value
        });
    }

    void set_next_token_2(char c, TokenIndex one, TokenIndex two) {
        if (matchchar(c)) set_next_token(two);
        else set_next_token(one);
    }

    Parser(shared_ptr<SourceData> src) {
        this->src = src;
        this->token_start = src->source;
        this->curr_char = src->source;
        this->nexts.push(Token{TK("@sof"), token_start, 0, current_line});
        this->indents.push(0);
    }
};

} // namespace pkpy