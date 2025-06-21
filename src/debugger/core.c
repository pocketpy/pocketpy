#include "pocketpy/interpreter/frame.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/debugger/core.h"

enum C11_STEP_MODE { C11_STEP_IN, C11_STEP_OVER, C11_STEP_OUT, C11_STEP_CONTINUE };

typedef struct c11_debugger_evalpath {
    c11_string* path;
    py_Type type;
    int frameid;
    int isinlocals;
} c11_debugger_evalpath;

typedef struct c11_debugger_breakpoint {
    const char* source_name;
    int lineno;
} c11_debugger_breakpoint;

typedef struct c11_debugger_scope_index {
    int locals_ref;
    int globals_ref;
} c11_debuggeer_scope_index;

#define SMALLMAP_T__HEADER
#define K int
#define V c11_debuggeer_scope_index
#define NAME c11_smallmap_d2index
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

static struct c11_debugger {
    c11_vector breakpoints;
    py_Frame* current_frame;
    enum py_TraceEvent current_event;
    const char* current_filename;
    int curr_stack_depth;
    int current_line;
    int pause_allowed_depth;
    int step_line;
    bool should_keep_suspend;
    enum C11_STEP_MODE step_mode;
    c11_vector var_evals;
    c11_vector frames;
    c11_vector py_frames;
    c11_vector varibales;
    c11_vector py_variables;
    c11_smallmap_d2index scopes_query_cache;
    int next_var_id;
} debugger;

void c11_debugger_init() {
    debugger.curr_stack_depth = 0;
    debugger.next_var_id = 0;
    debugger.current_line = -1;
    debugger.pause_allowed_depth = -1;
    debugger.step_line = -1;
    debugger.should_keep_suspend = false;
    debugger.step_mode = C11_STEP_CONTINUE;
    c11_vector__ctor(&debugger.breakpoints, sizeof(c11_debugger_breakpoint));
    c11_vector__ctor(&debugger.var_evals, sizeof(c11_debugger_evalpath));
    c11_vector__ctor(&debugger.py_frames, sizeof(py_Frame*));
    c11_vector__ctor(&debugger.frames, sizeof(c11_debugger_frame));
    c11_vector__ctor(&debugger.varibales, sizeof(c11_debugger_variable));
    c11_smallmap_d2index__ctor(&debugger.scopes_query_cache);
}

void c11_debugger_on_trace(py_Frame* frame, enum py_TraceEvent event) {
    debugger.current_frame = frame;
    debugger.current_event = event;
    c11_vector__clear(&debugger.var_evals);
    c11_vector__clear(&debugger.frames);
    c11_vector__clear(&debugger.py_frames);
    debugger.current_filename = py_Frame_sourceloc(debugger.current_frame, &debugger.current_line);
    if(event == TRACE_EVENT_PUSH)
        debugger.curr_stack_depth++;
    else if(event == TRACE_EVENT_POP)
        debugger.curr_stack_depth--;
}

void c11_debugger_stepin() {
    debugger.pause_allowed_depth = INT32_MAX;
    debugger.step_mode = C11_STEP_IN;
    debugger.should_keep_suspend = false;
}

void c11_debugger_stepover() {
    debugger.pause_allowed_depth = debugger.curr_stack_depth;
    debugger.step_line = debugger.current_line;
    debugger.step_mode = C11_STEP_OVER;
    debugger.should_keep_suspend = false;
}

void c11_debugger_stepout() {
    debugger.pause_allowed_depth = debugger.curr_stack_depth - 1;
    debugger.step_mode = C11_STEP_OUT;
    debugger.should_keep_suspend = false;
}

void c11_debugger_continue() {
    debugger.pause_allowed_depth = -1;
    debugger.step_mode = C11_STEP_CONTINUE;
    debugger.should_keep_suspend = false;
}

void c11_debugger_setbreakpoint(const char* filename, int lineno) {
    c11_debugger_breakpoint breakpoint = {.source_name = c11_strdup(filename), .lineno = lineno};
    c11_vector__push(c11_debugger_breakpoint, &debugger.breakpoints, breakpoint);
}

int c11_debugger_should_pause() {
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

    c11__foreach(c11_debugger_breakpoint, &debugger.breakpoints, bp) {
        if(strcmp(debugger.current_filename, bp->source_name) == 0 &&
           debugger.current_line == bp->lineno) {
            should_pause = true;
            break;
        }
    }
    if(should_pause) { debugger.should_keep_suspend = true; }
    return should_pause;
}

int c11_debugger_should_keep_pause(void) { return debugger.should_keep_suspend; }

c11_vector c11_debugger_frames() {
    if(debugger.frames.length != 0) { return debugger.frames; }
    int idx = 0;
    int line;
    const char* filename;
    py_Frame* now_frame = debugger.current_frame;
    while(now_frame) {
        c11_debugger_frame frame;
        filename = py_Frame_sourceloc(now_frame, &line);
        frame.file_name = filename;
        frame.lineno = line;
        frame.id = idx++;
        frame.module_name = now_frame->co->name->data;
        if(strcmp(frame.module_name, frame.file_name) == 0) { frame.module_name = "<pkpy_main>"; }
        c11_vector__push(c11_debugger_frame, &debugger.frames, frame);
        c11_vector__push(py_Frame*, &debugger.py_frames, now_frame);
        now_frame = now_frame->f_back;
    }
    return debugger.frames;
}

