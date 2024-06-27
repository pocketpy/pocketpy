#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/common/sstream.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void pk_SourceData__ctor(struct pk_SourceData* self,
                           const char* source,
                           const char* filename,
                           enum CompileMode mode) {
    py_Str__ctor(&self->filename, filename);
    self->mode = mode;
    c11_vector__ctor(&self->line_starts, sizeof(const char*));
    c11_vector__ctor(&self->_precompiled_tokens, sizeof(py_Str));

    // Skip utf8 BOM if there is any.
    if(strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
    // Drop all '\r'
    pk_SStream ss;
    pk_SStream__ctor(&ss);
    while(true){
        char c = *source;
        if(c == '\0') break;
        if(c != '\r') pk_SStream__write_char(&ss, c);
        source++;
    }
    self->source = pk_SStream__submit(&ss);
    source = py_Str__data(&self->source);
    self->is_precompiled = (strncmp(source, "pkpy:", 5) == 0);
    c11_vector__push(const char*, &self->line_starts, source);
}

void pk_SourceData__dtor(struct pk_SourceData* self) {
    py_Str__dtor(&self->filename);
    py_Str__dtor(&self->source);
    c11_vector__dtor(&self->line_starts);

    for(int i=0; i<self->_precompiled_tokens.count; i++){
        py_Str__dtor(c11__at(py_Str, &self->_precompiled_tokens, i));
    }
    c11_vector__dtor(&self->_precompiled_tokens);
}

pk_SourceData_ pk_SourceData__rcnew(const char* source, const char* filename, enum CompileMode mode) {
    pk_SourceData_ self = malloc(sizeof(struct pk_SourceData));
    pk_SourceData__ctor(self, source, filename, mode);
    self->rc.count = 1;
    self->rc.dtor = (void(*)(void*))pk_SourceData__dtor;
    return self;
}

bool pk_SourceData__get_line(const struct pk_SourceData* self, int lineno, const char** st, const char** ed) {
    if(self->is_precompiled || lineno == -1) { return false; }
    lineno -= 1;
    if(lineno < 0) lineno = 0;
    const char* _start = c11__getitem(const char*, &self->line_starts, lineno);
    const char* i = _start;
    // max 300 chars
    while(*i != '\n' && *i != '\0' && i - _start < 300)
        i++;
    *st = _start;
    *ed = i;
    return true;
}

py_Str pk_SourceData__snapshot(const struct pk_SourceData* self, int lineno, const char* cursor, const char* name) {
    pk_SStream ss;
    pk_SStream__ctor(&ss);

    // pk_SStream__write_cstr(&ss, "  File \"");
    // pk_SStream__write_Str(&ss, &self->filename);
    // pk_SStream__write_cstr(&ss, "\", line ");
    // pk_SStream__write_int(&ss, lineno);

    pk_SStream__write(&ss,
        "  File \"{}\", line {}",
        &self->filename,
        lineno
    );

    if(name && *name) {
        pk_SStream__write_cstr(&ss, ", in ");
        pk_SStream__write_cstr(&ss, name);
    }

    if(!self->is_precompiled) {
        pk_SStream__write_char(&ss, '\n');
        const char *st = NULL, *ed;
        if(pk_SourceData__get_line(self, lineno, &st, &ed)) {
            while(st < ed && isblank(*st))
                ++st;
            if(st < ed) {
                pk_SStream__write_cstr(&ss, "    ");
                pk_SStream__write_cstrn(&ss, st, ed - st);
                if(cursor && st <= cursor && cursor <= ed) {
                    pk_SStream__write_cstr(&ss, "\n    ");
                    for(int i = 0; i < (cursor - st); ++i)
                        pk_SStream__write_char(&ss, ' ');
                    pk_SStream__write_cstr(&ss, "^");
                }
            } else {
                st = NULL;
            }
        }

        if(!st) { pk_SStream__write_cstr(&ss, "    <?>"); }
    }
    return pk_SStream__submit(&ss);
}
