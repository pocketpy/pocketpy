#pragma once

#include "codeobject.h"
#include "common.h"
#include "lexer.h"
#include "error.h"
#include "vm.h"

namespace pkpy{

struct CodeEmitContext;
struct Expr;

#define PK_POOL128_DELETE(ptr) if(ptr != nullptr) { ptr->~T(); pool128_dealloc(ptr); ptr = nullptr; }

template<typename T>
class unique_ptr_128{
    T* ptr;
public:
    unique_ptr_128(): ptr(nullptr) {}
    unique_ptr_128(T* ptr): ptr(ptr) {}
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }
    T* detach() { T* p = ptr; ptr = nullptr; return p; }

    unique_ptr_128(const unique_ptr_128&) = delete;
    unique_ptr_128& operator=(const unique_ptr_128&) = delete;

    bool operator==(std::nullptr_t) const { return ptr == nullptr; }
    bool operator!=(std::nullptr_t) const { return ptr != nullptr; }

    ~unique_ptr_128(){ PK_POOL128_DELETE(ptr) }

    template<typename U>
    unique_ptr_128(unique_ptr_128<U>&& other): ptr(other.detach()) {}

    operator bool() const { return ptr != nullptr; }

    template<typename U>
    unique_ptr_128& operator=(unique_ptr_128<U>&& other) {
        PK_POOL128_DELETE(ptr)
        ptr = other.detach();
        return *this;
    }

    unique_ptr_128& operator=(std::nullptr_t) {
        PK_POOL128_DELETE(ptr)
        ptr = nullptr;
        return *this;
    }
};

typedef unique_ptr_128<Expr> Expr_;
typedef small_vector<Expr_, 4> Expr_vector;

template<>
struct TriviallyRelocatable<Expr_>{
    constexpr static bool value = true;
};

struct Expr{
    int line = 0;
    virtual ~Expr() = default;
    virtual void emit_(CodeEmitContext* ctx) = 0;
    virtual bool is_literal() const { return false; }
    virtual bool is_json_object() const { return false; }
    virtual bool is_attrib() const { return false; }
    virtual bool is_compare() const { return false; }
    virtual int star_level() const { return 0; }
    virtual bool is_tuple() const { return false; }
    virtual bool is_name() const { return false; }
    bool is_starred() const { return star_level() > 0; }

    // for OP_DELETE_XXX
    [[nodiscard]] virtual bool emit_del(CodeEmitContext* ctx) {
        return false;
    }

    // for OP_STORE_XXX
    [[nodiscard]] virtual bool emit_store(CodeEmitContext* ctx) {
        return false;
    }
};

struct CodeEmitContext{
    VM* vm;
    FuncDecl_ func;     // optional
    CodeObject_ co;     // 1 CodeEmitContext <=> 1 CodeObject_
    // some bugs on MSVC (error C2280) when using Expr_vector
    // so we use stack_no_copy instead
    stack_no_copy<Expr_> s_expr;
    int level;
    std::set<Str> global_names;
    CodeEmitContext(VM* vm, CodeObject_ co, int level): vm(vm), co(co), level(level) {}

    int curr_block_i = 0;
    bool is_compiling_class = false;
    int base_stack_size = 0;

    std::map<void*, int> _co_consts_nonstring_dedup_map;
    std::map<std::string, int, std::less<>> _co_consts_string_dedup_map;

    int get_loop() const;
    CodeBlock* enter_block(CodeBlockType type);
    void exit_block();
    void emit_expr();   // clear the expression stack and generate bytecode
    int emit_(Opcode opcode, uint16_t arg, int line, bool is_virtual=false);
    void revert_last_emit_();
    int emit_int(i64 value, int line);
    void patch_jump(int index);
    bool add_label(StrName name);
    int add_varname(StrName name);
    int add_const(PyObject*);
    int add_const_string(std::string_view);
    int add_func_decl(FuncDecl_ decl);
    void emit_store_name(NameScope scope, StrName name, int line);
    void try_merge_for_iter_store(int);
};

struct NameExpr: Expr{
    StrName name;
    NameScope scope;
    NameExpr(StrName name, NameScope scope): name(name), scope(scope) {}
    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
    bool is_name() const override { return true; }
};

struct InvertExpr: Expr{
    Expr_ child;
    InvertExpr(Expr_&& child): child(std::move(child)) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct StarredExpr: Expr{
    int level;
    Expr_ child;
    StarredExpr(int level, Expr_&& child): level(level), child(std::move(child)) {}
    int star_level() const override { return level; }
    void emit_(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    void emit_(CodeEmitContext* ctx) override;
};

struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    void emit_(CodeEmitContext* ctx) override;
};

// [None, True, False, ...]
struct Literal0Expr: Expr{
    TokenIndex token;
    Literal0Expr(TokenIndex token): token(token) {}
    bool is_json_object() const override { return true; }

