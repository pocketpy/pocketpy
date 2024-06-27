#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/sourcedata.h"

typedef struct pk_Lexer{
    pk_SourceData_ src;
    const char* token_start;
    const char* curr_char;
    int current_line;
    int brackets_level;

    c11_vector/*T=Token*/ nexts;
    c11_vector/*T=int*/ indents;
} pk_Lexer;

const static TokenValue EmptyTokenValue;

void pk_Lexer__ctor(pk_Lexer* self, pk_SourceData_ src){
    PK_INCREF(src);
    self->src = src;
    self->curr_char = self->token_start = py_Str__data(&src->source);
    self->current_line = 1;
    self->brackets_level = 0;
    c11_vector__ctor(&self->nexts, sizeof(Token));
    c11_vector__ctor(&self->indents, sizeof(int));
}

void pk_Lexer__dtor(pk_Lexer* self){
    PK_DECREF(self->src);
    c11_vector__dtor(&self->nexts);
    c11_vector__dtor(&self->indents);
}

void* pk_Lexer__run(pk_SourceData_ src, void** out_tokens){
    pk_Lexer lexer;
    pk_Lexer__ctor(&lexer, src);

    if(src->is_precompiled) {
        pk_Lexer__dtor(&lexer);
        return from_precompiled();
    }
    // push initial tokens
    Token sof = {TK_SOF, lexer.token_start, 0, lexer.current_line, lexer.brackets_level, EmptyTokenValue};
    c11_vector__push(Token, &lexer.nexts, sof);
    c11_vector__push(int, &lexer.indents, 0);

    bool eof = false;
    while(!eof) {
        void* err = lex_one_token(&eof);
        if(err){
            pk_Lexer__dtor(&lexer);
            return err;
        }
    }
    pk_Lexer__dtor(&lexer);
    return NULL;
}

char eatchar(pk_Lexer* self){
    char c = *self->curr_char;
    assert(c != '\n');  // eatchar() cannot consume a newline
    self->curr_char++;
    return c;
}

char eatchar_include_newline(pk_Lexer* self){
    char c = *self->curr_char;
    self->curr_char++;
    if(c == '\n') {
        self->current_line++;
        c11_vector__push(const char*, &self->src->line_starts, self->curr_char);
    }
    return c;
}

int eat_spaces(pk_Lexer* self){
    int count = 0;
    while(true) {
        switch(*self->curr_char) {
            case ' ': count += 1; break;
            case '\t': count += 4; break;
            default: return count;
        }
        eatchar(self);
    }
}

bool matchchar(pk_Lexer* self, char c){
    if(*self->curr_char != c) return false;
    eatchar_include_newline(self);
    return true;
}

bool match_n_chars(pk_Lexer* self, int n, char c0){
    const char* c = self->curr_char;
    for(int i = 0; i < n; i++) {
        if(*c == '\0') return false;
        if(*c != c0) return false;
        c++;
    }
    for(int i = 0; i < n; i++)
        eatchar_include_newline(self);
    return true;
}

bool match_string(pk_Lexer* self, const char* s){
    int s_len = strlen(s);
    if(strncmp(self->curr_char, s, s_len) == 0){
        for(int i = 0; i < s_len; i++)
            eatchar_include_newline(self);
    }
    return ok;
}

void skip_line_comment(pk_Lexer* self){
    while(*self->curr_char) {
        if(*self->curr_char == '\n') return;
        eatchar(self);
    }
}

void add_token(pk_Lexer* self, TokenIndex type, TokenValue value){
    switch(type) {
        case TK_LBRACE:
        case TK_LBRACKET:
        case TK_LPAREN: self->brackets_level++; break;
        case TK_RPAREN:
        case TK_RBRACKET:
        case TK_RBRACE: self->brackets_level--; break;
        default: break;
    }
    Token token = {type,
                       self->token_start,
                       (int)(self->curr_char - self->token_start),
                       self->current_line - ((type == TK_EOL) ? 1 : 0),
                       self->brackets_level,
                       value};
    // handle "not in", "is not", "yield from"
    if(self->nexts.count > 0) {
        Token* back = &c11_vector__back(Token, &self->nexts);
        if(back->type == TK_NOT_KW && type == TK_IN) {
            back->type = TK_NOT_IN;
            return;
        }
        if(back->type == TK_IS && type == TK_NOT_KW) {
            back->type = TK_IS_NOT;
            return;
        }
        if(back->type == TK_YIELD && type == TK_FROM) {
            back->type = TK_YIELD_FROM;
            return;
        }
        c11_vector__push(Token, &self->nexts, token);
    }
}


void add_token_2(pk_Lexer* self, char c, TokenIndex one, TokenIndex two){
    if(matchchar(self, c))
        add_token(self, two, EmptyTokenValue);
    else
        add_token(self, one, EmptyTokenValue);
}

bool eat_indentation(pk_Lexer* self){
    if(self->brackets_level > 0) return true;
    int spaces = eat_spaces(self);
    if(*self->curr_char == '#') skip_line_comment();
    if(*self->curr_char == '\0' || *self->curr_char == '\n'){
        return true;
    }
    // https://docs.python.org/3/reference/lexical_analysis.html#indentation
    int indents_back = c11_vector__back(int, &self->indents);
    if(spaces > indents_back) {
        c11_vector__push(int, &self->indents, spaces);
        Token t = {TK_INDENT, self->token_start, 0, self->current_line, self->brackets_level, EmptyTokenValue};
        c11_vector__push(Token, &self->nexts, t);
    } else if(spaces < indents_back) {
        do {
            c11_vector__pop(int, &self->indents);
            Token t = {TK_DEDENT, self->token_start, 0, self->current_line, self->brackets_level, EmptyTokenValue};
            c11_vector__push(Token, &self->nexts, t);
            indents_back = c11_vector__back(int, &self->indents);
        } while(spaces < indents_back);
        if(spaces != indents_back) { return false; }
    }
    return true;
}
