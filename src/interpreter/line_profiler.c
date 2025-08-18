#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/line_profiler.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"
#include <assert.h>

void LineProfiler__ctor(LineProfiler* self) {
    c11_smallmap_p2i__ctor(&self->records);
    c11_vector__ctor(&self->frame_records, sizeof(FrameRecord));
    self->enabled = false;
}

void LineProfiler__dtor(LineProfiler* self) {
    for(int i = 0; i < self->records.length; i++) {
        c11_smallmap_p2i_KV kv = c11__getitem(c11_smallmap_p2i_KV, &self->records, i);
        SourceData_ src = (SourceData_)kv.key;
        PK_DECREF(src);
        PK_FREE((void*)kv.value);
    }
    c11_smallmap_p2i__dtor(&self->records);
    c11_vector__dtor(&self->frame_records);
}

LineRecord* LineProfiler__get_record(LineProfiler* self, SourceLocation loc) {
    LineRecord* lines = (LineRecord*)c11_smallmap_p2i__get(&self->records, loc.src, 0);
    if(lines == NULL) {
        int max_lineno = loc.src->line_starts.length;
        lines = PK_MALLOC(sizeof(LineRecord) * (max_lineno + 1));
        memset(lines, 0, sizeof(LineRecord) * (max_lineno + 1));
        c11_smallmap_p2i__set(&self->records, loc.src, (py_i64)lines);
        PK_INCREF(loc.src);
    }
    return &lines[loc.lineno];
}

void LineProfiler__begin(LineProfiler* self) {
    assert(!self->enabled);
    self->enabled = true;
}

static void LineProfiler__increment_now(LineProfiler* self, clock_t now, LineRecord* curr_line) {
    FrameRecord* top_frame_record = &c11_vector__back(FrameRecord, &self->frame_records);
    if(!top_frame_record->is_lambda) {
        LineRecord* prev_line = top_frame_record->prev_line;
        clock_t delta = now - top_frame_record->prev_time;
        top_frame_record->prev_time = now;
        prev_line->hits++;
        prev_line->time += delta;
        // printf("  ==> increment_now: delta: %ld, hits: %lld\n", delta, prev_line->hits);
    }
    top_frame_record->prev_line = curr_line;
}

void LineProfiler__tracefunc_internal(LineProfiler* self,
                                      py_Frame* frame,
                                      enum py_TraceEvent event) {
    assert(self->enabled);
    clock_t now = clock();

    // SourceLocation curr_loc = Frame__source_location(frame);
    // printf("==> frame: %p:%d, event: %d, now: %ld\n", frame, curr_loc.lineno, event, now);

    SourceLocation curr_loc = Frame__source_location(frame);
    LineRecord* curr_line = LineProfiler__get_record(self, curr_loc);

    if(event == TRACE_EVENT_LINE) {
        LineProfiler__increment_now(self, now, curr_line);
    } else {
        if(event == TRACE_EVENT_PUSH) {
            FrameRecord f_record = {.frame = frame,
                                    .prev_time = now,
                                    .prev_line = curr_line,
                                    .is_lambda = false};
            if(!frame->is_locals_special && py_istype(frame->p0, tp_function)) {
                Function* fn = py_touserdata(frame->p0);
                c11_string* fn_name = fn->decl->code.name;
                f_record.is_lambda = fn_name->size > 0 && fn_name->data[0] == '<';
            }
            c11_vector__push(FrameRecord, &self->frame_records, f_record);
        } else if(event == TRACE_EVENT_POP) {
            LineProfiler__increment_now(self, now, NULL);
            assert(self->frame_records.length > 0);
            c11_vector__pop(&self->frame_records);
        }
    }
}

void LineProfiler__end(LineProfiler* self) {
    assert(self->enabled);
    if(self->frame_records.length > 0) LineProfiler__increment_now(self, clock(), NULL);
    self->enabled = false;
}

void LineProfiler__reset(LineProfiler* self) {
    LineProfiler__dtor(self);
    LineProfiler__ctor(self);
}

c11_string* LineProfiler__get_report(LineProfiler* self) {
    c11_sbuf sbuf;
    c11_sbuf__ctor(&sbuf);
    c11_sbuf__write_char(&sbuf, '{');
    c11_sbuf__write_cstr(&sbuf, "\"CLOCKS_PER_SEC\": ");
    c11_sbuf__write_i64(&sbuf, CLOCKS_PER_SEC);
    c11_sbuf__write_cstr(&sbuf, ", \"records\": ");

    c11_sbuf__write_char(&sbuf, '{');
    for(int i = 0; i < self->records.length; i++) {
        c11_smallmap_p2i_KV kv = c11__getitem(c11_smallmap_p2i_KV, &self->records, i);
        SourceData_ src = (SourceData_)kv.key;
        int line_record_length = src->line_starts.length + 1;
        c11_sv src_name = c11_string__sv(src->filename);
        c11_sbuf__write_quoted(&sbuf, src_name, '"');
        c11_sbuf__write_cstr(&sbuf, ": [");
        LineRecord* lines = (LineRecord*)kv.value;
        bool is_first = true;
        for(int j = 1; j < line_record_length; j++) {
            // [<j>, <hits>, <time>]
            if(lines[j].hits == 0 && lines[j].time == 0) continue;
            if(!is_first) c11_sbuf__write_cstr(&sbuf, ", ");
            c11_sbuf__write_char(&sbuf, '[');
            c11_sbuf__write_int(&sbuf, j);
            c11_sbuf__write_cstr(&sbuf, ", ");
            c11_sbuf__write_i64(&sbuf, lines[j].hits);
            c11_sbuf__write_cstr(&sbuf, ", ");
            c11_sbuf__write_i64(&sbuf, lines[j].time);
            c11_sbuf__write_char(&sbuf, ']');
            is_first = false;
        }
        c11_sbuf__write_cstr(&sbuf, "]");
        if(i < self->records.length - 1) c11_sbuf__write_cstr(&sbuf, ", ");
    }
    c11_sbuf__write_char(&sbuf, '}');
    c11_sbuf__write_char(&sbuf, '}');
    return c11_sbuf__submit(&sbuf);
}
