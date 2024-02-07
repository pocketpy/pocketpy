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

struct LineProfiler{
    clock_t prev_time;
    LineRecord* prev_record;
    int prev_line;

    // filename -> records
    std::map<std::string_view, std::vector<LineRecord>> records;

    std::set<FuncDecl*> functions;

    void begin();
    void _step(Frame* frame);
    void _step_end();
    void end();
    Str stats();
};

} // namespace pkpy
