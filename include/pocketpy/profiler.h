#pragma once

#include "frame.h"

namespace pkpy {

struct LineRecord{
    int line;
    i64 hits;
    clock_t time;

    LineRecord(): line(-1), hits(0), time(0) {}
    bool is_valid() const { return line != -1; }
};

struct FrameRecord{
    FrameId frame;
    clock_t prev_time;
    LineRecord* prev_record;
    int prev_line;
};

struct LineProfiler{
    // filename -> records
    std::map<std::string_view, std::vector<LineRecord>> records;
    stack<FrameRecord> frames;
    std::set<FuncDecl*> functions;

    void begin();
    void _step(FrameId frame);
    void _step_end(FrameId frame);
    void end();
    Str stats();
};

} // namespace pkpy
