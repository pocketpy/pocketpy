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

    virtual std::vector<const Expr*> children() const { return {}; }
    virtual bool is_starred() const { return false; }
    virtual bool is_literal() const { return false; }

    // for OP_DELETE_XXX
    virtual bool emit_del(CodeEmitContext* ctx) { return false; }

    // for OP_STORE_XXX
    [[nodiscard]] virtual bool emit_store(CodeEmitContext* ctx) { return false; }
};

struct CodeEmitContext{
    VM* vm;
    CodeObject_ co;
    stack<Expr_> s_expr;
    CodeEmitContext(VM* vm, CodeObject_ co): vm(vm), co(co) {}

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
        expr->emit(this);
    }

    int emit(Opcode opcode, int arg, int line) {
        co->codes.push_back(
            Bytecode{(uint16_t)opcode, (uint16_t)curr_block_i, arg, line}
        );
        int i = co->codes.size() - 1;
        if(line==BC_KEEPLINE){
            if(i>=1) co->codes[i].line = co->codes[i-1].line;
            else co->codes[i].line = 1;
        }
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

    int add_name(StrName name){
        for(int i=0; i<co->names.size(); i++){
            if(co->names[i] == name) return i;
        }
        co->names.push_back(name);
        return co->names.size() - 1;
    }

    int add_const(PyObject* v){
        co->consts.push_back(v);
        return co->consts.size() - 1;
    }

    int add_func_decl(FuncDecl_ decl){
        co->func_decls.push_back(decl);
        return co->func_decls.size() - 1;
    }
};

// PASS
struct NameExpr: Expr{
    StrName name;
    NameScope scope;
    NameExpr(StrName name, NameScope scope): name(name), scope(scope) {}

    Str str() const override { return "$" + name.str(); }

    void emit(CodeEmitContext* ctx) override {
        int index = ctx->add_name(name);
        ctx->emit(OP_LOAD_NAME, index, line);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        int index = ctx->add_name(name);
        switch(scope){
            case NAME_LOCAL:
                ctx->emit(OP_DELETE_LOCAL, index, line);
                break;
            case NAME_GLOBAL:
                ctx->emit(OP_DELETE_GLOBAL, index, line);
                break;
            default: UNREACHABLE(); break;
        }
        return true;
    }

    bool emit_store(CodeEmitContext* ctx) override {
        int index = ctx->add_name(name);
        switch(scope){
            case NAME_LOCAL:
                ctx->emit(OP_STORE_LOCAL, index, line);
                break;
            case NAME_GLOBAL:
                ctx->emit(OP_STORE_GLOBAL, index, line);
                break;
            default: UNREACHABLE(); break;
        }
        return true;
    }
};

// *号运算符，作为左值和右值效果不同
struct StarredExpr: Expr{
    Expr_ child;
    StarredExpr(Expr_&& child): child(std::move(child)) {}
    Str str() const override { return "*"; }

    std::vector<const Expr*> children() const override { return {child.get()}; }

    bool is_starred() const override { return true; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_STAR, BC_NOARG, line);
    }

    bool emit_store(CodeEmitContext* ctx) override {
        // simply proxy to child
        return child->emit_store(ctx);
    }
};

// PASS
struct NotExpr: Expr{
    Expr_ child;
    NotExpr(Expr_&& child): child(std::move(child)) {}
    Str str() const override { return "not"; }

    std::vector<const Expr*> children() const override { return {child.get()}; }

    void emit(CodeEmitContext* ctx) override {
        child->emit(ctx);
        ctx->emit(OP_UNARY_NOT, BC_NOARG, line);
    }
};

// PASS
struct AndExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    Str str() const override { return "and"; }

    std::vector<const Expr*> children() const override { return {lhs.get(), rhs.get()}; }

    void emit(CodeEmitContext* ctx) override {
        lhs->emit(ctx);
        int patch = ctx->emit(OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, line);
        rhs->emit(ctx);
        ctx->patch_jump(patch);
    }
};

// PASS
struct OrExpr: Expr{
    Expr_ lhs;
    Expr_ rhs;
    Str str() const override { return "or"; }

    std::vector<const Expr*> children() const override { return {lhs.get(), rhs.get()}; }

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

    PyObject* to_object(CodeEmitContext* ctx){
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
        return obj;
    }

    void emit(CodeEmitContext* ctx) override {
        PyObject* obj = to_object(ctx);
        if(obj == nullptr) UNREACHABLE();
        int index = ctx->add_const(obj);
        ctx->emit(OP_LOAD_CONST, index, line);
    }

    bool is_literal() const override { return true; }
};

