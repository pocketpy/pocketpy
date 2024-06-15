#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/common/sstream.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void pkpy_SourceData__ctor(struct pkpy_SourceData* self,
                           c11_string source,       // may not be null-terminated
                           const pkpy_Str* filename,
                           enum CompileMode mode) {
    self->filename = pkpy_Str__copy(filename);  // OPTIMIZEME?
    self->mode = mode;
    c11_vector__ctor(&self->line_starts, sizeof(const char*));
    c11_vector__ctor(&self->_precompiled_tokens, sizeof(pkpy_Str));

    int index = 0;
    // Skip utf8 BOM if there is any.
    if (source.size >= 3 && strncmp(source.data, "\xEF\xBB\xBF", 3) == 0) index += 3;
    // Drop all '\r'
    pkpy_SStream ss;
    pkpy_SStream__ctor2(&ss, source.size + 1);
    while(index < source.size){
        char c = source.data[index];
        if(c != '\r') pkpy_SStream__write_char(&ss, c);
        index++;
    }
    self->source = pkpy_SStream__submit(&ss);
    self->is_precompiled = (strncmp(pkpy_Str__data(&self->source), "pkpy:", 5) == 0);
    c11_vector__push(const char*, &self->line_starts, pkpy_Str__data(&self->source));
}

void pkpy_SourceData__dtor(struct pkpy_SourceData* self) {
    pkpy_Str__dtor(&self->filename);
    pkpy_Str__dtor(&self->source);
    c11_vector__dtor(&self->line_starts);

    for(int i=0; i<self->_precompiled_tokens.count; i++){
        pkpy_Str__dtor(c11__at(pkpy_Str, &self->_precompiled_tokens, i));
    }
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

    // pkpy_SStream__write_cstr(&ss, "  File \"");
    // pkpy_SStream__write_Str(&ss, &self->filename);
    // pkpy_SStream__write_cstr(&ss, "\", line ");
    // pkpy_SStream__write_int(&ss, lineno);

    pkpy_SStream__write(&ss,
        "  File \"{}\", line {}",
        &self->filename,
        lineno
    );

    if(name) {
        pkpy_SStream__write_cstr(&ss, ", in ");
        pkpy_SStream__write_cstr(&ss, name);
    }

    if(!self->is_precompiled) {
        pkpy_SStream__write_char(&ss, '\n');
        const char *st = NULL, *ed;
        if(pkpy_SourceData__get_line(self, lineno, &st, &ed)) {
            while(st < ed && isblank(*st))
                ++st;
            if(st < ed) {
                pkpy_SStream__write_cstr(&ss, "    ");
                pkpy_SStream__write_cstrn(&ss, st, ed - st);
                if(cursor && st <= cursor && cursor <= ed) {
                    pkpy_SStream__write_cstr(&ss, "\n    ");
                    for(int i = 0; i < (cursor - st); ++i)
                        pkpy_SStream__write_char(&ss, ' ');
                    pkpy_SStream__write_cstr(&ss, "^");
                }
            } else {
                st = NULL;
            }
        }

        if(!st) { pkpy_SStream__write_cstr(&ss, "    <?>"); }
    }
    return pkpy_SStream__submit(&ss);
}
