#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/line_profiler.h"
#include "pocketpy/objects/sourcedata.h"
#include <assert.h>

void LineProfiler__ctor(LineProfiler* self) {
    c11_smallmap_p2i__ctor(&self->records);
    self->prev_loc.src = NULL;
    self->prev_time = 0;
    self->enabled = false;
}

void LineProfiler__dtor(LineProfiler* self) {
    for(int i = 0; i < self->records.length; i++) {
        LineRecord* lines = c11__getitem(LineRecord*, &self->records, i);
        PK_FREE(lines);
    }
    c11_smallmap_p2i__dtor(&self->records);
}

LineRecord* LineProfiler__get_record(LineProfiler* self, SourceLocation loc) {
    LineRecord* lines = (LineRecord*)c11_smallmap_p2i__get(&self->records, loc.src, 0);
    if(lines == NULL) {
        int max_lineno = loc.src->line_starts.length;
        lines = PK_MALLOC(sizeof(LineRecord) * (max_lineno + 1));
        memset(lines, 0, sizeof(LineRecord) * (max_lineno + 1));
        c11_smallmap_p2i__set(&self->records, loc.src, (py_i64)lines);
    }
    return &lines[loc.lineno];
}

void LineProfiler__begin(LineProfiler* self) {
    assert(!self->enabled);
    self->prev_loc.src = NULL;
    self->prev_time = 0;
    self->enabled = true;
}

void LineProfiler__tracefunc_line(LineProfiler* self, py_Frame* frame) {
    assert(self->enabled);
    clock_t now = clock();
    if(self->prev_loc.src != NULL) {
        LineRecord* line = LineProfiler__get_record(self, self->prev_loc);
        line->hits++;
        line->time += now - self->prev_time;
    }
    self->prev_loc = Frame__source_location(frame);
    self->prev_time = now;
}

void LineProfiler__end(LineProfiler* self) {
    assert(self->enabled);
    if(self->prev_loc.src != NULL) {
        LineRecord* line = LineProfiler__get_record(self, self->prev_loc);
        line->hits++;
        line->time += clock() - self->prev_time;
    }
    self->enabled = false;
}

void LineProfiler__reset(LineProfiler* self) {
    LineProfiler__dtor(self);
    LineProfiler__ctor(self);
}

c11_string* LineProfiler__get_report(LineProfiler* self) {
    c11_sbuf sbuf;
    c11_sbuf__ctor(&sbuf);
    c11_sbuf__write_char(&sbuf, '{');
    for(int i = 0; i < self->records.length; i++) {
        c11_smallmap_p2i_KV kv = c11__getitem(c11_smallmap_p2i_KV, &self->records, i);
        SourceData_ src = (SourceData_)kv.key;
        int line_record_length = src->line_starts.length + 1;
        c11_sv src_name = c11_string__sv(src->filename);
        c11_sbuf__write_quoted(&sbuf, src_name, '"');
        c11_sbuf__write_cstr(&sbuf, ": [");
        LineRecord* lines = (LineRecord*)kv.value;
        for(int j = 1; j < line_record_length; j++) {
            // [<j>, <hits>, <time>]
            c11_sbuf__write_cstr(&sbuf, "[");
            c11_sbuf__write_int(&sbuf, j);
            c11_sbuf__write_cstr(&sbuf, ", ");
            c11_sbuf__write_i64(&sbuf, lines[j].hits);
            c11_sbuf__write_cstr(&sbuf, ", ");
            c11_sbuf__write_i64(&sbuf, lines[j].time);
            c11_sbuf__write_cstr(&sbuf, "]");
            if(j < line_record_length - 1) c11_sbuf__write_cstr(&sbuf, ", ");
        }
        c11_sbuf__write_cstr(&sbuf, "]");
        if(i < self->records.length - 1) c11_sbuf__write_cstr(&sbuf, ", ");
    }
    c11_sbuf__write_char(&sbuf, '}');
    return c11_sbuf__submit(&sbuf);
}
