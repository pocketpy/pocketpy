#pragma once

#include "pocketpy/pocketpy.h"
#include <time.h>

#include "pocketpy/interpreter/frame.h"

typedef struct LineRecord {
    py_i64 hits;
    clock_t time;
} LineRecord;

typedef struct FrameRecord {
    py_Frame* frame;
    clock_t prev_time;
    LineRecord* prev_line;
    bool is_lambda;
} FrameRecord;

typedef struct LineProfiler {
    c11_smallmap_p2i records;  // SourceData* -> LineRecord[]
    c11_vector /*T=FrameRecord*/ frame_records;  // FrameRecord[]
    bool enabled;
} LineProfiler;

void LineProfiler__ctor(LineProfiler* self);
void LineProfiler__dtor(LineProfiler* self);
LineRecord* LineProfiler__get_record(LineProfiler* self, SourceLocation loc);
void LineProfiler__begin(LineProfiler* self);
void LineProfiler__tracefunc_internal(LineProfiler* self, py_Frame* frame, enum py_TraceEvent event);
void LineProfiler__end(LineProfiler* self);
void LineProfiler__reset(LineProfiler* self);
c11_string* LineProfiler__get_report(LineProfiler* self);

void LineProfiler_tracefunc(py_Frame* frame, enum py_TraceEvent event);