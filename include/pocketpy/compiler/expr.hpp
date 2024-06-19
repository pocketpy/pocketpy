#pragma once

#include "pocketpy/objects/codeobject.hpp"
#include "pocketpy/compiler/lexer.hpp"

namespace pkpy {

struct CodeEmitContext;
struct Expr;

typedef small_vector<Expr*, 4> Expr_vector;

static bool default_false(const Expr*) { return false; }
static int default_zero(const Expr*) { return 0; }
static void default_dtor(Expr*) {}

struct ExprVt{
    void (*dtor)(Expr*);
    /* reflections */
    bool (*is_literal)(const Expr*);
    bool (*is_json_object)(const Expr*);
    bool (*is_attrib)(const Expr*);
    bool (*is_subscr)(const Expr*);
    bool (*is_compare)(const Expr*);
    int (*star_level)(const Expr*);
    bool (*is_tuple)(const Expr*);
    bool (*is_name)(const Expr*);
    /* emit */
    void (*emit_)(Expr*, CodeEmitContext*);
    bool (*emit_del)(Expr*, CodeEmitContext*);
    bool (*emit_store)(Expr*, CodeEmitContext*);
    void (*emit_inplace)(Expr*, CodeEmitContext*);
    bool (*emit_store_inplace)(Expr*, CodeEmitContext*);
};

void ExprVt__ctor(ExprVt* vt){
    vt->dtor = default_dtor;
    vt->is_literal = default_false;
    vt->is_json_object = default_false;
    vt->is_attrib = default_false;
    vt->is_subscr = default_false;
    vt->is_compare = default_false;
    vt->star_level = default_zero;
    vt->is_tuple = default_false;
    vt->is_name = default_false;
    vt->emit_ = NULL;   // must be set
    vt->emit_del = NULL;
    vt->emit_store = NULL;
    vt->emit_inplace = NULL;
    vt->emit_store_inplace = NULL;
}

struct Expr {
    ExprVt* vt;
    int line = 0;
    virtual ~Expr() = default;
    virtual void emit_(CodeEmitContext* ctx) = 0;

    Expr() = default;
    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;
    Expr& operator=(const Expr&) = delete;
    Expr& operator=(Expr&&) = delete;

    virtual bool is_literal() const { return false; }
    virtual bool is_json_object() const { return false; }
    virtual bool is_attrib() const { return false; }
    virtual bool is_subscr() const { return false; }
    virtual bool is_compare() const { return false; }
    virtual int star_level() const { return 0; }
    virtual bool is_tuple() const { return false; }
    virtual bool is_name() const { return false; }
    bool is_starred() const { return star_level() > 0; }

    // for OP_DELETE_XXX
    [[nodiscard]] virtual bool emit_del(CodeEmitContext* ctx) { return false; }
    // for OP_STORE_XXX
    [[nodiscard]] virtual bool emit_store(CodeEmitContext* ctx) { return false; }
    virtual void emit_inplace(CodeEmitContext* ctx) { emit_(ctx); }
    [[nodiscard]] virtual bool emit_store_inplace(CodeEmitContext* ctx) { return emit_store(ctx); }
};

void pk_Expr__emit_(Expr* self, CodeEmitContext* ctx){
    assert(self->vt->emit_);
    self->vt->emit_(self, ctx);
}

bool pk_Expr__emit_del(Expr* self, CodeEmitContext* ctx){
    if(!self->vt->emit_del) return false;
    return self->vt->emit_del(self, ctx);
}

bool pk_Expr__emit_store(Expr* self, CodeEmitContext* ctx){
    if(!self->vt->emit_store) return false;
    return self->vt->emit_store(self, ctx);
}

void pk_Expr__emit_inplace(Expr* self, CodeEmitContext* ctx){
    if(!self->vt->emit_inplace){
        pk_Expr__emit_(self, ctx);
        return;
    }
    self->vt->emit_inplace(self, ctx);
}

bool pk_Expr__emit_store_inplace(Expr* self, CodeEmitContext* ctx){
    if(!self->vt->emit_store_inplace){
        return pk_Expr__emit_store(self, ctx);
    }
    return self->vt->emit_store_inplace(self, ctx);
}

inline void delete_expr(Expr* p) noexcept{
    if(!p) return;
    p->~Expr();
    PoolExpr_dealloc(p);
}

struct CodeEmitContext{
    VM* vm;
    FuncDecl_ func;  // optional
    CodeObject_ co;  // 1 CodeEmitContext <=> 1 CodeObject_
    vector<Expr*> _s_expr;
    int level;
    vector<StrName> global_names;

