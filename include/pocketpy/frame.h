#pragma once

#include "codeobject.h"
#include "common.h"
#include "memory.h"
#include "obj.h"
#include "vector.h"

namespace pkpy{

// weak reference fast locals
struct FastLocals{
    // this is a weak reference
    const CodeObject* co;
    PyVar* a;

    int size() const{ return co->varnames.size();}

    PyVar& operator[](int i){ return a[i]; }
    PyVar operator[](int i) const { return a[i]; }

    FastLocals(const CodeObject* co, PyVar* a): co(co), a(a) {}

    PyVar* try_get_name(StrName name);
    NameDict_ to_namedict();

    PyVar* begin() const { return a; }
    PyVar* end() const { return a + size(); }
};

struct ValueStack {
    // We allocate extra PK_VM_STACK_SIZE/128 places to keep `_sp` valid when `is_overflow() == true`.
    PyVar _begin[PK_VM_STACK_SIZE + PK_VM_STACK_SIZE/128];
    PyVar* _sp;
    PyVar* _max_end;

    static constexpr size_t max_size() { return PK_VM_STACK_SIZE; }

    ValueStack(): _sp(_begin), _max_end(_begin + PK_VM_STACK_SIZE) {}

    PyVar& top(){ return _sp[-1]; }
    PyVar top() const { return _sp[-1]; }
    PyVar& second(){ return _sp[-2]; }
    PyVar second() const { return _sp[-2]; }
    PyVar& third(){ return _sp[-3]; }
    PyVar third() const { return _sp[-3]; }
    PyVar& peek(int n){ return _sp[-n]; }
    PyVar peek(int n) const { return _sp[-n]; }
    void push(PyVar v){ *_sp++ = v; }
    void pop(){ --_sp; }
    PyVar popx(){ return *--_sp; }
    ArgsView view(int n){ return ArgsView(_sp-n, _sp); }
    void shrink(int n){ _sp -= n; }
    int size() const { return _sp - _begin; }
    bool empty() const { return _sp == _begin; }
    PyVar* begin() { return _begin; }
    PyVar* end() { return _sp; }
    void reset(PyVar* sp) { _sp = sp; }
    void clear() { _sp = _begin; }
    bool is_overflow() const { return _sp >= _max_end; }

    PyVar operator[](int i) const { return _begin[i]; }
    PyVar& operator[](int i) { return _begin[i]; }
    
    ValueStack(const ValueStack&) = delete;
    ValueStack(ValueStack&&) = delete;
    ValueStack& operator=(const ValueStack&) = delete;
    ValueStack& operator=(ValueStack&&) = delete;
};

struct Frame {
    int _ip;
    int _next_ip;
    // This is for unwinding only, use `actual_sp_base()` for value stack access
    PyVar* _sp_base;

    const CodeObject* co;
    PyVar _module;
    PyVar _callable;    // a function object or nullptr (global scope)
    FastLocals _locals;

    NameDict& f_globals() { return _module->attr(); }
    PyVar f_closure_try_get(StrName name);

    // function scope
    Frame(PyVar* p0, const CodeObject* co, PyVar _module, PyVar _callable, PyVar* _locals_base)
            : _ip(-1), _next_ip(0), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(co, _locals_base) { }

    // exec/eval
    Frame(PyVar* p0, const CodeObject* co, PyVar _module, PyVar _callable, FastLocals _locals)
            : _ip(-1), _next_ip(0), _sp_base(p0), co(co), _module(_module), _callable(_callable), _locals(_locals) { }

    // global scope
    Frame(PyVar* p0, const CodeObject_& co, PyVar _module)
            : _ip(-1), _next_ip(0), _sp_base(p0), co(co.get()), _module(_module), _callable(nullptr), _locals(co.get(), p0) {}

    int next_bytecode() {
        _ip = _next_ip++;
        PK_DEBUG_ASSERT(_ip >= 0 && _ip < co->codes.size());
        return _ip;
    }

    PyVar* actual_sp_base() const { return _locals.a; }

    int stack_size(ValueStack* _s) const { return _s->_sp - actual_sp_base(); }
    ArgsView stack_view(ValueStack* _s) const { return ArgsView(actual_sp_base(), _s->_sp); }

    void jump_abs(int i){ _next_ip = i; }
    bool jump_to_exception_handler(ValueStack*);
    int _exit_block(ValueStack*, int);
    void jump_abs_break(ValueStack*, int);

    void loop_break(ValueStack* s_data, const CodeObject*){
        jump_abs_break(s_data, co->_get_block_codei(_ip).end);
    }

    int curr_lineno() const { return co->lines[_ip].lineno; }

    void _gc_mark() const {
        PK_OBJ_MARK(_module);
        co->_gc_mark();
        // Frame could be stored in a generator, so mark _callable for safety
        if(_callable != nullptr) PK_OBJ_MARK(_callable);
    }
};

struct LinkedFrame{
    LinkedFrame* f_back;
    Frame frame;
    template<typename... Args>
    LinkedFrame(LinkedFrame* f_back, Args&&... args) : f_back(f_back), frame(std::forward<Args>(args)...) {}
};

struct CallStack{
    static_assert(sizeof(LinkedFrame) <= 64 && std::is_trivially_destructible_v<LinkedFrame>);

    LinkedFrame* _tail;
    int _size;
    CallStack(): _tail(nullptr), _size(0) {}

    int size() const { return _size; }
    bool empty() const { return _size == 0; }
    void clear(){ while(!empty()) pop(); }

    template<typename... Args>
    void emplace(Args&&... args){
        _tail = new(pool64_alloc<LinkedFrame>()) LinkedFrame(_tail, std::forward<Args>(args)...);
        ++_size;
    }

    void pop(){
        PK_DEBUG_ASSERT(!empty())
        LinkedFrame* p = _tail;
        _tail = p->f_back;
        pool64_dealloc(p);
        --_size;
    }

    Frame& top() const {
        PK_DEBUG_ASSERT(!empty())
        return _tail->frame;
    }

    template<typename Func>
    void apply(Func&& f){
        for(LinkedFrame* p = _tail; p != nullptr; p = p->f_back) f(p->frame);
    }
};

}; // namespace pkpy