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
    bool enabled;
} LineProfiler;

void LineProfiler__ctor(LineProfiler* self);
void LineProfiler__dtor(LineProfiler* self);
LineRecord* LineProfiler__get_record(LineProfiler* self, SourceLocation loc);
void LineProfiler__begin(LineProfiler* self);
void LineProfiler__tracefunc_line(LineProfiler* self, py_Frame* frame);
void LineProfiler__end(LineProfiler* self);
void LineProfiler__reset(LineProfiler* self);

void LineProfiler__tracefunc(py_Frame* frame, enum py_TraceEvent event);