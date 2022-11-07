#pragma once

#include <string_view>
#include <cstring>
#include <queue>

#include "obj.h"

typedef uint8_t _TokenType;

constexpr const char* __TOKENS[] = {
    "@error", "@eof", "@eol", "@sof",
    ".", ",", ":", ";", "#", "(", ")", "[", "]", "{", "}", "%",
    "+", "-", "*", "/", "//", "**", "=", ">", "<",
    "==", "!=", ">=", "<=",
    "+=", "-=", "*=", "/=", "//=",
    /** KW_BEGIN **/
    "class", "import", "as", "def", "lambda", "pass", "del",
    "None", "in", "is", "and", "or", "not", "True", "False",
    "while", "for", "if", "elif", "else", "break", "continue", "return", "assert", "raise",
    /** KW_END **/
    "is not", "not in",
    "@id", "@num", "@str",
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
};

enum Precedence {
  PREC_NONE,
  PREC_LOWEST,
  PREC_ASSIGNMENT,    // =
  PREC_LOGICAL_OR,    // or
  PREC_LOGICAL_AND,   // and
  PREC_EQUALITY,      // == !=
  PREC_TEST,          // in is
  PREC_COMPARISION,   // < > <= >=
  PREC_TERM,          // + -
  PREC_FACTOR,        // * / %
  PREC_UNARY,         // - not
  PREC_EXPONENT,      // **
  PREC_CALL,          // ()
  PREC_SUBSCRIPT,     // []
  PREC_ATTRIB,        // .index
  PREC_PRIMARY,
};

// The context of the parsing phase for the compiler.
struct Parser {
    const char* source;         //< Currently compiled source.
    const char* token_start;    //< Start of the currently parsed token.
    const char* current_char;   //< Current char position in the source.
    const char* line_start;     //< Start of the current line.

    int current_line = 1;

    Token previous, current;
    std::queue<Token> nexts;

    std::stack<int> indents;

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
            line_start = current_char;
        }
        return c;
    }

    void eatName() {
        char c = peekChar();
        while (isalpha(c) || c=='_' || isdigit(c)) {
            eatChar();
            c = peekChar();
        }

        const char* name_start = token_start;
        int length = (int)(current_char - name_start);
        std::string_view name(name_start, length);
        if(__KW_MAP.count(name)){
            if(name == "not"){
                if(strncmp(current_char, " in", 3) == 0){
                    current_char += 3;
                    setNextToken(TK("not in"));
                    return;
                }
            }else if(name == "is"){
                if(strncmp(current_char, " not", 4) == 0){
                    current_char += 4;
                    setNextToken(TK("is not"));
                    return;
                }
            }
            setNextToken(__KW_MAP.at(name));
        } else {
            setNextToken(TK("@id"));
        }
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

    Parser(const char* source) {
        this->source = source;
        this->token_start = source;
        this->current_char = source;
        this->line_start = source;

        this->nexts.push(Token{TK("@sof"), token_start, 0, current_line});

        this->indents.push(0);
    }
};