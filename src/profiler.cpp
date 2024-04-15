#include "pocketpy/profiler.h"

namespace pkpy{

static std::string left_pad(std::string s, int width){
    int n = width - s.size();
    if(n <= 0) return s;
    return std::string(n, ' ') + s;
}

static std::string to_string_1f(f64 x){
    char buf[32];
    snprintf(buf, 32, "%.1f", x);
    return buf;
}

void LineProfiler::begin(){
    frames.clear();
}

void LineProfiler::_step(int callstack_size, Frame* frame){
    auto line_info = frame->co->lines[frame->_ip];
    if(line_info.is_virtual) return;
    std::string_view filename = frame->co->src->filename.sv();
    int line = line_info.lineno;

    if(frames.empty()){
        frames.push({callstack_size, frame, clock(), nullptr});
    }else{
        _step_end(callstack_size, frame, line);
    }

    auto& file_records = records[filename];
    if(file_records.empty()){
        // initialize file_records
        int total_lines = frame->co->src->line_starts.size();
        file_records.resize(total_lines + 1);
        for(int i=1; i<=total_lines; i++){
            file_records[i].line = i;
        }
    }

    frames.top().prev_record = &file_records.at(line);
}

void LineProfiler::_step_end(int callstack_size, Frame* frame, int line){
    clock_t now = clock();
    _FrameRecord& top_frame_record = frames.top();
    _LineRecord* prev_record = top_frame_record.prev_record;

    int id_delta = callstack_size - top_frame_record.callstack_size;
    PK_ASSERT(abs(id_delta) <= 1)

    // current line is about to change
    if(prev_record->line != line){
        clock_t delta = now - top_frame_record.prev_time;
        top_frame_record.prev_time = now;
        if(id_delta != 1) prev_record->hits++;
        prev_record->time += delta;
    }
    
    if(id_delta == 1){
        frames.push({callstack_size, frame, now, nullptr});
    }else{
        if(id_delta == -1) frames.pop();
    }
}

void LineProfiler::end(){
    clock_t now = clock();
    _FrameRecord& top_frame_record = frames.top();
    _LineRecord* prev_record = top_frame_record.prev_record;

    clock_t delta = now - top_frame_record.prev_time;
    top_frame_record.prev_time = now;
    prev_record->hits++;
    prev_record->time += delta;

    frames.pop();
    PK_ASSERT(frames.empty());
}

Str LineProfiler::stats(){
    SStream ss;
    for(FuncDecl* decl: functions){
        int start_line = decl->code->start_line;
        int end_line = decl->code->end_line;
        if(start_line == -1 || end_line == -1) continue;
        std::string_view filename = decl->code->src->filename.sv();
        std::vector<_LineRecord>& file_records = records[filename];
        if(file_records.empty()) continue;
        clock_t total_time = 0;
        for(int line = start_line; line <= end_line; line++){
            total_time += file_records.at(line).time;
        }
        ss << "Total time: " << (f64)total_time / CLOCKS_PER_SEC << "s\n";
        ss << "File: " << filename << "\n";
        ss << "Function: " << decl->code->name << " at line " << start_line << "\n";
        ss << "Line #      Hits         Time  Per Hit   % Time  Line Contents\n";
        ss << "==============================================================\n";
        for(int line = start_line; line <= end_line; line++){
            const _LineRecord& record = file_records.at(line);
            if(!record.is_valid()) continue;
            ss << left_pad(std::to_string(line), 6);
            if(record.hits == 0){
                ss << std::string(10 + 13 + 9 + 9, ' ');
            }else{
                ss << left_pad(std::to_string(record.hits), 10);
                ss << left_pad(std::to_string(record.time), 13);
                ss << left_pad(std::to_string(record.time / record.hits), 9);
                if(total_time == 0){
                    ss << left_pad("0.0", 9);
                }else{
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

}   // namespace pkpy