#include "pocketpy/common/config.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/compiler/lexer.h"

const char* pk_TokenSymbols[] = {
    "@eof", "@eol", "@sof",
    "@id", "@num", "@str", "@fstr", "@long", "@bytes", "@imag",
    "@indent", "@dedent",
    // These 3 are compound keywords which are generated on the fly
    "is not", "not in", "yield from",
    /*****************************************/
    "+", "+=", "-", "-=",   // (INPLACE_OP - 1) can get '=' removed
    "*", "*=", "/", "/=", "//", "//=", "%", "%=",
    "&", "&=", "|", "|=", "^", "^=", 
    "<<", "<<=", ">>", ">>=",
    /*****************************************/
    "(", ")", "[", "]", "{", "}",
    ".", "..", "...", ",", ":", ";",
    "**", "->", "#", "@",
    ">", "<", "=", "==", "!=", ">=", "<=", "~",
    /** KW_BEGIN **/
    // NOTE: These keywords should be sorted in ascending order!!
    "False", "None", "True", "and", "as", "assert", "break", "class", "continue",
    "def", "del", "elif", "else", "except", "finally", "for", "from", "global",
    "if", "import", "in", "is", "lambda", "not", "or", "pass", "raise", "return",
    "try", "while", "with", "yield",
};

void pkpy_TokenDeserializer__ctor(pkpy_TokenDeserializer* self, const char* source){
    self->curr = source;
    self->source = source;
}

bool pkpy_TokenDeserializer__match_char(pkpy_TokenDeserializer* self, char c){
    if(*self->curr == c) {
        self->curr++;
        return true;
    }
    return false;
}

c11_string pkpy_TokenDeserializer__read_string(pkpy_TokenDeserializer* self, char c){
    const char* start = self->curr;
    while(*self->curr != c)
        self->curr++;
    c11_string retval = {start, (int)(self->curr-start)};
    self->curr++;  // skip the delimiter
    return retval;
}

pkpy_Str pkpy_TokenDeserializer__read_string_from_hex(pkpy_TokenDeserializer* self, char c){
    c11_string sv = pkpy_TokenDeserializer__read_string(self, c);
    const char* s = sv.data;
    char* buffer = (char*)malloc(sv.size / 2 + 1);
    for(int i = 0; i < sv.size; i += 2) {
        char c = 0;
        if(s[i] >= '0' && s[i] <= '9')
            c += s[i] - '0';
        else if(s[i] >= 'a' && s[i] <= 'f')
            c += s[i] - 'a' + 10;
        else
            assert(false);
        c <<= 4;
        if(s[i + 1] >= '0' && s[i + 1] <= '9')
            c += s[i + 1] - '0';
        else if(s[i + 1] >= 'a' && s[i + 1] <= 'f')
            c += s[i + 1] - 'a' + 10;
        else
            assert(false);
        buffer[i / 2] = c;
    }
    buffer[sv.size / 2] = 0;
    return (pkpy_Str){
        .size = sv.size / 2,
        .is_ascii = c11__isascii(buffer, sv.size / 2),
        .is_sso = false,
        ._ptr = buffer
    };
}

int pkpy_TokenDeserializer__read_count(pkpy_TokenDeserializer* self){
    assert(*self->curr == '=');
    self->curr++;
    return pkpy_TokenDeserializer__read_uint(self, '\n');
}

int64_t pkpy_TokenDeserializer__read_uint(pkpy_TokenDeserializer* self, char c){
    int64_t out = 0;
    while(*self->curr != c) {
        out = out * 10 + (*self->curr - '0');
        self->curr++;
    }
    self->curr++;  // skip the delimiter
    return out;
}

double pkpy_TokenDeserializer__read_float(pkpy_TokenDeserializer* self, char c){
    c11_string sv = pkpy_TokenDeserializer__read_string(self, c);
    pkpy_Str nullterm;
    pkpy_Str__ctor2(&nullterm, sv.data, sv.size);
    char* end;
    double retval = strtod(pkpy_Str__data(&nullterm), &end);
    pkpy_Str__dtor(&nullterm);
    assert(*end == 0);
    return retval;
}
