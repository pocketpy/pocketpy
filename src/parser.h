#pragma once

#include "obj.h"

typedef uint8_t _TokenType;

constexpr const char* __TOKENS[] = {
    "@error", "@eof", "@eol", "@sof",
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}", "%",
    "+", "-", "*", "/", "//", "**", "=", ">", "<", "...", "->",
    "<<", ">>", "&", "|", "^", "?",
    "==", "!=", ">=", "<=",
    "+=", "-=", "*=", "/=", "//=",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del",
    "None", "in", "is", "and", "or", "not", "True", "False", "global",
    "goto", "label",      // extended keywords, not available in cpython
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise",
    /** KW_END **/
    "is not", "not in",
    "@id", "@num", "@str", "@fstr",
    "@indent", "@dedent"
};

const _TokenType __TOKENS_LEN = sizeof(__TOKENS) / sizeof(__TOKENS[0]);

constexpr _TokenType TK(const char* const token) {
    for(int k=0; k<__TOKENS_LEN; k++){
        const char* i = __TOKENS[k];
        const char* j = token;
        while(*i && *j && *i == *j){
            i++; j++;
        }
        if(*i == *j) return k;
    }
    return 0;
}

#define TK_STR(t) __TOKENS[t]

const _TokenType __KW_BEGIN = TK("class");
const _TokenType __KW_END = TK("raise");

const std::unordered_map<std::string_view, _TokenType> __KW_MAP = [](){
    std::unordered_map<std::string_view, _TokenType> map;
    for(int k=__KW_BEGIN; k<=__KW_END; k++) map[__TOKENS[k]] = k;
    return map;
}();


struct Token{
  _TokenType type;

  const char* start; //< Begining of the token in the source.
  int length;        //< Number of chars of the token.
  int line;          //< Line number of the token (1 based).
  PyVar value;       //< Literal value of the token.

  const _Str str() const {
    return _Str(start, length);
  }

  const _Str info() const {
    _StrStream ss;
    _Str raw = str();
    if (raw == _Str("\n")) raw = "\\n";
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
    _Source src;

    const char* token_start;
    const char* current_char;
    int current_line = 1;
    Token previous, current;
    std::queue<Token> nexts;
    std::stack<int> indents;

    int brackets_level_0 = 0;
    int brackets_level_1 = 0;
    int brackets_level_2 = 0;

    Token nextToken(){
        if(nexts.empty()) return makeErrToken();
        Token t = nexts.front();
        if(t.type == TK("@eof") && indents.size()>1){
            nexts.pop();
            indents.pop();
            return Token{TK("@dedent"), token_start, 0, current_line};
        }
        nexts.pop();
        return t;
    }

    char peekChar() {
        return *current_char;
    }

    char peekNextChar() {
        if (peekChar() == '\0') return '\0';
        return *(current_char + 1);
    }

    int eatSpaces(){
        int count = 0;
        while (true) {
            switch (peekChar()) {
                case ' ': count++; break;
                case '\t': count+=4; break;
                default: return count;
            }
            eatChar();
        }
    }

    bool eatIndentation(){
        if(brackets_level_0 > 0 || brackets_level_1 > 0 || brackets_level_2 > 0) return true;
        int spaces = eatSpaces();
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

    char eatChar() {
        char c = peekChar();
        if(c == '\n') throw std::runtime_error("eatChar() cannot consume a newline");
        current_char++;
        return c;
    }

    char eatCharIncludeNewLine() {
        char c = peekChar();
        current_char++;
        if (c == '\n'){
            current_line++;
            src->lineStarts.push_back(current_char);
        }
        return c;
    }

    inline bool isNameStart(char c){
        if(isalpha(c) || c=='_') return true;
        if(!isascii(c)) return true;
        return false;
    }

    int eatName() {
        current_char--;
        while(true){
            uint8_t c = peekChar();
            int u8bytes = 0;
            if((c & 0b10000000) == 0b00000000) u8bytes = 1;
            else if((c & 0b11100000) == 0b11000000) u8bytes = 2;
            else if((c & 0b11110000) == 0b11100000) u8bytes = 3;
            else if((c & 0b11111000) == 0b11110000) u8bytes = 4;
            else return 1;
            if(u8bytes == 1){
                if(isalpha(c) || c=='_' || isdigit(c)) {
                    current_char++;
                    continue;
                }else{
                    break;
                }
            }
            // handle multibyte char
            std::string u8str(current_char, u8bytes);
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
            if(__isLoChar(value)) current_char += u8bytes;
            else break;
        }

        int length = (int)(current_char - token_start);
        if(length == 0) return 3;
        std::string_view name(token_start, length);
        if(__KW_MAP.count(name)){
            if(name == "not"){
                if(strncmp(current_char, " in", 3) == 0){
                    current_char += 3;
                    setNextToken(TK("not in"));
                    return 0;
                }
            }else if(name == "is"){
                if(strncmp(current_char, " not", 4) == 0){
                    current_char += 4;
                    setNextToken(TK("is not"));
                    return 0;
                }
            }
            setNextToken(__KW_MAP.at(name));
        } else {
            setNextToken(TK("@id"));
        }
        return 0;
    }

    void skipLineComment() {
        char c;
        while ((c = peekChar()) != '\0') {
            if (c == '\n') return;
            eatChar();
        }
    }
    
    // If the current char is [c] consume it and advance char by 1 and returns
    // true otherwise returns false.
    bool matchChar(char c) {
        if (peekChar() != c) return false;
        eatCharIncludeNewLine();
        return true;
    }

    // Returns an error token from the current position for reporting error.
    Token makeErrToken() {
        return Token{TK("@error"), token_start, (int)(current_char - token_start), current_line};
    }

    // Initialize the next token as the type.
    void setNextToken(_TokenType type, PyVar value=nullptr) {

        switch(type){
            case TK("("): brackets_level_0++; break;
            case TK(")"): brackets_level_0--; break;
            case TK("["): brackets_level_1++; break;
            case TK("]"): brackets_level_1--; break;
            case TK("{"): brackets_level_2++; break;
            case TK("}"): brackets_level_2--; break;
        }

        nexts.push( Token{
            type,
            token_start,
            (int)(current_char - token_start),
            current_line - ((type == TK("@eol")) ? 1 : 0),
            value
        });
    }

    void setNextTwoCharToken(char c, _TokenType one, _TokenType two) {
        if (matchChar(c)) setNextToken(two);
        else setNextToken(one);
    }

    Parser(_Source src) {
        this->src = src;
        this->token_start = src->source;
        this->current_char = src->source;
        this->nexts.push(Token{TK("@sof"), token_start, 0, current_line});
        this->indents.push(0);
    }
};