#pragma once

#include "codeobject.h"
#include "common.h"
#include "lexer.h"
#include "error.h"
#include "vm.h"

namespace pkpy{

struct CodeEmitContext;
struct Expr;
typedef std::unique_ptr<Expr> Expr_;

struct Expr{
    int line = 0;
    virtual ~Expr() = default;
    virtual void emit(CodeEmitContext* ctx) = 0;
    virtual std::string str() const = 0;

    virtual bool is_literal() const { return false; }
    virtual bool is_json_object() const { return false; }
    virtual bool is_attrib() const { return false; }
    virtual bool is_compare() const { return false; }
    virtual int star_level() const { return 0; }
    virtual bool is_tuple() const { return false; }
    bool is_starred() const { return star_level() > 0; }

    // for OP_DELETE_XXX
    [[nodiscard]] virtual bool emit_del(CodeEmitContext* ctx) {
        PK_UNUSED(ctx);
        return false;
    }

    // for OP_STORE_XXX
    [[nodiscard]] virtual bool emit_store(CodeEmitContext* ctx) {
        PK_UNUSED(ctx);
        return false;
    }
};

struct CodeEmitContext{
    VM* vm;
    CodeObject_ co;
    // some bugs on MSVC (error C2280) when using std::vector<Expr_>
    // so we use stack_no_copy instead
    stack_no_copy<Expr_> s_expr;
    int level;
    std::set<Str> global_names;
    CodeEmitContext(VM* vm, CodeObject_ co, int level): vm(vm), co(co), level(level) {}

    int curr_block_i = 0;
    bool is_compiling_class = false;
    int for_loop_depth = 0;

    bool is_curr_block_loop() const;
    void enter_block(CodeBlockType type);
    void exit_block();
    void emit_expr();   // clear the expression stack and generate bytecode
    std::string _log_s_expr();
    int emit(Opcode opcode, int arg, int line);
    void patch_jump(int index);
    bool add_label(StrName name);
    int add_varname(StrName name);
    int add_const(PyObject* v);
    int add_func_decl(FuncDecl_ decl);
};

struct NameExpr: Expr{
    StrName name;
    NameScope scope;
    NameExpr(StrName name, NameScope scope): name(name), scope(scope) {}

    std::string str() const override { return fmt("Name(", name.escape(), ")"); }

    void emit(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct InvertExpr: Expr{
    Expr_ child;
    InvertExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Invert()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct StarredExpr: Expr{
    int level;
    Expr_ child;
    StarredExpr(int level, Expr_&& child): level(level), child(std::move(child)) {}
    std::string str() const override { return fmt("Starred(level=", level, ")"); }
    int star_level() const override { return level; }
    void emit(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Not()"; }

    void emit(CodeEmitContext* ctx) override;
};

struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return "And()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    std::string str() const override { return "Or()"; }
    void emit(CodeEmitContext* ctx) override;
};

// [None, True, False, ...]
struct Literal0Expr: Expr{
    TokenIndex token;
    Literal0Expr(TokenIndex token): token(token) {}
    std::string str() const override { return TK_STR(token); }
    bool is_json_object() const override { return true; }

    void emit(CodeEmitContext* ctx) override;
};

struct LongExpr: Expr{
    Str s;
    LongExpr(const Str& s): s(s) {}
    std::string str() const override { return s.str(); }
    void emit(CodeEmitContext* ctx) override;
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expr{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    std::string str() const override;
    void emit(CodeEmitContext* ctx) override;
    bool is_literal() const override { return true; }
    bool is_json_object() const override { return true; }
};

struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    std::string str() const override { return "Negated()"; }

    void emit(CodeEmitContext* ctx) override;
    bool is_json_object() const override { return child->is_literal(); }
};

struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    std::string str() const override { return "Slice()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct DictItemExpr: Expr{
    Expr_ key;      // maybe nullptr if it is **kwargs
    Expr_ value;
    std::string str() const override { return "DictItem()"; }
    int star_level() const override { return value->star_level(); }
    void emit(CodeEmitContext* ctx) override;
};

struct SequenceExpr: Expr{
    std::vector<Expr_> items;
    SequenceExpr(std::vector<Expr_>&& items): items(std::move(items)) {}
    virtual Opcode opcode() const = 0;

    void emit(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit(ctx);
        ctx->emit(opcode(), items.size(), line);
    }
};

struct ListExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "List()"; }

    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_LIST_UNPACK;
        return OP_BUILD_LIST;
    }

