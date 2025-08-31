#include "pocketpy/interpreter/frame.h"
#include "pocketpy/objects/exception.h"
#include "pocketpy/pocketpy.h"
#include <ctype.h>

#if PK_ENABLE_OS

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
    const char* current_excname;
    const char* current_excmessage;
    enum py_TraceEvent current_event;

    int curr_stack_depth;
    int current_line;
    int pause_allowed_depth;
    int step_line;
    C11_STEP_MODE step_mode;
    bool keep_suspend;
    bool isexceptionmode;

    c11_vector* exception_stacktrace;
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
    py_newnone(py_list_emplace(python_vars));
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

const inline static char* format_filepath(const char* path) {
    if(strstr(path, "..")) { return NULL; }
    if(strstr(path + 1, "./") || strstr(path + 1, ".\\")) { return NULL; }
    if(path[0] == '.' && (path[1] == '/' || path[1] == '\\')) { return path + 2; }
    return path;
}

void c11_debugger_init() {
    debugger.curr_stack_depth = 0;
    debugger.current_line = -1;
    debugger.pause_allowed_depth = -1;
    debugger.step_line = -1;
    debugger.keep_suspend = false;
    debugger.isexceptionmode = false;
    debugger.step_mode = C11_STEP_CONTINUE;
    init_structures();
}

C11_DEBUGGER_STATUS c11_debugger_on_trace(py_Frame* frame, enum py_TraceEvent event) {
    debugger.current_frame = frame;
    debugger.current_event = event;
    const char* source_name = py_Frame_sourceloc(debugger.current_frame, &debugger.current_line);
    debugger.current_filename = format_filepath(source_name);
    if(debugger.current_filename == NULL) { return C11_DEBUGGER_FILEPATH_ERROR; }
    clear_structures();
    switch(event) {
        case TRACE_EVENT_PUSH: debugger.curr_stack_depth++; break;
        case TRACE_EVENT_POP: debugger.curr_stack_depth--; break;
        default: break;
    }
    // if(debugger.curr_stack_depth == 0) return C11_DEBUGGER_EXIT;
    return C11_DEBUGGER_SUCCESS;
}

void c11_debugger_exception_on_trace(py_Ref exc) {
    BaseException* ud = py_touserdata(exc);
    c11_vector* stacktrace = &ud->stacktrace;
    const char* name = py_tpname(exc->type);
    const char* message = safe_stringify_exception(exc);
    debugger.exception_stacktrace = stacktrace;
    debugger.isexceptionmode = true;
    debugger.current_excname = name;
    debugger.current_excmessage = message;
    clear_structures();
    py_assign(py_list_getitem(python_vars, 0), exc);
    py_clearexc(NULL);
}

const char* c11_debugger_excinfo(const char** message) {
    *message = debugger.current_excmessage;
    return debugger.current_excname;
}

void c11_debugger_set_step_mode(C11_STEP_MODE mode) {
    switch(mode) {
        case C11_STEP_IN: debugger.pause_allowed_depth = INT32_MAX; break;
        case C11_STEP_OVER:
            debugger.pause_allowed_depth = debugger.curr_stack_depth;
            debugger.step_line = debugger.current_line;
            break;
        case C11_STEP_OUT: debugger.pause_allowed_depth = debugger.curr_stack_depth - 1; break;
        case C11_STEP_CONTINUE: debugger.pause_allowed_depth = -1; break;
        default: break;
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
            PK_FREE((void*)it->sourcename);
        }
    }

    c11_vector__swap(&tmp_breakpoints, &debugger.breakpoints);
    c11_vector__dtor(&tmp_breakpoints);
    return debugger.breakpoints.length;
}

bool c11_debugger_path_equal(const char* path1, const char* path2) {
    if(path1 == NULL || path2 == NULL) return false;
    while(*path1 && *path2) {
        char c1 = (*path1 == '\\') ? '/' : *path1;
        char c2 = (*path2 == '\\') ? '/' : *path2;
        c1 = (char)tolower((unsigned char)c1);
        c2 = (char)tolower((unsigned char)c2);
        if(c1 != c2) return false;
        path1++;
        path2++;
    }
    return *path1 == *path2;
}

C11_STOP_REASON c11_debugger_should_pause() {
    if(debugger.current_event == TRACE_EVENT_POP && !debugger.isexceptionmode)
        return C11_DEBUGGER_NOSTOP;
    if(py_checkexc() && debugger.isexceptionmode == false)
        return C11_DEBUGGER_NOSTOP;
    C11_STOP_REASON pause_resaon = C11_DEBUGGER_NOSTOP;
    int is_out = debugger.curr_stack_depth <= debugger.pause_allowed_depth;
    int is_new_line = debugger.current_line != debugger.step_line;
    switch(debugger.step_mode) {
        case C11_STEP_IN: pause_resaon = C11_DEBUGGER_STEP; break;
        case C11_STEP_OVER:
            if(is_new_line && is_out) pause_resaon = C11_DEBUGGER_STEP;
            break;
        case C11_STEP_OUT:
            if(is_out) pause_resaon = C11_DEBUGGER_STEP;
            break;
        case C11_STEP_CONTINUE:
        default: break;
    }
    if(debugger.step_mode == C11_STEP_CONTINUE) {
        c11__foreach(c11_debugger_breakpoint, &debugger.breakpoints, bp) {
            if(c11_debugger_path_equal(debugger.current_filename, bp->sourcename) &&
               debugger.current_line == bp->lineno) {
                pause_resaon = C11_DEBUGGER_BP;
                break;
            }
        }
    }
    if(debugger.isexceptionmode) pause_resaon = C11_DEBUGGER_EXCEPTION;
    if(pause_resaon != C11_DEBUGGER_NOSTOP) { debugger.keep_suspend = true; }
    return pause_resaon;
}

