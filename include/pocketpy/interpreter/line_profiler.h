#pragma once

#include "pocketpy/pocketpy.h"
#include <time.h>

#include "pocketpy/interpreter/frame.h"

typedef struct LineRecord {
    py_i64 hits;
    clock_t time;
} LineRecord;

typedef struct LineProfiler {
    c11_smallmap_p2i records;  // SourceData* -> LineRecord[]
    SourceLocation prev_loc;
    clock_t prev_time;
} LineProfiler;

void LineProfiler__ctor(LineProfiler* self);
void LineProfiler__dtor(LineProfiler* self);
LineRecord* LineProfiler__get_record(LineProfiler* self, SourceLocation loc);
void LineProfiler__begin(LineProfiler* self, bool reset);
void LineProfiler__tracefunc_line(LineProfiler* self, py_Frame* frame);
void LineProfiler__end(LineProfiler* self);
void VM__set_line_profiler(VM* self, LineProfiler* profiler);
