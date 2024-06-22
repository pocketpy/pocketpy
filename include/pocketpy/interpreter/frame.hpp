#pragma once

#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/codeobject.hpp"

namespace pkpy {

// weak reference fast locals
struct FastLocals {
    // this is a weak reference
    const CodeObject* co;
    PyVar* a;

    int size() const { return co->nlocals; }

    PyVar& operator[] (int i) { return a[i]; }

    PyVar operator[] (int i) const { return a[i]; }

    FastLocals(const CodeObject* co, PyVar* a) : co(co), a(a) {}

    PyVar* try_get_name(StrName name);
    NameDict* to_namedict();

    PyVar* begin() const { return a; }

    PyVar* end() const { return a + size(); }
};

struct ValueStack {
    PK_ALWAYS_PASS_BY_POINTER(ValueStack)

    // We allocate extra PK_VM_STACK_SIZE/128 places to keep `_sp` valid when `is_overflow() == true`.
    PyVar _begin[PK_VM_STACK_SIZE + PK_VM_STACK_SIZE / 128];
    PyVar* _sp;
    PyVar* _max_end;

    constexpr static size_t max_size() { return PK_VM_STACK_SIZE; }

    ValueStack() : _sp(_begin), _max_end(_begin + PK_VM_STACK_SIZE) {}

    PyVar& top() { return _sp[-1]; }

    PyVar top() const { return _sp[-1]; }

    PyVar& second() { return _sp[-2]; }

    PyVar second() const { return _sp[-2]; }

    PyVar& third() { return _sp[-3]; }

    PyVar third() const { return _sp[-3]; }

    PyVar& peek(int n) { return _sp[-n]; }

    PyVar peek(int n) const { return _sp[-n]; }

    void push(PyVar v) { *_sp++ = v; }

    void push(std::nullptr_t) { std::memset(_sp++, 0, sizeof(PyVar)); }

    void pop() { --_sp; }

    PyVar popx() {
        --_sp;
        return *_sp;
    }

    ArgsView view(int n) { return ArgsView(_sp - n, _sp); }

    void shrink(int n) { _sp -= n; }

    int size() const { return _sp - _begin; }

    bool empty() const { return _sp == _begin; }

    PyVar* begin() { return _begin; }

    PyVar* end() { return _sp; }

    void reset(PyVar* sp) { _sp = sp; }

    void clear() { _sp = _begin; }

    bool is_overflow() const { return _sp >= _max_end; }

    template <typename... Args>
    void emplace(Args&&... args) {
        new (_sp) PyVar(std::forward<Args>(args)...);
        ++_sp;
    }
};

struct UnwindTarget {
    UnwindTarget* next;
    int iblock;
    int offset;

    UnwindTarget(int iblock, int offset) : next(nullptr), iblock(iblock), offset(offset) {}
};

struct Frame {
    PK_ALWAYS_PASS_BY_POINTER(Frame)

    const Bytecode* _ip;
    // This is for unwinding only, use `actual_sp_base()` for value stack access
    PyVar* _sp_base;

    const CodeObject* co;
    PyObject* _module;
    PyObject* _callable;  // a function object or nullptr (global scope)
    FastLocals _locals;

    // This list will be freed in __pop_frame
    UnwindTarget* _uw_list;

    NameDict& f_globals() { return _module->attr(); }

    PyVar* f_closure_try_get(StrName name);

    int ip() const { return _ip - (Bytecode*)co->codes.data; }

    // function scope
    Frame(PyVar* p0, const CodeObject* co, PyObject* _module, PyObject* _callable, PyVar* _locals_base) :
        _ip((Bytecode*)co->codes.data - 1), _sp_base(p0), co(co), _module(_module), _callable(_callable),
        _locals(co, _locals_base), _uw_list(nullptr) {}

    // exec/eval
    Frame(PyVar* p0, const CodeObject* co, PyObject* _module, PyObject* _callable, FastLocals _locals) :
        _ip((Bytecode*)co->codes.data - 1), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(_locals),
        _uw_list(nullptr) {}

    // global scope
    Frame(PyVar* p0, const CodeObject* co, PyObject* _module) :
        _ip((Bytecode*)co->codes.data - 1), _sp_base(p0), co(co), _module(_module), _callable(nullptr),
        _locals(co, p0), _uw_list(nullptr) {}

    PyVar* actual_sp_base() const { return _locals.a; }

    ArgsView stack_view(ValueStack* _s) const { return ArgsView(actual_sp_base(), _s->_sp); }

    [[nodiscard]] int prepare_jump_exception_handler(ValueStack*);
    void prepare_jump_break(ValueStack*, int);
    int _exit_block(ValueStack*, int);

    [[nodiscard]] int prepare_loop_break(ValueStack* s_data) {
        int iblock = c11__getitem(BytecodeEx, &co->codes_ex, ip()).iblock;
        int target = c11__getitem(CodeBlock, &co->blocks, iblock).end;
        prepare_jump_break(s_data, target);
        return target;
    }

    int curr_lineno() const {
        return c11__getitem(BytecodeEx, &co->codes_ex, ip()).lineno;
    }

    void set_unwind_target(PyVar* _sp);
    UnwindTarget* find_unwind_target(int iblock);

    void _gc_mark(VM* vm) const;
    ~Frame();
};

struct LinkedFrame {
    LinkedFrame* f_back;
    Frame frame;

    template <typename... Args>
    LinkedFrame(LinkedFrame* f_back, Args&&... args) : f_back(f_back), frame(std::forward<Args>(args)...) {}
};

struct CallStack {
    static_assert(sizeof(LinkedFrame) <= 128);

    LinkedFrame* _tail;
    int _size;

    CallStack() : _tail(nullptr), _size(0) {}

    int size() const { return _size; }

    bool empty() const { return _size == 0; }

    void clear() {
        while(!empty())
            pop();
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        static_assert(sizeof(LinkedFrame) <= kPoolFrameBlockSize);
        _tail = new (PoolFrame_alloc()) LinkedFrame(_tail, std::forward<Args>(args)...);
        ++_size;
    }

    void pop();
    LinkedFrame* popx();
    void pushx(LinkedFrame* p);

    Frame& top() const {
        assert(!empty());
        return _tail->frame;
    }

    template <typename Func>
    void apply(Func&& f) {
        for(LinkedFrame* p = _tail; p != nullptr; p = p->f_back)
            f(p->frame);
    }

    ~CallStack() { clear(); }
};

};  // namespace pkpy
