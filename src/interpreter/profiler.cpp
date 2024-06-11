#include "pocketpy/interpreter/profiler.hpp"

#if PK_ENABLE_PROFILER
namespace pkpy {

static std::string left_pad(std::string s, int width) {
    int n = width - s.size();
    if(n <= 0) return s;
    return std::string(n, ' ') + s;
}

static std::string to_string_1f(f64 x) {
    char buf[32];
    snprintf(buf, 32, "%.1f", x);
    return buf;
}

void LineProfiler::begin() { frames.clear(); }

void LineProfiler::_step(int callstack_size, Frame* frame) {
    auto line_info = frame->co->lines[frame->ip()];
    if(line_info.is_virtual) return;
    std::string_view filename = frame->co->src.filename().sv();
    int line = line_info.lineno;

    if(frames.empty()) {
        frames.push_back({callstack_size, frame, clock(), nullptr});
    } else {
        _step_end(callstack_size, frame, line);
    }

    _LineRecord* file_records;
    
    auto p = records.try_get(filename);
    if(p == nullptr) {
        int total_lines = frame->co->src->line_starts.size();
        file_records = new _LineRecord[total_lines + 1];
        for(int i = 1; i <= total_lines; i++) file_records[i].line = i;
        records.insert(filename, file_records);
    }else{
        file_records = *p;
    }

    frames.back().prev_record = &file_records[line];
}

void LineProfiler::_step_end(int callstack_size, Frame* frame, int line) {
    clock_t now = clock();
    _FrameRecord& top_frame_record = frames.back();
    _LineRecord* prev_record = top_frame_record.prev_record;

    int id_delta = callstack_size - top_frame_record.callstack_size;
    assert(abs(id_delta) <= 1);

    // current line is about to change
    if(prev_record->line != line) {
        clock_t delta = now - top_frame_record.prev_time;
        top_frame_record.prev_time = now;
        if(id_delta != 1) prev_record->hits++;
        prev_record->time += delta;
    }

    if(id_delta == 1) {
        frames.push_back({callstack_size, frame, now, nullptr});
    } else {
        if(id_delta == -1) frames.pop_back();
    }
}

void LineProfiler::end() {
    clock_t now = clock();
    _FrameRecord& top_frame_record = frames.back();
    _LineRecord* prev_record = top_frame_record.prev_record;

    clock_t delta = now - top_frame_record.prev_time;
    top_frame_record.prev_time = now;
    prev_record->hits++;
    prev_record->time += delta;

    frames.pop_back();
    assert(frames.empty());
}

Str LineProfiler::stats() {
    SStream ss;
    for(FuncDecl* decl: functions) {
        int start_line = decl->code->start_line;
        int end_line = decl->code->end_line;
        if(start_line == -1 || end_line == -1) continue;
        std::string_view filename = decl->code->src.filename().sv();
        const _LineRecord* file_records = records[filename];
        clock_t total_time = 0;
        for(int line = start_line; line <= end_line; line++) {
            total_time += file_records[line].time;
        }
        ss << "Total time: " << (f64)total_time / CLOCKS_PER_SEC << "s\n";
        ss << "File: " << filename << "\n";
        ss << "Function: " << decl->code->name << " at line " << start_line << "\n";
        ss << "Line #      Hits         Time  Per Hit   % Time  Line Contents\n";
        ss << "==============================================================\n";
        for(int line = start_line; line <= end_line; line++) {
            const _LineRecord& record = file_records[line];
            if(!record.is_valid()) continue;
            ss << left_pad(std::to_string(line), 6);
            if(record.hits == 0) {
                ss << std::string(10 + 13 + 9 + 9, ' ');
            } else {
                ss << left_pad(std::to_string(record.hits), 10);
                ss << left_pad(std::to_string(record.time), 13);
                ss << left_pad(std::to_string(record.time / record.hits), 9);
                if(total_time == 0) {
                    ss << left_pad("0.0", 9);
                } else {
                    ss << left_pad(to_string_1f(record.time * (f64)100 / total_time), 9);
                }
            }
            // line_content
            ss << "  " << decl->code->src->get_line(line) << "\n";
        }
        ss << "\n";
    }
    return ss.str();
}

LineProfiler::~LineProfiler() {
    for(auto& p: records) delete p.second;
}

}  // namespace pkpy

#endif