// PASS
struct NegatedExpr: Expr{
    Expr_ child;
    NegatedExpr(Expr_&& child): child(std::move(child)) {}
    Str str() const override { return "-"; }

    std::vector<const Expr*> children() const override { return {child.get()}; }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        // if child is a int of float, do constant folding
        if(child->is_literal()){
            LiteralExpr* lit = static_cast<LiteralExpr*>(child.get());
            PyObject* obj = nullptr;
            if(std::holds_alternative<i64>(lit->value)){
                obj = VAR(-std::get<i64>(lit->value));
            }
            if(std::holds_alternative<f64>(lit->value)){
                obj = VAR(-std::get<f64>(lit->value));
            }
            if(obj != nullptr){
                ctx->emit(OP_LOAD_CONST, ctx->add_const(obj), line);
                return;
            }
        }
        child->emit(ctx);
        ctx->emit(OP_UNARY_NEGATIVE, BC_NOARG, line);
    }
};

// PASS
struct SliceExpr: Expr{
    Expr_ start;
    Expr_ stop;
    Expr_ step;
    Str str() const override { return "slice()"; }

    std::vector<const Expr*> children() const override {
        // may contain nullptr
        return {start.get(), stop.get(), step.get()};
    }

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

struct DictItemExpr: Expr{
    Expr_ key;
    Expr_ value;
    Str str() const override { return "k:v"; }
    std::vector<const Expr*> children() const override { return {key.get(), value.get()}; }

    void emit(CodeEmitContext* ctx) override {
        key->emit(ctx);
        value->emit(ctx);
        ctx->emit(OP_BUILD_TUPLE, 2, line);
    }
};

struct SequenceExpr: Expr{
    std::vector<Expr_> items;
    SequenceExpr(std::vector<Expr_>&& items): items(std::move(items)) {}
    virtual Opcode opcode() const = 0;

    std::vector<const Expr*> children() const override {
        std::vector<const Expr*> ret;
        for(auto& item: items) ret.push_back(item.get());
        return ret;
    }

    void emit(CodeEmitContext* ctx) override {
        for(auto& item: items) item->emit(ctx);
        ctx->emit(opcode(), items.size(), line);
    }
};

struct ListExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Str str() const override { return "list()"; }
    Opcode opcode() const override { return OP_BUILD_LIST; }
};

struct DictExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Str str() const override { return "dict()"; }
    Opcode opcode() const override { return OP_BUILD_DICT; }
};

struct SetExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Str str() const override { return "set()"; }
    Opcode opcode() const override { return OP_BUILD_SET; }
};

struct TupleExpr: SequenceExpr{
    using SequenceExpr::SequenceExpr;
    Str str() const override { return "tuple()"; }
    Opcode opcode() const override { return OP_BUILD_TUPLE; }

    bool emit_store(CodeEmitContext* ctx) override {
        // TOS is an iterable
        // items may contain StarredExpr, we should check it
        int starred_i = -1;
        for(int i=0; i<items.size(); i++){
            if(!items[i]->is_starred()) continue;
            if(starred_i == -1) starred_i = i;
            else return false;  // multiple StarredExpr not allowed
        }

        if(starred_i == -1){
            // Unpacks TOS into count individual values, which are put onto the stack right-to-left.
            ctx->emit(OP_UNPACK_SEQUENCE, items.size(), line);
        }else{
            // starred assignment target must be in a tuple
            if(items.size() == 1) return false;
            // starred assignment target must be the last one (differ from CPython)
            if(starred_i != items.size()-1) return false;
            ctx->emit(OP_UNPACK_EX, items.size()-1, line);
        }
        for(auto& e: items){
            bool ok = e->emit_store(ctx);
            if(!ok) return false;
        }
        return true;
    }

    bool emit_del(CodeEmitContext* ctx) override{
        for(auto& e: items){
            bool ok = e->emit_del(ctx);
            if(!ok) return false;
        }
        return true;
    }
};

struct CompExpr: Expr{
    Expr_ expr;       // loop expr
    Expr_ vars;       // loop vars
    Expr_ iter;       // loop iter
    Expr_ cond;       // optional if condition

    virtual Opcode op0() = 0;
    virtual Opcode op1() = 0;

    void emit(CodeEmitContext* ctx){
        ctx->emit(op0(), 0, line);
        iter->emit(ctx);
        ctx->emit(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        ctx->enter_block(FOR_LOOP);
        ctx->emit(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx);
        // this error occurs in `vars` instead of this line, but...nevermind
        if(!ok) UNREACHABLE();  // TODO: raise a SyntaxError instead
        if(cond){
            cond->emit(ctx);
            int patch = ctx->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
            ctx->emit(op1(), BC_NOARG, BC_KEEPLINE);
            ctx->patch_jump(patch);
        }else{
            ctx->emit(op1(), BC_NOARG, BC_KEEPLINE);
        }
        ctx->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx->exit_block();
    }
};

struct ListCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_LIST; }
    Opcode op1() override { return OP_LIST_APPEND; }
    Str str() const override { return "listcomp()"; }
};

