#pragma once

#include "frame.h"

namespace pkpy {

struct _LineRecord{
    int line;
    i64 hits;
    clock_t time;

    _LineRecord(): line(-1), hits(0), time(0) {}
    bool is_valid() const { return line != -1; }
};

struct _FrameRecord{
    LinkedFrame* frame;
    clock_t prev_time;
    _LineRecord* prev_record;
};

struct LineProfiler{
    // filename -> records
    std::map<std::string_view, std::vector<_LineRecord>> records;
    stack_no_copy<_FrameRecord> frames;
    std::set<FuncDecl*> functions;

    void begin();
    void _step(LinkedFrame*);
    void _step_end(LinkedFrame*, int);
    void end();
    Str stats();
};

} // namespace pkpy
