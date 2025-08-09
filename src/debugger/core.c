

#include "pocketpy/interpreter/frame.h"
#include "pocketpy/pocketpy.h"
#include <ctype.h>

#include "pocketpy/debugger/core.h"

typedef struct c11_debugger_breakpoint {
    const char* sourcename;
    int lineno;
} c11_debugger_breakpoint;

typedef struct c11_debugger_scope_index {
    int locals_ref;
    int globals_ref;
} c11_debugger_scope_index;

#define SMALLMAP_T__HEADER
#define SMALLMAP_T__SOURCE
#define K int
#define V c11_debugger_scope_index
#define NAME c11_smallmap_d2index
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE
#undef SMALLMAP_T__HEADER

static struct c11_debugger {
    py_Frame* current_frame;
    const char* current_filename;
    enum py_TraceEvent current_event;

    int curr_stack_depth;
    int current_line;
    int pause_allowed_depth;
    int step_line;
    enum C11_STEP_MODE step_mode;
    bool keep_suspend;

    c11_vector breakpoints;
    c11_vector py_frames;
    c11_smallmap_d2index scopes_query_cache;

    #define python_vars py_r7()

} debugger;

inline static void init_structures() {
    c11_vector__ctor(&debugger.breakpoints, sizeof(c11_debugger_breakpoint));
    c11_vector__ctor(&debugger.py_frames, sizeof(py_Frame*));
    c11_smallmap_d2index__ctor(&debugger.scopes_query_cache);
    py_newlist(python_vars);
    py_newnil(py_list_emplace(python_vars));
}

inline static void clear_structures() {
    c11_vector__clear(&debugger.py_frames);
    c11_smallmap_d2index__clear(&debugger.scopes_query_cache);
    py_list_clear(python_vars);
    py_newnone(py_list_emplace(python_vars));
}

inline static py_Ref get_variable(int var_ref) {
    assert(var_ref < py_list_len(python_vars) && var_ref > 0);
    return py_list_getitem(python_vars, var_ref);
}

void c11_debugger_init() {
    debugger.curr_stack_depth = 0;
    debugger.current_line = -1;
    debugger.pause_allowed_depth = -1;
    debugger.step_line = -1;
    debugger.keep_suspend = false;
    debugger.step_mode = C11_STEP_CONTINUE;
    init_structures();
}

void c11_debugger_on_trace(py_Frame* frame, enum py_TraceEvent event) {
    debugger.current_frame = frame;
    debugger.current_event = event;
    debugger.current_filename = py_Frame_sourceloc(debugger.current_frame, &debugger.current_line);
    clear_structures();
    if(event == TRACE_EVENT_LINE) return;
    event == TRACE_EVENT_PUSH ? debugger.curr_stack_depth++ : debugger.curr_stack_depth--;
}

void c11_debugger_set_step_mode(enum C11_STEP_MODE mode) {
    switch(mode) {
        case C11_STEP_IN: debugger.pause_allowed_depth = INT32_MAX; break;
        case C11_STEP_OVER:
            debugger.pause_allowed_depth = debugger.curr_stack_depth;
            debugger.step_line = debugger.current_line;
            break;
        case C11_STEP_OUT: debugger.pause_allowed_depth = debugger.curr_stack_depth - 1; break;
        case C11_STEP_CONTINUE: debugger.pause_allowed_depth = -1; break;
    }
    debugger.step_mode = mode;
    debugger.keep_suspend = false;
}


int c11_debugger_setbreakpoint(const char* filename, int lineno) {
    c11_debugger_breakpoint breakpoint = {.sourcename = c11_strdup(filename), .lineno = lineno};
    c11_vector__push(c11_debugger_breakpoint, &debugger.breakpoints, breakpoint);
    return debugger.breakpoints.length;
}

int c11_debugger_reset_breakpoints_by_source(const char* sourcesname) {
    c11_vector tmp_breakpoints;
    c11_vector__ctor(&tmp_breakpoints, sizeof(c11_debugger_breakpoint));

    c11__foreach(c11_debugger_breakpoint, &debugger.breakpoints, it) {
        if(strcmp(it->sourcename, sourcesname) != 0) {
            c11_debugger_breakpoint* dst =
                (c11_debugger_breakpoint*)c11_vector__emplace(&tmp_breakpoints);
            *dst = *it;
        } else {
            free((void*)it->sourcename);
        }
    }

    c11_vector__swap(&tmp_breakpoints, &debugger.breakpoints);
    c11_vector__dtor(&tmp_breakpoints);
    return debugger.breakpoints.length;
}

int c11_debugger_should_pause() {
    if(debugger.current_event == TRACE_EVENT_POP) return false;
    bool should_pause = false;
    int is_out = debugger.curr_stack_depth <= debugger.pause_allowed_depth;
    int is_new_line = debugger.current_line != debugger.step_line;
    switch(debugger.step_mode) {
        case C11_STEP_IN: should_pause = true; break;

        case C11_STEP_OVER:
            if(is_new_line && is_out) should_pause = true;
            break;
        case C11_STEP_OUT:
            if(is_out) should_pause = true;
            break;
        case C11_STEP_CONTINUE:
        default: break;
    }
    if(debugger.step_mode == C11_STEP_CONTINUE) {
        c11__foreach(c11_debugger_breakpoint, &debugger.breakpoints, bp) {
            if(strcmp(debugger.current_filename, bp->sourcename) == 0 &&
               debugger.current_line == bp->lineno) {
                should_pause = true;
                break;
            }
        }
    }
    if(should_pause) { debugger.keep_suspend = true; }
    return should_pause;
}

