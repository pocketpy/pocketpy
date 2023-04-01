#pragma once

#include "codeobject.h"
#include "common.h"
#include "lexer.h"
#include "error.h"
#include "ceval.h"


namespace pkpy{

struct CodeEmitContext;
struct Expr{
    int line = 0;
    virtual ~Expr() = default;
    virtual void emit(CodeEmitContext* ctx) = 0;
    virtual Str str() const = 0;

    virtual void emit_ref(CodeEmitContext* ctx){
        throw std::runtime_error("emit_ref() is not supported");
    }
};

struct CodeEmitContext{
    CodeObject_ co;
    VM* vm;
    stack<Expr_> s_expr;

    CodeEmitContext(VM* vm, CodeObject_ co): co(co) {}
    CodeEmitContext(const CodeEmitContext&) = delete;
    CodeEmitContext& operator=(const CodeEmitContext&) = delete;
    CodeEmitContext(CodeEmitContext&&) = delete;
    CodeEmitContext& operator=(CodeEmitContext&&) = delete;

    int curr_block_i = 0;
    bool is_compiling_class = false;

    bool is_curr_block_loop() const {
        return co->blocks[curr_block_i].type == FOR_LOOP || co->blocks[curr_block_i].type == WHILE_LOOP;
    }

    void enter_block(CodeBlockType type){
        co->blocks.push_back(CodeBlock{
            type, curr_block_i, (int)co->codes.size()
        });
        curr_block_i = co->blocks.size()-1;
    }

    void exit_block(){
        co->blocks[curr_block_i].end = co->codes.size();
        curr_block_i = co->blocks[curr_block_i].parent;
        if(curr_block_i < 0) UNREACHABLE();
    }

    // clear the expression stack and generate bytecode
    void emit_expr(){
        if(s_expr.size() != 1) UNREACHABLE();
        Expr_ expr = s_expr.popx();
        // emit
        // ...
    }

    int emit(Opcode opcode, int arg, int line) {
        co->codes.push_back(
            Bytecode{(uint8_t)opcode, (uint16_t)curr_block_i, arg, line}
        );
        int i = co->codes.size() - 1;
        if(line==BC_KEEPLINE && i>=1) co->codes[i].line = co->codes[i-1].line;
        return i;
    }

    void patch_jump(int index) {
        int target = co->codes.size();
        co->codes[index].arg = target;
    }

    bool add_label(StrName label){
        if(co->labels.count(label)) return false;
        co->labels[label] = co->codes.size();
        return true;
    }

    int add_name(StrName name, NameScope scope){
        if(scope == NAME_LOCAL && co->global_names.count(name)) scope = NAME_GLOBAL;
        auto p = std::make_pair(name, scope);
        for(int i=0; i<co->names.size(); i++){
            if(co->names[i] == p) return i;
        }
        co->names.push_back(p);
        return co->names.size() - 1;
    }

    int add_const(PyObject* v){
        co->consts.push_back(v);
        return co->consts.size() - 1;
    }
};

struct NameExpr: Expr{
    Str name;
    NameScope scope;
    NameExpr(const Str& name, NameScope scope): name(name), scope(scope) {}
    NameExpr(Str&& name, NameScope scope): name(std::move(name)), scope(scope) {}

    Str str() const override { return "$" + name; }

    void emit(CodeEmitContext* ctx) override {
        int index = ctx->add_name(name, scope);
        ctx->emit(OP_LOAD_NAME, index, line);
    }

    void emit_ref(CodeEmitContext* ctx) override {
        int index = ctx->add_name(name, scope);
        ctx->emit(OP_LOAD_NAME_REF, index, line);
    }
};


struct StarredExpr: Expr{
    Expr_ child;
    StarredExpr(Expr_&& child): child(std::move(child)) {}
    Str str() const override { return "*"; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_STAR, (int)false, line);
    }

    void emit_ref(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_STAR, (int)true, line);
    }
};

struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    Str str() const override { return "-"; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_NEGATIVE, BC_NOARG, line);
    }
};

struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    Str str() const override { return "not"; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_NOT, BC_NOARG, line);
    }
};

struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    Str str() const override { return "and"; }

    void emit(CodeEmitContext* ctx) override {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }
};

struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    Str str() const override { return "or"; }

    void emit(CodeEmitContext* ctx) override {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_TRUE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }
};

// [None, True, False, ...]
struct Literal0Expr: Expr{
    TokenIndex token;
    Literal0Expr(TokenIndex token): token(token) {}
    Str str() const override { return TK_STR(token); }

    void emit(CodeEmitContext* ctx) override {
        switch (token) {
            case TK("None"):    ctx->emit(OP_LOAD_NONE, BC_NOARG, line); break;
            case TK("True"):    ctx->emit(OP_LOAD_TRUE, BC_NOARG, line); break;
            case TK("False"):   ctx->emit(OP_LOAD_FALSE, BC_NOARG, line); break;
            case TK("..."):     ctx->emit(OP_LOAD_ELLIPSIS, BC_NOARG, line); break;
            default: UNREACHABLE();
        }
    }
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expr{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    Str str() const override {
        if(std::holds_alternative<i64>(value)){
            return std::to_string(std::get<i64>(value));
        }

        if(std::holds_alternative<f64>(value)){
            return std::to_string(std::get<f64>(value));
        }

        if(std::holds_alternative<Str>(value)){
            return std::get<Str>(value).escape(true);
        }

        UNREACHABLE();
    }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        PyObject* obj = nullptr;
        if(std::holds_alternative<i64>(value)){
            obj = VAR(std::get<i64>(value));
        }

        if(std::holds_alternative<f64>(value)){
            obj = VAR(std::get<f64>(value));
        }

        if(std::holds_alternative<Str>(value)){
            obj = VAR(std::get<Str>(value));
        }

        if(!obj) UNREACHABLE();
        int index = ctx->add_const(obj);
        ctx->emit(OP_LOAD_CONST, index, line);
    }
};

struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    Str str() const override { return "slice()"; }

    void emit(CodeEmitContext* ctx) override {
        if(start){
            start->emit(ctx);
        }else{
            ctx->emit(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(stop){
            stop->emit(ctx);
        }else{
            ctx->emit(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(step){
            step->emit(ctx);
        }else{
            ctx->emit(OP_LOAD_NONE, BC_NOARG, line);
        }

        ctx->emit(OP_BUILD_SLICE, BC_NOARG, line);
    }
};

struct SequenceExpr: Expr{
    std::vector<Expr_> items;
    virtual Opcode opcode() const = 0;

    void emit(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit(ctx);
        ctx->emit(opcode(), items.size(), line);
    }
};

struct ListExpr: SequenceExpr{
    Str str() const override { return "list()"; }
    Opcode opcode() const override { return OP_BUILD_LIST; }
};

struct DictExpr: SequenceExpr{
    Str str() const override { return "dict()"; }
    Opcode opcode() const override { return OP_BUILD_MAP; }
};

struct SetExpr: SequenceExpr{
    Str str() const override { return "set()"; }
    Opcode opcode() const override { return OP_BUILD_SET; }
};

struct TupleExpr: SequenceExpr{
    Str str() const override { return "tuple()"; }
    Opcode opcode() const override { return OP_BUILD_TUPLE; }
};

struct CompExpr: Expr{
    Expr_ expr;       // loop expr
    Expr_ vars;       // loop vars
    Expr_ iter;       // loop iter
    Expr_ cond;       // optional if condition
    virtual void emit_expr() = 0;
};

// a:b
struct DictItemExpr: Expr{
    Expr_ key;
    Expr_ value;
    Str str() const override { return "k:v"; }
};

struct ListCompExpr: CompExpr{
};

struct DictCompExpr: CompExpr{
};

struct SetCompExpr: CompExpr{
};

struct LambdaExpr: Expr{
    Function func;
    NameScope scope;
    Str str() const override { return "<lambda>"; }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        ctx->emit(OP_LOAD_FUNCTION, ctx->add_const(VAR(func)), line);
        if(scope == NAME_LOCAL) ctx->emit(OP_SETUP_CLOSURE, BC_NOARG, line);
    }
};

struct FStringExpr: Expr{
    Str src;
    FStringExpr(const Str& src): src(src) {}
    Str str() const override {
        return "f" + src.escape(true);
    }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        static const std::regex pattern(R"(\{(.*?)\})");
        std::sregex_iterator begin(src.begin(), src.end(), pattern);
        std::sregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::smatch m = *it;
            if (i < m.position()) {
                std::string literal = src.substr(i, m.position() - i);
                ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(literal)), line);
                size++;
            }
            ctx->emit(OP_LOAD_EVAL_FN, BC_NOARG, line);
            ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(m[1].str())), line);
            ctx->emit(OP_CALL, 1, line);
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < src.size()) {
            std::string literal = src.substr(i, src.size() - i);
            ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(literal)), line);
            size++;
        }
        ctx->emit(OP_BUILD_STRING, size, line);
    }
};