struct DictCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_DICT; }
    Opcode op1() override { return OP_DICT_ADD; }
    Str str() const override { return "dictcomp()"; }
};

struct SetCompExpr: CompExpr{
    Opcode op0() override { return OP_BUILD_SET; }
    Opcode op1() override { return OP_SET_ADD; }
    Str str() const override { return "setcomp()"; }
};

struct LambdaExpr: Expr{
    FuncDecl_ decl;
    NameScope scope;
    Str str() const override { return "<lambda>"; }
    
    LambdaExpr(NameScope scope){
        this->decl = make_sp<FuncDecl>();
        this->decl->name = "<lambda>";
        this->scope = scope;
    }

    void emit(CodeEmitContext* ctx) override {
        int index = ctx->add_func_decl(decl);
        ctx->emit(OP_LOAD_FUNCTION, index, line);
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
            ctx->emit(OP_LOAD_BUILTIN_EVAL, BC_NOARG, line);
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

    void emit(CodeEmitContext* ctx) override{
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_LOAD_SUBSCR, BC_NOARG, line);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_DELETE_SUBSCR, BC_NOARG, line);
        return true;
    }

    bool emit_store(CodeEmitContext* ctx) override {
        a->emit(ctx);
        b->emit(ctx);
        ctx->emit(OP_STORE_SUBSCR, BC_NOARG, line);
        return true;
    }
};

struct AttribExpr: Expr{
    Expr_ a;
    Str b;
    AttribExpr(Expr_ a, const Str& b): a(std::move(a)), b(b) {}
    AttribExpr(Expr_ a, Str&& b): a(std::move(a)), b(std::move(b)) {}
    Str str() const override { return "a.b"; }

    void emit(CodeEmitContext* ctx) override{
        a->emit(ctx);
        int index = ctx->add_name(b);
        ctx->emit(OP_LOAD_ATTR, index, line);
    }

    bool emit_del(CodeEmitContext* ctx) override {
        a->emit(ctx);
        int index = ctx->add_name(b);
        ctx->emit(OP_DELETE_ATTR, index, line);
        return true;
    }

    bool emit_store(CodeEmitContext* ctx) override {
        a->emit(ctx);
        int index = ctx->add_name(b);
        ctx->emit(OP_STORE_ATTR, index, line);
        return true;
    }
};

// PASS
struct CallExpr: Expr{
    Expr_ callable;
    std::vector<Expr_> args;
    std::vector<std::pair<Str, Expr_>> kwargs;
    Str str() const override { return "call(...)"; }

    std::vector<const Expr*> children() const override {
        std::vector<const Expr*> ret;
        for(auto& item: args) ret.push_back(item.get());
        // ...ignore kwargs for simplicity
        return ret;
    }

    bool need_unpack() const {
        for(auto& item: args) if(item->is_starred()) return true;
        return false;
    }

    void emit(CodeEmitContext* ctx) override {
        VM* vm = ctx->vm;
        callable->emit(ctx);
        // emit args
        for(auto& item: args) item->emit(ctx);
        // emit kwargs
        for(auto& item: kwargs){
            // TODO: optimize this
            ctx->emit(OP_LOAD_CONST, ctx->add_const(VAR(item.first)), line);
            item.second->emit(ctx);
        }
        int KWARGC = (int)kwargs.size();
        int ARGC = (int)args.size();
        if(KWARGC > 0){
            ctx->emit(need_unpack() ? OP_CALL_KWARGS_UNPACK : OP_CALL_KWARGS, (KWARGC<<16)|ARGC, line);
        }else{
            ctx->emit(need_unpack() ? OP_CALL_UNPACK : OP_CALL, ARGC, line);
        }
    }
};

struct BinaryExpr: Expr{
    TokenIndex op;
    Expr_ lhs;
    Expr_ rhs;
    Str str() const override { return TK_STR(op); }

    std::vector<const Expr*> children() const override {
        return {lhs.get(), rhs.get()};
    }

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

// PASS
struct TernaryExpr: Expr{
    Expr_ cond;
    Expr_ true_expr;
    Expr_ false_expr;

    Str str() const override {
        return "cond ? t : f";
    }

    std::vector<const Expr*> children() const override {
        return {cond.get(), true_expr.get(), false_expr.get()};
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