    void emit_(CodeEmitContext* ctx) override;
};

struct LongExpr: Expr{
    Str s;
    LongExpr(const Str& s): s(s) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct BytesExpr: Expr{
    Str s;
    BytesExpr(const Str& s): s(s) {}
    void emit_(CodeEmitContext* ctx) override;
};

struct ImagExpr: Expr{
    f64 value;
    ImagExpr(f64 value): value(value) {}
    void emit_(CodeEmitContext* ctx) override;
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expr{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    void emit_(CodeEmitContext* ctx) override;
    bool is_literal() const override { return true; }
    bool is_json_object() const override { return true; }
};

struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    void emit_(CodeEmitContext* ctx) override;
    bool is_json_object() const override { return child->is_literal(); }
};

struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    void emit_(CodeEmitContext* ctx) override;
};

struct DictItemExpr: Expr{
    Expr_ key;      // maybe nullptr if it is **kwargs
    Expr_ value;
    int star_level() const override { return value->star_level(); }
    void emit_(CodeEmitContext* ctx) override;
};

struct SequenceExpr: Expr{
    Expr_vector items;
    SequenceExpr(Expr_vector&& items): items(std::move(items)) {}
    virtual Opcode opcode() const = 0;

    void emit_(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit_(ctx);
        ctx->emit_(opcode(), items.size(), line);
    }
};

struct ListExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_LIST_UNPACK;
        return OP_BUILD_LIST;
    }

    bool is_json_object() const override { return true; }
};

struct DictExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_DICT_UNPACK;
        return OP_BUILD_DICT;
    }

    bool is_json_object() const override { return true; }
};

struct SetExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_SET_UNPACK;
        return OP_BUILD_SET;
    }
};

struct TupleExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    bool is_tuple() const override { return true; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_TUPLE_UNPACK;
        return OP_BUILD_TUPLE;
    }

    bool emit_store(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
};

struct CompExpr: Expr{
    Expr_ expr;       // loop expr
    Expr_ vars;       // loop vars
    Expr_ iter;       // loop iter
    Expr_ cond;       // optional if condition

    virtual Opcode op0() = 0;
    virtual Opcode op1() = 0;

    void emit_(CodeEmitContext* ctx) override;
};

struct ListCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_LIST; }
    Opcode op1() override { return OP_LIST_APPEND; }
};

struct DictCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_DICT; }
    Opcode op1() override { return OP_DICT_ADD; }
};

struct SetCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_SET; }
    Opcode op1() override { return OP_SET_ADD; }
};

struct LambdaExpr: Expr{
    FuncDecl_ decl;

    LambdaExpr(FuncDecl_ decl): decl(decl) {}

    void emit_(CodeEmitContext* ctx) override {
        int index = ctx->add_func_decl(decl);
        ctx->emit_(OP_LOAD_FUNCTION, index, line);
    }
};

struct FStringExpr: Expr{
    Str src;
    FStringExpr(const Str& src): src(src) {}
    void _load_simple_expr(CodeEmitContext* ctx, Str expr);
    void emit_(CodeEmitContext* ctx) override;
};

struct SubscrExpr: Expr{
    Expr_ a;
    Expr_ b;
    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct AttribExpr: Expr{
    Expr_ a;
    StrName b;
    AttribExpr(Expr_ a, StrName b): a(std::move(a)), b(b) {}

    void emit_(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
    void emit_method(CodeEmitContext* ctx);
    bool is_attrib() const override { return true; }
};

struct CallExpr: Expr{
    Expr_ callable;
    Expr_vector args;
    // **a will be interpreted as a special keyword argument: {"**": a}
    std::vector<std::pair<Str, Expr_>> kwargs;
    void emit_(CodeEmitContext* ctx) override;
};

struct GroupedExpr: Expr{
    Expr_ a;
    GroupedExpr(Expr_&& a): a(std::move(a)) {}

    void emit_(CodeEmitContext* ctx) override{
        a->emit_(ctx);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        return a->emit_del(ctx);
    }

    bool emit_store(CodeEmitContext* ctx) override {
        return a->emit_store(ctx);
    }
};

struct BinaryExpr: Expr{
    TokenIndex op;
    Expr_ lhs;
    Expr_ rhs;
    bool is_compare() const override;
    void _emit_compare(CodeEmitContext* ctx, pod_vector<int>& jmps);
    void emit_(CodeEmitContext* ctx) override;
};


struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;
    void emit_(CodeEmitContext* ctx) override;
};


} // namespace pkpy