    CodeEmitContext(VM* vm, CodeObject_ co, int level) : vm(vm), co(co), level(level) {
        c11_smallmap_s2n__ctor(&_co_consts_string_dedup_map);
    }

    int curr_iblock = 0;
    bool is_compiling_class = false;

    c11_smallmap_s2n _co_consts_string_dedup_map;

    int get_loop() const noexcept;
    CodeBlock* enter_block(CodeBlockType type) noexcept;
    void exit_block() noexcept;
    int emit_(Opcode opcode, uint16_t arg, int line, bool is_virtual = false) noexcept;
    void revert_last_emit_() noexcept;
    int emit_int(i64 value, int line) noexcept;
    void patch_jump(int index) noexcept;
    bool add_label(StrName name) noexcept;
    int add_varname(StrName name) noexcept;
    int add_const(PyVar) noexcept;
    int add_const_string(std::string_view) noexcept;
    int add_func_decl(FuncDecl_ decl) noexcept;
    void emit_store_name(NameScope scope, StrName name, int line) noexcept;
    void try_merge_for_iter_store(int) noexcept;
    // emit top -> pop -> delete
    void s_emit_top() noexcept{
        s_debug_info("s_emit_top");
        Expr* e = _s_expr.popx_back();
        e->emit_(this);
        delete_expr(e);
    }
    // push
    void s_push(Expr* expr) noexcept{
        s_debug_info("s_push");
        _s_expr.push_back(expr);
    }
    // top
    Expr* s_top() noexcept{
        return _s_expr.back();
    }
    // size
    int s_size() const noexcept{
        return _s_expr.size();
    }
    // pop -> delete
    void s_pop() noexcept{
        s_debug_info("s_pop");
        Expr* e = _s_expr.popx_back();
        delete_expr(e);
    }
    // pop move
    Expr* s_popx() noexcept{
        s_debug_info("s_popx");
        return _s_expr.popx_back();
    }
    // clean
    void s_clean() noexcept{
        s_debug_info("s_clean");
        c11_smallmap_s2n__dtor(&_co_consts_string_dedup_map);
        for(Expr* e: _s_expr) delete_expr(e);
        _s_expr.clear();
    }
    // emit decorators
    void s_emit_decorators(int count) noexcept;
    // debug stack
#if PK_DEBUG_COMPILER
    void s_debug_info(const char* op) noexcept{
        SStream ss;
        for(int i=0; i<s_size(); i++) {
            Expr* e = _s_expr[i];
            ss << typeid(*e).name();
            if(i != s_size() - 1) ss << ", ";
        }
        printf("[%s] %s\n", ss.str().c_str(), op);
    }
#else
    void s_debug_info(const char*) noexcept{}
#endif
};

struct NameExpr : Expr {
    StrName name;
    NameScope scope;

    NameExpr(StrName name, NameScope scope) : name(name), scope(scope) {}

    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;

    bool is_name() const override { return true; }
};

struct _UnaryExpr : Expr {
    Expr* child;
    _UnaryExpr(Expr* child) : child(child) {}
    _UnaryExpr() : child(nullptr) {}
    ~_UnaryExpr() { delete_expr(child); }
};

struct _BinaryExpr : Expr {
    Expr* lhs;
    Expr* rhs;
    _BinaryExpr(Expr* lhs, Expr* rhs) : lhs(lhs), rhs(rhs) {}
    _BinaryExpr() : lhs(nullptr), rhs(nullptr) {}
    ~_BinaryExpr() {
        delete_expr(lhs);
        delete_expr(rhs);
    }
};

struct InvertExpr : _UnaryExpr {
    using _UnaryExpr::_UnaryExpr;
    void emit_(CodeEmitContext* ctx) override;
};

struct StarredExpr : _UnaryExpr {
    int level;