int c11_debugger_should_keep_pause(void) { return debugger.keep_suspend; }


inline static c11_sv sv_from_cstr(const char* str) {
    c11_sv sv = {.data = str, .size = strlen(str)};
    return sv;
}

const inline static char* get_basename(const char* path) {
    const char* last_slash = strrchr(path, '/');
#ifdef _WIN32
    const char* last_backslash = strrchr(path, '\\');
    if(!last_slash || (last_backslash && last_backslash > last_slash)) {
        last_slash = last_backslash;
    }
#endif
    return last_slash ? last_slash + 1 : path;
}

void c11_debugger_frames(c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer, "{\"stackFrames\": [");
    int idx = 0;
    py_Frame* now_frame = debugger.current_frame;
    debugger.py_frames.length = 0;
    while(now_frame) {
        if(idx > 0) c11_sbuf__write_char(buffer, ',');
        int line;
        const char* filename = py_Frame_sourceloc(now_frame, &line);
        const char* basename = get_basename(filename);
        const char* modname = now_frame->co->name->data;
        pk_sprintf(
            buffer,
            "{\"id\": %d, \"name\": %Q, \"line\": %d, \"column\": 1, \"source\": {\"name\": %Q, \"path\": %Q}}",
            idx,
            sv_from_cstr(modname),
            line,
            sv_from_cstr(basename),
            sv_from_cstr(filename));
        c11_vector__push(py_Frame*, &debugger.py_frames, now_frame);
        now_frame = now_frame->f_back;
        idx++;
    }
    pk_sprintf(buffer, "], \"totalFrames\": %d}", idx);
}

inline static c11_debugger_scope_index append_new_scope(int frameid) {
    assert(frameid < debugger.py_frames.length);
    py_Frame* requested_frame = c11__getitem(py_Frame*, &debugger.py_frames, frameid);
    int base_index = py_list_len(python_vars);
    py_Ref new_locals = py_list_emplace(python_vars);
    py_Ref new_globals = py_list_emplace(python_vars);
    py_Frame_newlocals(requested_frame, new_locals);
    py_Frame_newglobals(requested_frame, new_globals);
    c11_debugger_scope_index result = {.locals_ref = base_index, .globals_ref = base_index + 1};
    return result;
}

void c11_debugger_scopes(int frameid, c11_sbuf* buffer) {
    // query cache
    c11_debugger_scope_index* result =
        c11_smallmap_d2index__try_get(&debugger.scopes_query_cache, frameid);

    c11_sbuf__write_cstr(buffer, "{\"scopes\":");
    const char* scopes_fmt =
        "[{\"name\": \"locals\", \"variablesReference\": %d, \"expensive\": false}, "
        "{\"name\": \"globals\", \"variablesReference\": %d, \"expensive\": true}]";
    if(result != NULL) {
        pk_sprintf(buffer, scopes_fmt, result->locals_ref, result->globals_ref);
    } else {
        c11_debugger_scope_index new_record = append_new_scope(frameid);
        c11_smallmap_d2index__set(&debugger.scopes_query_cache, frameid, new_record);
        pk_sprintf(buffer, scopes_fmt, new_record.locals_ref, new_record.globals_ref);
    }
    c11_sbuf__write_char(buffer, '}');
}

bool c11_debugger_unfold_var(int var_id, c11_sbuf* buffer) {
    py_Ref var = get_variable(var_id);
    if(!var) return false;

    // 1. extend
    const char* expand_code = NULL;
    switch(py_typeof(var)) {
        case tp_dict:
        case tp_namedict: expand_code = "[(k,v) for k,v in _0.items()]"; break;
        case tp_list:
        case tp_tuple: expand_code = "[(f'[{i}]',v) for i,v in enumerate(_0)]"; break;
        default: expand_code = "[(k,v) for k,v in _0.__dict__.items()]"; break;
    }
    if(!py_smarteval(expand_code, NULL, var)) {
        py_printexc();
        return false;
    }
    py_Ref kv_list = py_pushtmp();
    py_assign(kv_list, py_retval());
    // 2. prepare base_ref
    int base_index = py_list_len(python_vars);
    py_Ref base_var_ref = py_pushtmp();
    py_newint(base_var_ref, base_index);

    // 3. construct DAP JSON
    const char* dap_code =
        "{'variables': ["
        "  {"
        "    'name': kv[0],"
        "    'value': repr(kv[1]),"
        "    'variablesReference': (_1 + i) if (isinstance(kv[1], (dict, list, tuple)) or kv[1].__dict__ is not None) else 0,"
        "    'type': type(kv[1]).__name__"
        "  }"
        "  for i, kv in enumerate(_0)"
        "]}";
    if(!py_smarteval(dap_code, NULL, kv_list, base_var_ref)) {
        py_printexc();
        return false;
    }
    py_Ref dap_obj = py_pushtmp();
    py_assign(dap_obj, py_retval());

    // 4. extend python_vars
    if(!py_smartexec("_0.extend([kv[1] for kv in _1])", NULL, python_vars, kv_list)) {
        py_printexc();
        return false;
    }
    // 5. dump & write
    if(!py_json_dumps(dap_obj, 0)) {
        py_printexc();
        return false;
    }

    c11_sbuf__write_cstr(buffer, py_tostr(py_retval()));

    // 6. clear
    py_pop();  // dap_obj
    py_pop();  // base_var_ref
    py_pop();  // kv_list
    return true;
}
#undef python_vars
