#include "pocketpy/compiler/compiler.h"

Error* pk_compile(pk_SourceData_ src){
    c11_array/*T=Token*/ tokens;
    Error* err = pk_Lexer__process(src, &tokens);
    if(err) return err;

    Token* data = (Token*)tokens.data;
    printf("%s\n", py_Str__data(&src->filename));
    for(int i = 0; i < tokens.count; i++) {
        Token* t = data + i;
        py_Str tmp;
        py_Str__ctor2(&tmp, t->start, t->length);
        printf("[%d] %s: %s\n", t->line, pk_TokenSymbols[t->type], py_Str__data(&tmp));
        py_Str__dtor(&tmp);
    }

    c11_array__dtor(&tokens);
    return NULL;
}