struct SubscrExpr: Expr{
    Expr_ a;
    Expr_ b;
    Str str() const override { return "a[b]"; }
};

struct AttribExpr: Expr{
    Expr_ a;
    Str b;
    AttribExpr(Expr_ a, const Str& b): a(std::move(a)), b(b) {}
    AttribExpr(Expr_ a, Str&& b): a(std::move(a)), b(std::move(b)) {}
    Str str() const override { return "a.b"; }
};

struct CallExpr: Expr{
    Expr_ callable;
    std::vector<Expr_> args;
    std::vector<std::pair<Str, Expr_>> kwargs;
    Str str() const override { return "()"; }
};

struct BinaryExpr: Expr{
    TokenIndex op;
    Expr_ lhs;
    Expr_ rhs;
    Str str() const override { return TK_STR(op); }

    void emit(CodeEmitContext* ctx) override {
        lhs->emit(ctx);
        rhs->emit(ctx);
        switch (op) {
            case TK("+"):   ctx->emit(OP_BINARY_OP, 0, line);  break;
            case TK("-"):   ctx->emit(OP_BINARY_OP, 1, line);  break;
            case TK("*"):   ctx->emit(OP_BINARY_OP, 2, line);  break;
            case TK("/"):   ctx->emit(OP_BINARY_OP, 3, line);  break;
            case TK("//"):  ctx->emit(OP_BINARY_OP, 4, line);  break;
            case TK("%"):   ctx->emit(OP_BINARY_OP, 5, line);  break;
            case TK("**"):  ctx->emit(OP_BINARY_OP, 6, line);  break;

            case TK("<"):   ctx->emit(OP_COMPARE_OP, 0, line);    break;
            case TK("<="):  ctx->emit(OP_COMPARE_OP, 1, line);    break;
            case TK("=="):  ctx->emit(OP_COMPARE_OP, 2, line);    break;
            case TK("!="):  ctx->emit(OP_COMPARE_OP, 3, line);    break;
            case TK(">"):   ctx->emit(OP_COMPARE_OP, 4, line);    break;
            case TK(">="):  ctx->emit(OP_COMPARE_OP, 5, line);    break;
            case TK("in"):      ctx->emit(OP_CONTAINS_OP, 0, line);   break;
            case TK("not in"):  ctx->emit(OP_CONTAINS_OP, 1, line);   break;
            case TK("is"):      ctx->emit(OP_IS_OP, 0, line);         break;
            case TK("is not"):  ctx->emit(OP_IS_OP, 1, line);         break;

            case TK("<<"):  ctx->emit(OP_BITWISE_OP, 0, line);    break;
            case TK(">>"):  ctx->emit(OP_BITWISE_OP, 1, line);    break;
            case TK("&"):   ctx->emit(OP_BITWISE_OP, 2, line);    break;
            case TK("|"):   ctx->emit(OP_BITWISE_OP, 3, line);    break;
            case TK("^"):   ctx->emit(OP_BITWISE_OP, 4, line);    break;
            default: UNREACHABLE();
        }
    }
};

struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;

    Str str() const override {
        return "cond ? true_expr : false_expr";
    }

    void emit(CodeEmitContext* ctx) override {
        cond->emit(ctx);
        int patch = ctx->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, cond->line);
        true_expr->emit(ctx);
        int patch_2 = ctx->emit(OP_JUMP_ABSOLUTE, BC_NOARG, true_expr->line);
        ctx->patch_jump(patch);
        false_expr->emit(ctx);
        ctx->patch_jump(patch_2);
    }
};


} // namespace pkpy