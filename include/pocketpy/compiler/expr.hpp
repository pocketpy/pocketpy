#pragma once

#include "pocketpy/objects/codeobject.hpp"
#include "pocketpy/compiler/lexer.hpp"

namespace pkpy {

struct CodeEmitContext;
struct Expr;

typedef small_vector<Expr*, 4> Expr_vector;

struct Expr {
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

inline void delete_expr(Expr* p){
    if(!p) return;
    p->Expr::~Expr();
    PoolExpr_dealloc(p);
}

struct CodeEmitContext {
    VM* vm;
    FuncDecl_ func;  // optional
    CodeObject_ co;  // 1 CodeEmitContext <=> 1 CodeObject_
    vector<Expr*> s_expr;
    int level;
    vector<StrName> global_names;

    CodeEmitContext(VM* vm, CodeObject_ co, int level) : vm(vm), co(co), level(level) {}

    int curr_iblock = 0;
    bool is_compiling_class = false;

    small_map<PyVar, int> _co_consts_nonstring_dedup_map;
    small_map<std::string_view, int> _co_consts_string_dedup_map;

    int get_loop() const;
    CodeBlock* enter_block(CodeBlockType type);
    void exit_block();
    void emit_expr(bool emit = true);  // clear the expression stack and generate bytecode
    int emit_(Opcode opcode, uint16_t arg, int line, bool is_virtual = false);
    void revert_last_emit_();
    int emit_int(i64 value, int line);
    void patch_jump(int index);
    bool add_label(StrName name);
    int add_varname(StrName name);
    int add_const(PyVar);
    int add_const_string(std::string_view);
    int add_func_decl(FuncDecl_ decl);
    void emit_store_name(NameScope scope, StrName name, int line);
    void try_merge_for_iter_store(int);
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
    Expr_vector items;

    SequenceExpr(Expr_vector&& items) : items(std::move(items)) {}

    virtual Opcode opcode() const = 0;

    void emit_(CodeEmitContext* ctx) override {
        for(auto& item: items)
            item->emit_(ctx);
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

    virtual Opcode op0() = 0;
    virtual Opcode op1() = 0;

    void emit_(CodeEmitContext* ctx) override;

    ~CompExpr() {
        delete_expr(expr);
        delete_expr(vars);
        delete_expr(iter);
        delete_expr(cond);
    }
};

struct ListCompExpr : CompExpr {
    Opcode op0() override { return OP_BUILD_LIST; }

    Opcode op1() override { return OP_LIST_APPEND; }
};

struct DictCompExpr : CompExpr {
    Opcode op0() override { return OP_BUILD_DICT; }

    Opcode op1() override { return OP_DICT_ADD; }
};

struct SetCompExpr : CompExpr {
    Opcode op0() override { return OP_BUILD_SET; }

    Opcode op1() override { return OP_SET_ADD; }
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
    vector<std::pair<StrName, Expr*>> kwargs;
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
