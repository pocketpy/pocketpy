#include "pocketpy/interpreter/line_profiler.h"

void LineProfiler__ctor(LineProfiler* self) {
    c11_smallmap_p2i__ctor(&self->records);
    self->prev_loc.src = NULL;
    self->prev_time = 0;
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

void LineProfiler__begin(LineProfiler* self, bool reset) {
    if(self->records.length > 0 && reset) {
        LineProfiler__dtor(self);
        LineProfiler__ctor(self);
    }
    self->prev_loc.src = NULL;
    self->prev_time = 0;
    VM__set_line_profiler(pk_current_vm, self);
}

void LineProfiler__tracefunc_line(LineProfiler* self, py_Frame* frame) {
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
    if(self->prev_loc.src != NULL) {
        LineRecord* line = LineProfiler__get_record(self, self->prev_loc);
        line->hits++;
        line->time += clock() - self->prev_time;
    }
    VM__set_line_profiler(pk_current_vm, NULL);
}
