#pragma once

#include "frame.h"

namespace pkpy {

struct LineRecord{
    int line;
    SourceData* src;

    i64 hits;
    clock_t time;

    LineRecord(int line, SourceData* src): line(line), src(src), hits(0), time(0) {}

    std::string_view line_content() const;
};

struct LineProfiler{
    clock_t prev_time;
    LineRecord* prev_record;
    int prev_line;

    // filename -> records
    std::map<std::string_view, std::map<int, LineRecord>> records;

    void begin();
    void _step(Frame* frame);
    void _step_end();
    void end();
    Str stats();
};

} // namespace pkpy