    StarredExpr(Expr* child, int level) : _UnaryExpr(child), level(level) {}

    int star_level() const override { return level; }

    void emit_(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct NotExpr : _UnaryExpr {
    using _UnaryExpr::_UnaryExpr;
    void emit_(CodeEmitContext* ctx) override;
};

struct AndExpr : _BinaryExpr {
    using _BinaryExpr::_BinaryExpr;
    void emit_(CodeEmitContext* ctx) override;
};

struct OrExpr : _BinaryExpr {
    using _BinaryExpr::_BinaryExpr;
    void emit_(CodeEmitContext* ctx) override;
};

// [None, True, False, ...]
struct Literal0Expr : Expr {
    TokenIndex token;

    Literal0Expr(TokenIndex token) : token(token) {}

    bool is_json_object() const override { return true; }

    void emit_(CodeEmitContext* ctx) override;
};

struct LongExpr : Expr {
    Str s;

    LongExpr(const Str& s) : s(s) {}

    void emit_(CodeEmitContext* ctx) override;
};

struct BytesExpr : Expr {
    Str s;

    BytesExpr(const Str& s) : s(s) {}

    void emit_(CodeEmitContext* ctx) override;
};

struct ImagExpr : Expr {
    f64 value;

    ImagExpr(f64 value) : value(value) {}

    void emit_(CodeEmitContext* ctx) override;
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr : Expr {
    TokenValue value;

    LiteralExpr(TokenValue value) : value(value) {}

    void emit_(CodeEmitContext* ctx) override;

    bool is_literal() const override { return true; }

    bool is_json_object() const override { return true; }
};

struct NegatedExpr : _UnaryExpr {
    using _UnaryExpr::_UnaryExpr;
    void emit_(CodeEmitContext* ctx) override;
    bool is_json_object() const override { return child->is_literal(); }
};

struct SliceExpr : Expr {
    Expr* start = nullptr;
    Expr* stop = nullptr;
    Expr* step = nullptr;

    void emit_(CodeEmitContext* ctx) override;

    ~SliceExpr() {
        delete_expr(start);
        delete_expr(stop);
        delete_expr(step);
    }
};

struct DictItemExpr : Expr {
    Expr* key;  // maybe nullptr if it is **kwargs
    Expr* value;

    DictItemExpr(): key(nullptr), value(nullptr) {}

    int star_level() const override { return value->star_level(); }

    void emit_(CodeEmitContext* ctx) override;

    ~DictItemExpr() {
        delete_expr(key);
        delete_expr(value);
    }
};

struct SequenceExpr : Expr {
    array<Expr*> items;

    SequenceExpr(int count) : items(count) {}

    virtual Opcode opcode() const = 0;

    void emit_(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit_(ctx);
        ctx->emit_(opcode(), items.size(), line);
    }

    ~SequenceExpr() {
        for(Expr* item: items) delete_expr(item);
    }
};

struct ListExpr : SequenceExpr {
    using SequenceExpr::SequenceExpr;

    Opcode opcode() const override {
        for(auto& e: items)
            if(e->is_starred()) return OP_BUILD_LIST_UNPACK;
        return OP_BUILD_LIST;
    }

    bool is_json_object() const override { return true; }
};

struct DictExpr : SequenceExpr {
    using SequenceExpr::SequenceExpr;

    Opcode opcode() const override {
        for(auto& e: items)
            if(e->is_starred()) return OP_BUILD_DICT_UNPACK;
        return OP_BUILD_DICT;
    }

    bool is_json_object() const override { return true; }
};

struct SetExpr : SequenceExpr {
    using SequenceExpr::SequenceExpr;

    Opcode opcode() const override {
        for(auto& e: items)
            if(e->is_starred()) return OP_BUILD_SET_UNPACK;
        return OP_BUILD_SET;
    }
};

struct TupleExpr : SequenceExpr {
    using SequenceExpr::SequenceExpr;

    bool is_tuple() const override { return true; }

    Opcode opcode() const override {
        for(auto& e: items)
            if(e->is_starred()) return OP_BUILD_TUPLE_UNPACK;
        return OP_BUILD_TUPLE;
    }

    bool emit_store(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
};

struct CompExpr : Expr {
    Expr* expr = nullptr;  // loop expr
    Expr* vars = nullptr;  // loop vars
    Expr* iter = nullptr;  // loop iter
    Expr* cond = nullptr;  // optional if condition

    Opcode op0;
    Opcode op1;

    CompExpr(Opcode op0, Opcode op1) : op0(op0), op1(op1) {}

    void emit_(CodeEmitContext* ctx) override;

    ~CompExpr() {
        delete_expr(expr);
        delete_expr(vars);
        delete_expr(iter);
        delete_expr(cond);
    }
};

struct LambdaExpr : Expr {
    FuncDecl_ decl;

    LambdaExpr(FuncDecl_ decl) : decl(decl) {}

    void emit_(CodeEmitContext* ctx) override {
        int index = ctx->add_func_decl(decl);
        ctx->emit_(OP_LOAD_FUNCTION, index, line);
    }
};

struct FStringExpr : Expr {
    Str src;

    FStringExpr(const Str& src) : src(src) {}

    void _load_simple_expr(CodeEmitContext* ctx, Str expr);
    void emit_(CodeEmitContext* ctx) override;
};

struct SubscrExpr : _BinaryExpr {
    using _BinaryExpr::_BinaryExpr;
    bool is_subscr() const override { return true; }

    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;

    void emit_inplace(CodeEmitContext* ctx) override;
    bool emit_store_inplace(CodeEmitContext* ctx) override;
};

struct AttribExpr : _UnaryExpr {
    StrName name;

    AttribExpr(Expr* child, StrName name) : _UnaryExpr(child), name(name) {}

    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
    void emit_method(CodeEmitContext* ctx);

    bool is_attrib() const override { return true; }

    void emit_inplace(CodeEmitContext* ctx) override;
    bool emit_store_inplace(CodeEmitContext* ctx) override;
};

struct CallExpr : Expr {
    Expr* callable;
    Expr_vector args;
    // **a will be interpreted as a special keyword argument: {"**": a}
    vector<pair<StrName, Expr*>> kwargs;
    void emit_(CodeEmitContext* ctx) override;

    ~CallExpr() {
        delete_expr(callable);
        for(Expr* arg: args) delete_expr(arg);
        for(auto [_, arg]: kwargs) delete_expr(arg);
    }
};

struct GroupedExpr : _UnaryExpr {
    using _UnaryExpr::_UnaryExpr;
    void emit_(CodeEmitContext* ctx) override { child->emit_(ctx); }

    bool emit_del(CodeEmitContext* ctx) override { return child->emit_del(ctx); }

    bool emit_store(CodeEmitContext* ctx) override { return child->emit_store(ctx); }
};

struct BinaryExpr : _BinaryExpr {
    TokenIndex op;
    bool inplace;

    BinaryExpr(TokenIndex op, bool inplace = false)
        : _BinaryExpr(), op(op), inplace(inplace) {}

    bool is_compare() const override;
    void _emit_compare(CodeEmitContext*, small_vector_2<int, 8>&);
    void emit_(CodeEmitContext* ctx) override;
};

struct TernaryExpr : Expr {
    Expr* cond = nullptr;
    Expr* true_expr = nullptr;
    Expr* false_expr = nullptr;

    void emit_(CodeEmitContext* ctx) override;

    ~TernaryExpr() {
        delete_expr(cond);
        delete_expr(true_expr);
        delete_expr(false_expr);
    }
};

}  // namespace pkpy
