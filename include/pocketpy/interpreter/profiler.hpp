#pragma once

#include "pocketpy/common/config.h"
#include "pocketpy/interpreter/frame.hpp"

#if PK_ENABLE_PROFILER
#include <ctime>

namespace pkpy {

struct _LineRecord {
    int line;
    i64 hits;
    clock_t time;

    _LineRecord() : line(-1), hits(0), time(0) {}

    bool is_valid() const { return line != -1; }
};

struct _FrameRecord {
    int callstack_size;
    Frame* frame;
    clock_t prev_time;
    _LineRecord* prev_record;
};

struct LineProfiler {
    // filename -> records
    small_map<std::string_view, _LineRecord*> records;
    vector<_FrameRecord> frames;
    vector<FuncDecl*> functions;

    void begin();
    void _step(int, Frame*);
    void _step_end(int, Frame*, int);
    void end();
    Str stats();
    ~LineProfiler();
};

}  // namespace pkpy

#endif