    bool is_json_object() const override { return true; }
};

struct DictExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "Dict()"; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_DICT_UNPACK;
        return OP_BUILD_DICT;
    }

    bool is_json_object() const override { return true; }
};

struct SetExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "Set()"; }
    Opcode opcode() const override {
        for(auto& e: items) if(e->is_starred()) return OP_BUILD_SET_UNPACK;
        return OP_BUILD_SET;
    }
};

struct TupleExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    std::string str() const override { return "Tuple()"; }
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

    void emit(CodeEmitContext* ctx) override;
};

struct ListCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_LIST; }
    Opcode op1() override { return OP_LIST_APPEND; }
    std::string str() const override { return "ListComp()"; }
};

struct DictCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_DICT; }
    Opcode op1() override { return OP_DICT_ADD; }
    std::string str() const override { return "DictComp()"; }
};

struct SetCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_SET; }
    Opcode op1() override { return OP_SET_ADD; }
    std::string str() const override { return "SetComp()"; }
};

struct LambdaExpr: Expr{
    FuncDecl_ decl;
    std::string str() const override { return "Lambda()"; }

    LambdaExpr(FuncDecl_ decl): decl(decl) {}

    void emit(CodeEmitContext* ctx) override {
        int index = ctx->add_func_decl(decl);
        ctx->emit(OP_LOAD_FUNCTION, index, line);
    }
};

struct FStringExpr: Expr{
    Str src;
    FStringExpr(const Str& src): src(src) {}
    std::string str() const override {
        return fmt("f", src.escape());
    }

    void _load_simple_expr(CodeEmitContext* ctx, Str expr);
    void emit(CodeEmitContext* ctx) override;
};

struct SubscrExpr: Expr{
    Expr_ a;
    Expr_ b;
    std::string str() const override { return "Subscr()"; }

    void emit(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
};

struct AttribExpr: Expr{
    Expr_ a;
    Str b;
    AttribExpr(Expr_ a, const Str& b): a(std::move(a)), b(b) {}
    AttribExpr(Expr_ a, Str&& b): a(std::move(a)), b(std::move(b)) {}
    std::string str() const override { return "Attrib()"; }

    void emit(CodeEmitContext* ctx) override;
    bool emit_del(CodeEmitContext* ctx) override;
    bool emit_store(CodeEmitContext* ctx) override;
    void emit_method(CodeEmitContext* ctx);
    bool is_attrib() const override { return true; }
};

struct CallExpr: Expr{
    Expr_ callable;
    std::vector<Expr_> args;
    // **a will be interpreted as a special keyword argument: {"**": a}
    std::vector<std::pair<Str, Expr_>> kwargs;
    std::string str() const override { return "Call()"; }
    void emit(CodeEmitContext* ctx) override;
};

struct GroupedExpr: Expr{
    Expr_ a;
    std::string str() const override { return "Grouped()"; }

    GroupedExpr(Expr_&& a): a(std::move(a)) {}

    void emit(CodeEmitContext* ctx) override{
        a->emit(ctx);
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
    std::string str() const override { return TK_STR(op); }

    bool is_compare() const override;
    void _emit_compare(CodeEmitContext* ctx, std::vector<int>& jmps);
    void emit(CodeEmitContext* ctx) override;
};


struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;
    std::string str() const override { return "Ternary()"; }
    void emit(CodeEmitContext* ctx) override;
};


} // namespace pkpy