static void c11_debugger_new_newevalpath(int scope_id, c11_debugger_variable* var) {
    c11_debugger_evalpath current =
        c11__getitem(c11_debugger_evalpath, &debugger.var_evals, scope_id);
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    switch(current.type) {
        case tp_list: pk_sprintf(&ss, "%s[%s]", current.path->data, var->name); break;
        default: pk_sprintf(&ss, "%s['%s']", current.path->data, var->name); break;
    }
    c11_debugger_evalpath newevalpath = {.path = c11_sbuf__submit(&ss),
                                         .type = var->type,
                                         .isinlocals = current.isinlocals,
                                         .frameid = current.frameid};
    c11_vector__push(c11_debugger_evalpath, &debugger.var_evals, newevalpath);
    // printf("add\n");
}

static c11_string* c11_debugger_fullevalpath(int var_id, int* isinlocals, int* frameid) {
    c11_debugger_evalpath var_evalpath =
        c11__getitem(c11_debugger_evalpath, &debugger.var_evals, var_id);
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    const char* pythonsnippets;
    switch(var_evalpath.type) {
        case tp_dict:
            pythonsnippets = "[(k,repr(v),v) for k,v in %s.items()]";
            pk_sprintf(&ss, pythonsnippets, var_evalpath.path->data);
            break;
        case tp_list: {
            const char* fmt = "[(repr(i),repr(%s[i]),%s[i]) for i in range(len(%s))]";
            pk_sprintf(&ss,
                       fmt,
                       var_evalpath.path->data,
                       var_evalpath.path->data,
                       var_evalpath.path->data);
            break;
        }
        default: break;
    }
    *isinlocals = var_evalpath.isinlocals;
    *frameid = var_evalpath.frameid;
    return c11_sbuf__submit(&ss);
}

inline static void make_scope(c11_debugger_variable* scope, const char* name, int var_ref) {
    scope->var_ref = var_ref;
    scope->name = name;
    scope->value = NULL;
    scope->type = NULL;
}

inline static void save_frame_scopes(int frameid){
    py_Ref ref_loclas = c11_vector__emplace(c11_vector *self)
}

c11_vector c11_debugger_scopes(int frameid) {
    c11_vector scopes;
    c11_vector__ctor(&scopes, sizeof(c11_debugger_variable));
    c11_debugger_variable* it_locals = c11_vector__emplace(&scopes);
    c11_debugger_variable* it_globals = c11_vector__emplace(&scopes);
    // query cache
    c11_debuggeer_scope_index* result =
        c11_smallmap_d2index__try_get(&debugger.scopes_query_cache, frameid);
    if(result != NULL) {
        *it_locals = *c11__at(c11_debugger_variable, &debugger.varibales, result->locals_ref);
        *it_globals = *c11__at(c11_debugger_variable, &debugger.varibales, result->globals_ref);
        return scopes;
    }
    make_scope(it_locals, "locals", result->locals_ref);
    make_scope(it_globals, "globals", result->globals_ref);
    c11_vector__extend(c11_debugger_variable, &debugger.varibales, scopes.data, 2);
    return scopes;
}

c11_vector c11_debugger_get_children(int var_id) {
    c11_vector variables;
    c11_vector__ctor(&variables, sizeof(c11_debugger_variable));

    int isinlocals;
    int frameid;
    c11_string* evalpath = c11_debugger_fullevalpath(var_id, &isinlocals, &frameid);
    py_Ref scope = py_pushtmp();
    py_Ref vars = py_pushtmp();
    py_Frame* active_frame = c11__getitem(py_Frame*, &debugger.py_frames, frameid);
    isinlocals ? py_Frame_newlocals(active_frame, scope) : py_Frame_newglobals(active_frame, scope);

    py_smarteval(evalpath->data, NULL, scope);
    py_assign(vars, py_retval());

    int len = py_list_len(vars);
    for(int i = 0; i < len; i++) {
        py_Ref var = py_pushtmp();
        var = py_list_getitem(vars, i);

        py_Ref py_name = py_tuple_getitem(var, 0);
        py_Ref py_value = py_tuple_getitem(var, 1);
        py_Ref py_real = py_tuple_getitem(var, 2);
        py_Type type = py_typeof(py_real);
        bool is_composite = type == tp_dict || type == tp_list;
        c11_debugger_variable variable = {
            .name = py_tostr(py_name),
            .value = py_tostr(py_value),
            .type = type,
            .is_composite = is_composite,
            .id = debugger.next_var_id++,
        };
        c11_vector__push(c11_debugger_variable, &variables, variable);
        py_pop();
    }

    if(!c11_vector__contains(&debugger.var_history_querys, &var_id)) {
        c11__foreach(c11_debugger_variable, &variables, it) {
            c11_debugger_new_newevalpath(var_id, it);
        }
        c11_vector__push(int, &debugger.var_history_querys, var_id);
    }

    py_pop();
    py_pop();
    c11_string__delete(evalpath);
    return variables;
}
