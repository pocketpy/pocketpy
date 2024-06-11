#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/common/sstream.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void pkpy_SourceData__ctor(struct pkpy_SourceData* self,
                           const char* source,
                           int source_size,
                           const pkpy_Str* filename,
                           enum pkpy_CompileMode mode) {
    pkpy_Rcptr__ctor_withd(self, &pkpy_SourceData__dtor);

    self->filename = pkpy_Str__copy(filename);  // OPTIMIZEME?
    self->mode = mode;

    c11_vector__ctor(&self->line_starts, sizeof(const char*));
    c11_vector__ctor(&self->_precompiled_tokens, sizeof(pkpy_Str));

    int index = (strncmp(source, "\xEF\xBB\xBF", 3) == 0) ? 3 : 0;
    int len = source_size - index;
    for(int i = 0; i < source_size; ++i)
        len -= (source[i] == '\r');

    char *buf = malloc(len + 1), *p = buf;
    buf[len] = '\0';
    for(; index < source_size; ++index) {
        if(source[index] != '\r') *(p++) = source[index];
    }
    pkpy_Str__take_buf(&self->source, buf, len);

    self->is_precompiled = (strncmp(pkpy_Str__data(&self->source), "pkpy:", 5) == 0);
    c11_vector__push_back(const char*, &self->line_starts, pkpy_Str__data(&self->source));
}

void pkpy_SourceData__dtor(struct pkpy_SourceData* self) {
    pkpy_Str__dtor(&self->filename);
    pkpy_Str__dtor(&self->source);
    c11_vector__dtor(&self->line_starts);
    c11_vector__dtor(&self->_precompiled_tokens);
}

bool pkpy_SourceData__get_line(const struct pkpy_SourceData* self, int lineno, const char** st, const char** ed) {
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

pkpy_Str pkpy_SourceData__snapshot(const struct pkpy_SourceData* self, int lineno, const char* cursor, const char* name) {
    pkpy_SStream ss;
    pkpy_SStream__ctor(&ss);
    pkpy_SStream__append_cstr(&ss, "  File \"");
    pkpy_SStream__append_Str(&ss, &self->filename);
    pkpy_SStream__append_cstr(&ss, "\", line ");
    pkpy_SStream__append_int(&ss, lineno);

    if(name) {
        pkpy_SStream__append_cstr(&ss, ", in ");
        pkpy_SStream__append_cstr(&ss, name);
    }

    if(!self->is_precompiled) {
        pkpy_SStream__append_char(&ss, '\n');
        const char *st = NULL, *ed;
        if(pkpy_SourceData__get_line(self, lineno, &st, &ed)) {
            while(st < ed && isblank(*st))
                ++st;
            if(st < ed) {
                pkpy_SStream__append_cstr(&ss, "    ");
                pkpy_SStream__append_cstrn(&ss, st, ed - st);
                if(cursor && st <= cursor && cursor <= ed) {
                    pkpy_SStream__append_cstr(&ss, "\n    ");
                    for(int i = 0; i < (cursor - st); ++i)
                        pkpy_SStream__append_char(&ss, ' ');
                    pkpy_SStream__append_cstr(&ss, "^");
                }
            } else {
                st = NULL;
            }
        }

        if(!st) { pkpy_SStream__append_cstr(&ss, "    <?>"); }
    }
    pkpy_Str res = pkpy_SStream__to_Str(&ss);
    pkpy_SStream__dtor(&ss);
    return res;
}