int c11_debugger_should_keep_pause(void) { return debugger.keep_suspend; }

inline static c11_sv sv_from_cstr(const char* str) {
    c11_sv sv = {.data = str, .size = strlen(str)};
    return sv;
}

const inline static char* get_basename(const char* path) {
    const char* last_slash = strrchr(path, '/');
#if defined(_WIN32) || defined(_WIN64)
    const char* last_backslash = strrchr(path, '\\');
    if(!last_slash || (last_backslash && last_backslash > last_slash)) {
        last_slash = last_backslash;
    }
#endif
    return last_slash ? last_slash + 1 : path;
}

void c11_debugger_normal_frames(c11_sbuf* buffer) {
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

void c11_debugger_exception_frames(c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer, "{\"stackFrames\": [");
    int idx = 0;
    c11__foreach(BaseExceptionFrame, debugger.exception_stacktrace, it) {
        if(idx > 0) c11_sbuf__write_char(buffer, ',');
        int line = it->lineno;
        const char* filename = it->src->filename->data;
        const char* basename = get_basename(filename);
        const char* modname = it->name == NULL ? basename : it->name->data;
        pk_sprintf(
            buffer,
            "{\"id\": %d, \"name\": %Q, \"line\": %d, \"column\": 1, \"source\": {\"name\": %Q, \"path\": %Q}}",
            idx,
            sv_from_cstr(modname),
            line,
            sv_from_cstr(basename),
            sv_from_cstr(filename));
        idx++;
    }
    pk_sprintf(buffer, "], \"totalFrames\": %d}", idx);
}

void c11_debugger_frames(c11_sbuf* buffer) {
    debugger.isexceptionmode ? c11_debugger_exception_frames(buffer)
                             : c11_debugger_normal_frames(buffer);
}

inline static c11_debugger_scope_index append_new_scope(int frameid) {
    assert(frameid < debugger.py_frames.length);
    py_Frame* requested_frame = c11__getitem(py_Frame*, &debugger.py_frames, frameid);
    int base_index = py_list_len(python_vars);
    py_Ref new_locals = py_list_emplace(python_vars);
    py_Frame_newlocals(requested_frame, new_locals);
    py_Ref new_globals = py_list_emplace(python_vars);
    py_Frame_newglobals(requested_frame, new_globals);
    c11_debugger_scope_index result = {.locals_ref = base_index, .globals_ref = base_index + 1};
    return result;
}

inline static c11_debugger_scope_index append_new_exception_scope(int frameid) {
    assert(frameid < debugger.exception_stacktrace->length);
    BaseExceptionFrame* requested_frame =
        c11__at(BaseExceptionFrame, debugger.exception_stacktrace, frameid);
    int base_index = py_list_len(python_vars);
    py_list_append(python_vars, &requested_frame->locals);
    py_list_append(python_vars, &requested_frame->globals);
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
        c11_debugger_scope_index new_record = debugger.isexceptionmode
                                                  ? append_new_exception_scope(frameid)
                                                  : append_new_scope(frameid);
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

    // 3. construct DAP JSON and extend python_vars
    py_Ref dap_obj = py_pushtmp();
    py_newdict(dap_obj);
    const char* dap_code =
        "_2['variables'] = []\n"
        "var_ref = _1\n"
        "for k, v in _0:\n"
        "    has_children = isinstance(v, (dict, list, tuple)) or v.__dict__ is not None\n"
        "    _2['variables'].append({\n"
        "        'name': k if type(k) == str else str(k),\n"
        "        'value': repr(v) if type(v) == str else str(v),\n"
        "        'variablesReference': var_ref if has_children else 0,\n"
        "        'type': type(v).__name__\n"
        "    })\n"
        "    if has_children: var_ref += 1\n";
    if(!py_smartexec(dap_code, NULL, kv_list, base_var_ref, dap_obj)) {
        py_printexc();
        return false;
    }

    // 4. extend python_vars
    if(!py_smartexec(
           "_0.extend([v for k, v in _1 if isinstance(v, (dict, list, tuple)) or v.__dict__ is not None])",
           NULL,
           python_vars,
           kv_list)) {
        py_printexc();
        return false;
    }

    // 5. dump & write
    if(!py_json_dumps(dap_obj, 0)) {
        // printf("dap_obj: %s\n", py_tpname(py_typeof(dap_obj)));
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

#endif  // PK_ENABLE_OS
