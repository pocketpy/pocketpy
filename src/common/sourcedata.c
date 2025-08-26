#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/common/sstream.h"
#include <ctype.h>
#include <string.h>

static void SourceData__ctor(struct SourceData* self,
                             const char* source,
                             const char* filename,
                             enum py_CompileMode mode,
                             bool is_dynamic) {
    self->filename = c11_string__new(filename);
    self->mode = mode;
    c11_vector__ctor(&self->line_starts, sizeof(const char*));

    // Skip utf8 BOM if there is any.
    if(strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;
    // Drop all '\r'
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    while(true) {
        char c = *source;
        if(c == '\0') break;
        if(c != '\r') c11_sbuf__write_char(&ss, c);
        source++;
    }
    self->source = c11_sbuf__submit(&ss);
    // remove trailing newline
    int last_index = self->source->size - 1;
    while(last_index >= 0 && isspace((unsigned char)self->source->data[last_index])) {
        last_index--;
    }
    if(last_index >= 0) {
        self->source->size = last_index + 1;
        self->source->data[last_index + 1] = '\0';
    }

    self->is_dynamic = is_dynamic;
    c11_vector__push(const char*, &self->line_starts, self->source->data);
}

static void SourceData__dtor(struct SourceData* self) {
    c11_string__delete(self->filename);
    c11_string__delete(self->source);
    c11_vector__dtor(&self->line_starts);
}

SourceData_ SourceData__rcnew(const char* source,
                              const char* filename,
                              enum py_CompileMode mode,
                              bool is_dynamic) {
    SourceData_ self = PK_MALLOC(sizeof(struct SourceData));
    SourceData__ctor(self, source, filename, mode, is_dynamic);
    self->rc.count = 1;
    self->rc.dtor = (void (*)(void*))SourceData__dtor;
    return self;
}

bool SourceData__get_line(const struct SourceData* self,
                          int lineno,
                          const char** st,
                          const char** ed) {
    if(lineno < 0) return false;
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

void SourceData__snapshot(const struct SourceData* self,
                          c11_sbuf* ss,
                          int lineno,
                          const char* cursor,
                          const char* name) {
    pk_sprintf(ss, "  File \"%s\", line %d", self->filename->data, lineno);

    if(name && *name) {
        c11_sbuf__write_cstr(ss, ", in ");
        c11_sbuf__write_cstr(ss, name);
    }

    c11_sbuf__write_char(ss, '\n');
    const char *st = NULL, *ed;
    if(SourceData__get_line(self, lineno, &st, &ed)) {
        while(st < ed && isblank(*st))
            ++st;
        if(st < ed) {
            c11_sbuf__write_cstr(ss, "    ");
            c11_sbuf__write_cstrn(ss, st, ed - st);
            if(cursor && st <= cursor && cursor <= ed) {
                c11_sbuf__write_cstr(ss, "\n    ");
                for(int i = 0; i < (cursor - st); ++i)
                    c11_sbuf__write_char(ss, ' ');
                c11_sbuf__write_cstr(ss, "^");
            }
        } else {
            st = NULL;
        }
    }

    if(!st) { c11_sbuf__write_cstr(ss, "    <?>"); }
}
