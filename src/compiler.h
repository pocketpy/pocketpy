#pragma once

#include "codeobject.h"
#include "common.h"
#include "lexer.h"
#include "error.h"
#include "ceval.h"
#include "expr.h"
#include "obj.h"
#include "str.h"

namespace pkpy{

class Compiler;
typedef void (Compiler::*PrattCallback)();

struct PrattRule{
    PrattCallback prefix;
    PrattCallback infix;
    Precedence precedence;
};

class Compiler {
    std::unique_ptr<Lexer> lexer;
    stack<CodeEmitContext> contexts;
    std::map<TokenIndex, PrattRule> rules;
    bool used = false;
    VM* vm;

    CodeObject* co() const{ return contexts.top().co.get(); }
    CodeEmitContext* ctx() { return &contexts.top(); }
    CompileMode mode() const{ return lexer->src->mode; }
    NameScope name_scope() const { return contexts.size()>1 ? NAME_LOCAL : NAME_GLOBAL; }

    template<typename... Args>
    CodeObject_ push_context(Args&&... args){
        CodeObject_ co = make_sp<CodeObject>(std::forward<Args>(args)...);
        contexts.push(CodeEmitContext(vm, co));
        return co;
    }

    void pop_context(){
        if(!ctx()->s_expr.empty()) UNREACHABLE();
        if(ctx()->co->codes.back().op != OP_RETURN_VALUE){
            ctx()->emit(OP_LOAD_NONE, BC_NOARG, BC_KEEPLINE);
            ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        }
        ctx()->co->optimize(vm);
        contexts.pop();
    }

public:
    Compiler(VM* vm, const char* source, Str filename, CompileMode mode){
        this->vm = vm;
        this->lexer = std::make_unique<Lexer>(
            make_sp<SourceData>(source, filename, mode)
        );

// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define METHOD(name) &Compiler::name
#define NO_INFIX nullptr, PREC_NONE
        for(TokenIndex i=0; i<kTokenCount; i++) rules[i] = { nullptr, NO_INFIX };
        rules[TK(".")] =    { nullptr,               METHOD(exprAttrib),         PREC_ATTRIB };
        rules[TK("(")] =    { METHOD(exprGroup),     METHOD(exprCall),           PREC_CALL };
        rules[TK("[")] =    { METHOD(exprList),      METHOD(exprSubscr),         PREC_SUBSCRIPT };
        rules[TK("{")] =    { METHOD(exprMap),       NO_INFIX };
        rules[TK("%")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("+")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("-")] =    { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("*")] =    { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("/")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("//")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("**")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_EXPONENT };
        rules[TK(">")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("==")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_EQUALITY };
        rules[TK("!=")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_EQUALITY };
        rules[TK(">=")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<=")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("in")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("is")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("not in")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("is not")] =   { nullptr,               METHOD(exprBinaryOp),       PREC_TEST };
        rules[TK("and") ] =     { nullptr,               METHOD(exprAnd),            PREC_LOGICAL_AND };
        rules[TK("or")] =       { nullptr,               METHOD(exprOr),             PREC_LOGICAL_OR };
        rules[TK("not")] =      { METHOD(exprNot),       nullptr,                    PREC_LOGICAL_NOT };
        rules[TK("True")] =     { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("False")] =    { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("None")] =     { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("...")] =      { METHOD(exprLiteral0),  NO_INFIX };
        rules[TK("lambda")] =   { METHOD(exprLambda),    NO_INFIX };
        rules[TK("@id")] =      { METHOD(exprName),      NO_INFIX };
        rules[TK("@num")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@str")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@fstr")] =    { METHOD(exprFString),   NO_INFIX };
        rules[TK("?")] =        { nullptr,               METHOD(exprTernary),        PREC_TERNARY };
        rules[TK(",")] =        { nullptr,               METHOD(exprTuple),          PREC_TUPLE };
        rules[TK("<<")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
#undef METHOD
#undef NO_INFIX

        // rules[TK("=")] =        { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        // rules[TK("+=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("-=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("*=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("/=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("//=")] =      { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("%=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("&=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("|=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("^=")] =       { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK(">>=")] =      { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
        // rules[TK("<<=")] =      { nullptr,               METHOD(exprInplaceAssign),  PREC_ASSIGNMENT };
    }

private:
    int i = 0;
    std::vector<Token> tokens;

    const Token& prev() { return tokens.at(i-1); }
    const Token& curr() { return tokens.at(i); }
    const Token& next() { return tokens.at(i+1); }
    const Token& peek(int offset) { return tokens.at(i+offset); }
    void advance() { i++; }

    bool match(TokenIndex expected) {
        if (curr().type != expected) return false;
        advance();
        return true;
    }

    void consume(TokenIndex expected) {
        if (!match(expected)){
            StrStream ss;
            ss << "expected '" << TK_STR(expected) << "', but got '" << TK_STR(curr().type) << "'";
            SyntaxError(ss.str());
        }
    }

    bool match_newlines(bool repl_throw=false) {
        bool consumed = false;
        if (curr().type == TK("@eol")) {
            while (curr().type == TK("@eol")) advance();
            consumed = true;
        }
        if (repl_throw && curr().type == TK("@eof")){
            throw NeedMoreLines(ctx()->is_compiling_class);
        }
        return consumed;
    }

    bool match_end_stmt() {
        if (match(TK(";"))) { match_newlines(); return true; }
        if (match_newlines() || curr().type == TK("@eof")) return true;
        if (curr().type == TK("@dedent")) return true;
        return false;
    }

    void consume_end_stmt() {
        if (!match_end_stmt()) SyntaxError("expected statement end");
    }

    void EXPR(ExprAction action=EXPR_PUSH_STACK) {
        parse_expression(PREC_TUPLE + 1, action);
    }

    void EXPR_TUPLE(ExprAction action=EXPR_PUSH_STACK) {
        parse_expression(PREC_TUPLE, action);
    }

    template <typename T, typename... Args>
    std::unique_ptr<T> expr_prev_line(Args&&... args) {
        std::unique_ptr<T> expr = std::make_unique<T>(std::forward<Args>(args)...);
        expr->line = prev().line;
        return expr;
    }

    /********************************************/

    // PASS
    void exprLiteral(){
        ctx()->s_expr.push(expr_prev_line<LiteralExpr>(prev().value));
    }

    // PASS
    void exprFString(){
        ctx()->s_expr.push(expr_prev_line<FStringExpr>(std::get<Str>(prev().value)));
    }

    // PASS
    void exprLambda(){
        auto e = expr_prev_line<LambdaExpr>();
        e->func.name = "<lambda>";
        e->scope = name_scope();
        if(!match(TK(":"))){
            _compile_f_args(e->func, false);
            consume(TK(":"));
        }
        e->func.code = push_context(lexer->src, "<lambda>");
        // https://github.com/blueloveTH/pocketpy/issues/37
        EXPR(true);
        ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        pop_context();
        ctx()->s_expr.push(std::move(e));
    }

    void exprAssign(){
        // if(co()->codes.empty()) UNREACHABLE();
        // bool is_load_name_ref = co()->codes.back().op == OP_LOAD_NAME_REF;
        // int _name_arg = co()->codes.back().arg;
        // // if the last op is OP_LOAD_NAME_REF, remove it
        // // because we will emit OP_STORE_NAME or OP_STORE_CLASS_ATTR
        // if(is_load_name_ref) co()->codes.pop_back();

        // co()->_rvalue += 1;
        // TokenIndex op = prev().type;
        // if(op == TK("=")) {     // a = (expr)
        //     EXPR_TUPLE();
        //     if(is_load_name_ref){
        //         auto op = ctx()->is_compiling_class ? OP_STORE_CLASS_ATTR : OP_STORE_NAME;
        //         emit(op, _name_arg);
        //     }else{
        //         if(ctx()->is_compiling_class) SyntaxError();
        //         emit(OP_STORE_REF);
        //     }
        // }else{                  // a += (expr) -> a = a + (expr)
        //     if(ctx()->is_compiling_class) SyntaxError();
        //     if(is_load_name_ref){
        //         emit(OP_LOAD_NAME, _name_arg);
        //     }else{
        //         emit(OP_DUP_TOP_VALUE);
        //     }
        //     EXPR();
        //     switch (op) {
        //         case TK("+="):      emit(OP_BINARY_OP, 0);  break;
        //         case TK("-="):      emit(OP_BINARY_OP, 1);  break;
        //         case TK("*="):      emit(OP_BINARY_OP, 2);  break;
        //         case TK("/="):      emit(OP_BINARY_OP, 3);  break;
        //         case TK("//="):     emit(OP_BINARY_OP, 4);  break;
        //         case TK("%="):      emit(OP_BINARY_OP, 5);  break;
        //         case TK("<<="):     emit(OP_BITWISE_OP, 0);  break;
        //         case TK(">>="):     emit(OP_BITWISE_OP, 1);  break;
        //         case TK("&="):      emit(OP_BITWISE_OP, 2);  break;
        //         case TK("|="):      emit(OP_BITWISE_OP, 3);  break;
        //         case TK("^="):      emit(OP_BITWISE_OP, 4);  break;
        //         default: UNREACHABLE();
        //     }
        //     if(is_load_name_ref){
        //         emit(OP_STORE_NAME, _name_arg);
        //     }else{
        //         emit(OP_STORE_REF);
        //     }
        // }
        // co()->_rvalue -= 1;
    }

    // PASS
    void exprTuple(){
        auto e = expr_prev_line<TupleExpr>();
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            e->items.push_back(ctx()->s_expr.popx());
        } while(match(TK(",")));
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprOr(){
        auto e = expr_prev_line<OrExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_OR + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprAnd(){
        auto e = expr_prev_line<AndExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_AND + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprTernary(){
        auto e = expr_prev_line<TernaryExpr>();
        e->cond = ctx()->s_expr.popx();
        EXPR();         // if true
        e->true_expr = ctx()->s_expr.popx();
        consume(TK(":"));
        EXPR();         // if false
        e->false_expr = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprBinaryOp(){
        auto e = expr_prev_line<BinaryExpr>();
        e->op = prev().type;
        e->lhs = ctx()->s_expr.popx();
        parse_expression(rules[e->op].precedence + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprNot() {
        parse_expression(PREC_LOGICAL_NOT + 1);
        ctx()->s_expr.push(expr_prev_line<NotExpr>(ctx()->s_expr.popx()));
    }

    // PASS
    void exprUnaryOp(){
        TokenIndex op = prev().type;
        parse_expression(PREC_UNARY + 1);
        switch(op){
            case TK("-"):
                ctx()->s_expr.push(expr_prev_line<NegatedExpr>(ctx()->s_expr.popx()));
                break;
            case TK("*"):
                ctx()->s_expr.push(expr_prev_line<StarredExpr>(ctx()->s_expr.popx()));
                break;
            default: UNREACHABLE();
        }
    }

    // PASS
    void exprGroup(){
        match_newlines(mode()==REPL_MODE);
        EXPR_TUPLE();   // () is just for change precedence
        match_newlines(mode()==REPL_MODE);
        consume(TK(")"));
    }

    // PASS
    template<typename T>
    void _consume_comp(Expr_ expr){
        static_assert(std::is_base_of<CompExpr, T>::value);
        std::unique_ptr<CompExpr> ce = std::make_unique<T>();
        ce->expr = std::move(expr);
        EXPR_TUPLE();   // must be a lvalue
        ce->vars = ctx()->s_expr.popx();
        consume(TK("in"));
        EXPR();
        ce->iter = ctx()->s_expr.popx();
        match_newlines(mode()==REPL_MODE);
        if(match(TK("if"))){
            EXPR();
            ce->cond = ctx()->s_expr.popx();
        }
        ctx()->s_expr.push(std::move(ce));
        match_newlines(mode()==REPL_MODE);
    }

    // PASS
    void exprList() {
        auto e = expr_prev_line<ListExpr>();
        do {
            match_newlines(mode()==REPL_MODE);
            if (curr().type == TK("]")) break;
            EXPR();
            e->items.push_back(ctx()->s_expr.popx());
            match_newlines(mode()==REPL_MODE);
            if(e->items.size()==1 && match(TK("for"))){
                _consume_comp<ListCompExpr>(std::move(e->items[0]));
                consume(TK("]"));
                return;
            }
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK("]"));
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprMap() {
        bool parsing_dict = false;  // {...} may be dict or set
        std::vector<Expr_> items;
        do {
            match_newlines(mode()==REPL_MODE);
            if (curr().type == TK("}")) break;
            EXPR();
            if(curr().type == TK(":")) parsing_dict = true;
            if(parsing_dict){
                consume(TK(":"));
                EXPR();
                auto dict_item = expr_prev_line<DictItemExpr>();
                dict_item->key = ctx()->s_expr.popx();
                dict_item->value = ctx()->s_expr.popx();
                items.push_back(std::move(dict_item));
            }else{
                items.push_back(ctx()->s_expr.popx());
            }
            match_newlines(mode()==REPL_MODE);
            if(items.size()==1 && match(TK("for"))){
                if(parsing_dict) _consume_comp<DictCompExpr>(std::move(items[0]));
                else _consume_comp<SetCompExpr>(std::move(items[0]));
                consume(TK("}"));
                return;
            }
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK("}"));
        if(items.size()==0 || parsing_dict){
            auto e = expr_prev_line<DictExpr>(std::move(items));
            ctx()->s_expr.push(std::move(e));
        }else{
            auto e = expr_prev_line<SetExpr>(std::move(items));
            ctx()->s_expr.push(std::move(e));
        }
    }

    // PASS
    void exprCall() {
        auto e = expr_prev_line<CallExpr>();
        e->callable = ctx()->s_expr.popx();
        do {
            match_newlines(mode()==REPL_MODE);
            if (curr().type==TK(")")) break;
            if(curr().type==TK("@id") && next().type==TK("=")) {
                consume(TK("@id"));
                Str key = prev().str();
                consume(TK("="));
                EXPR();
                e->kwargs.push_back({key, ctx()->s_expr.popx()});
            } else{
                if(!e->kwargs.empty()) SyntaxError("positional argument follows keyword argument");
                EXPR();
                // if(co()->codes.back().op == OP_UNARY_STAR) need_unpack = true;
                e->args.push_back(ctx()->s_expr.popx());
            }
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK(")"));
        ctx()->s_expr.push(std::move(e));
        // if(ARGC > 32767) SyntaxError("too many positional arguments");
        // if(KWARGC > 32767) SyntaxError("too many keyword arguments");
        // if(KWARGC > 0){
        //     emit(need_unpack ? OP_CALL_KWARGS_UNPACK : OP_CALL_KWARGS, (KWARGC << 16) | ARGC);
        // }else{
        //     emit(need_unpack ? OP_CALL_UNPACK : OP_CALL, ARGC);
        // }
    }

    // PASS
    void exprName(){
        ctx()->s_expr.push(expr_prev_line<NameExpr>(prev().str(), name_scope()));
    }

    // PASS
    void exprAttrib() {
        consume(TK("@id"));
        ctx()->s_expr.push(
            expr_prev_line<AttribExpr>(ctx()->s_expr.popx(), prev().str())
        );
    }

    // PASS
    void exprSubscr() {
        auto e = expr_prev_line<SubscrExpr>();
        std::vector<Expr_> items;
        do {
            EXPR_TUPLE();
            items.push_back(ctx()->s_expr.popx());
        } while(match(TK(":")));
        consume(TK("]"));
        switch(items.size()){
            case 1:
                e->b = std::move(items[0]);
                break;
            case 2: case 3: {
                auto slice = expr_prev_line<SliceExpr>();
                slice->start = std::move(items[0]);
                slice->stop = std::move(items[1]);
                if(items.size()==3){
                    slice->step = std::move(items[2]);
                }
                e->b = std::move(slice);
            } break;
            default: SyntaxError(); break;
        }
        ctx()->s_expr.push(std::move(e));
    }

    // PASS
    void exprLiteral0() {
        ctx()->s_expr.push(expr_prev_line<Literal0Expr>(prev().type));
    }

    void compile_block_body() {
        consume(TK(":"));
        if(curr().type!=TK("@eol") && curr().type!=TK("@eof")){
            compile_stmt();     // inline block
            return;
        }
        if(!match_newlines(mode()==REPL_MODE)){
            SyntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (curr().type != TK("@dedent")) {
            match_newlines();
            compile_stmt();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    Str _compile_import() {
        consume(TK("@id"));
        Str name = prev().str();
        int index = ctx()->add_name(name, NAME_SPECIAL);
        ctx()->emit(OP_IMPORT_NAME, index, peek(-2).line);
        return name;
    }

    // import a as b
    void compile_normal_import() {
        do {
            Str name = _compile_import();
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            int index = ctx()->add_name(name, name_scope());
            ctx()->emit(OP_STORE_NAME, index, prev().line);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b as c, d as e
    void compile_from_import() {
        _compile_import();
        consume(TK("import"));
        if (match(TK("*"))) {
            if(name_scope() != NAME_GLOBAL) SyntaxError("import * can only be used in global scope");
            ctx()->emit(OP_STORE_ALL_NAMES, BC_NOARG, prev().line);
            consume_end_stmt();
            return;
        }
        do {
            ctx()->emit(OP_DUP_TOP_VALUE, BC_NOARG, BC_KEEPLINE);
            consume(TK("@id"));
            Str name = prev().str();
            int index = ctx()->add_name(name, NAME_ATTR);
            ctx()->emit(OP_BUILD_ATTR, index, prev().line);
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            index = ctx()->add_name(name, name_scope());
            ctx()->emit(OP_STORE_NAME, index, prev().line);
        } while (match(TK(",")));
        ctx()->emit(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
        consume_end_stmt();
    }

    void parse_expression(int precedence, bool push_stack=true) {
        advance();
        PrattCallback prefix = rules[prev().type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, but got ") + TK_STR(prev().type));
        (this->*prefix)();
        while (rules[curr().type].precedence >= precedence) {
            TokenIndex op = curr().type;
            advance();
            PrattCallback infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
        if(!push_stack) ctx()->emit_expr();
    }

    void compile_if_stmt() {
        EXPR(true);   // condition
        int patch = ctx()->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        if (match(TK("elif"))) {
            int exit_patch = ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_if_stmt();
            ctx()->patch_jump(exit_patch);
        } else if (match(TK("else"))) {
            int exit_patch = ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_block_body();
            ctx()->patch_jump(exit_patch);
        } else {
            ctx()->patch_jump(patch);
        }
    }

    void compile_while_loop() {
        ctx()->enter_block(WHILE_LOOP);
        EXPR(true);   // condition
        int patch = ctx()->emit(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx()->patch_jump(patch);
        ctx()->exit_block();
    }

    void compile_for_loop() {
        EXPR_TUPLE();
        ctx()->emit_lvalue();
        consume(TK("in"));
        EXPR(true);
        ctx()->emit(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        ctx()->enter_block(FOR_LOOP);
        ctx()->emit(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        compile_block_body();
        ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, BC_KEEPLINE);
        ctx()->exit_block();
    }

    void compile_try_except() {
        ctx()->enter_block(TRY_EXCEPT);
        ctx()->emit(OP_TRY_BLOCK_ENTER, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit(OP_TRY_BLOCK_EXIT, BC_NOARG, BC_KEEPLINE);
        std::vector<int> patches = {
            ctx()->emit(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE)
        };
        ctx()->exit_block();

        do {
            consume(TK("except"));
            if(match(TK("@id"))){
                int name_idx = ctx()->add_name(prev().str(), NAME_SPECIAL);
                emit(OP_EXCEPTION_MATCH, name_idx);
            }else{
                emit(OP_LOAD_TRUE);
            }
            int patch = emit(OP_POP_JUMP_IF_FALSE);
            emit(OP_POP_TOP);       // pop the exception on match
            compile_block_body();
            patches.push_back(emit(OP_JUMP_ABSOLUTE));
            patch_jump(patch);
        }while(curr().type == TK("except"));
        emit(OP_RE_RAISE);      // no match, re-raise
        for (int patch : patches) patch_jump(patch);
    }

    void compile_decorated(){
        EXPR(true);
        if(!match_newlines(mode()==REPL_MODE)) SyntaxError();
        ctx()->emit(OP_SETUP_DECORATOR, BC_NOARG, prev().line);
        consume(TK("def"));
        compile_function();
    }

    bool try_compile_assignment(){

    }

    void compile_stmt() {
        advance();
        int kw_line = prev().line;  // backup line number
        switch(prev().type){
            case TK("break"):
                if (!ctx()->is_curr_block_loop()) SyntaxError("'break' outside loop");
                ctx()->emit(OP_LOOP_BREAK, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("continue"):
                if (!ctx()->is_curr_block_loop()) SyntaxError("'continue' not properly in loop");
                ctx()->emit(OP_LOOP_CONTINUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("yield"):
                if (contexts.size() <= 1) SyntaxError("'yield' outside function");
                EXPR_TUPLE(true);
                // if yield present, the function is a generator
                ctx()->co->is_generator = true;
                ctx()->emit(OP_YIELD_VALUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("return"):
                if (contexts.size() <= 1) SyntaxError("'return' outside function");
                if(match_end_stmt()){
                    ctx()->emit(OP_LOAD_NONE, BC_NOARG, kw_line);
                }else{
                    EXPR_TUPLE(true);
                    consume_end_stmt();
                }
                ctx()->emit(OP_RETURN_VALUE, BC_NOARG, kw_line);
                break;
            /*************************************************/
            case TK("if"): compile_if_stmt(); break;
            case TK("while"): compile_while_loop(); break;
            case TK("for"): compile_for_loop(); break;
            case TK("import"): compile_normal_import(); break;
            case TK("from"): compile_from_import(); break;
            case TK("def"): compile_function(); break;
            case TK("@"): compile_decorated(); break;
            case TK("try"): compile_try_except(); break;
            case TK("pass"): consume_end_stmt(); break;
            /*************************************************/
            case TK("assert"):
                EXPR_TUPLE(true);
                // TODO: change OP_ASSERT impl in ceval.h
                ctx()->emit(OP_ASSERT, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("del"):
                EXPR_TUPLE();
                ctx()->emit_lvalue();
                ctx()->emit(OP_DELETE_REF, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("global"):
                do {
                    consume(TK("@id"));
                    co()->global_names.insert(prev().str());
                } while (match(TK(",")));
                consume_end_stmt();
                break;
            case TK("raise"): {
                consume(TK("@id"));
                int dummy_t = ctx()->add_name(prev().str(), NAME_SPECIAL);
                if(match(TK("(")) && !match(TK(")"))){
                    EXPR(true); consume(TK(")"));
                }else{
                    ctx()->emit(OP_LOAD_NONE, BC_NOARG, BC_KEEPLINE);
                }
                ctx()->emit(OP_RAISE, dummy_t, kw_line);
                consume_end_stmt();
            } break;
            case TK("with"): {
                EXPR(true);
                consume(TK("as"));
                consume(TK("@id"));
                int index = ctx()->add_name(prev().str(), name_scope());
                emit(OP_STORE_NAME, index);
                emit(OP_LOAD_NAME_REF, index);
                emit(OP_WITH_ENTER);
                compile_block_body();
                emit(OP_LOAD_NAME_REF, index);
                emit(OP_WITH_EXIT);
            } break;
            /*************************************************/
            // TODO: refactor goto/label use special $ syntax
            case TK("label"):
                if(mode()!=EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
                consume(TK(".")); consume(TK("@id"));
                bool ok = co()->add_label(prev().str());
                if(!ok) SyntaxError("label " + prev().str().escape(true) + " already exists");
                consume_end_stmt();
                break;
            case TK("goto"):
                if(mode() != EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
                consume(TK(".")); consume(TK("@id"));
                emit(OP_GOTO, co()->add_name(prev().str(), NAME_SPECIAL));
                consume_end_stmt();
                break;
            /*************************************************/
            // dangling expression or assignment
            default: {
                EXPR_TUPLE(true);
                bool assigment = try_compile_assignment();
                if(!assigment){
                    if(mode()==REPL_MODE && name_scope()==NAME_GLOBAL){
                        emit(OP_PRINT_EXPR, BC_NOARG, BC_KEEPLINE);
                    }
                    emit(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                }
                consume_end_stmt();
            }
        }
    }

    void compile_class(){
        consume(TK("@id"));
        int cls_name_idx = co()->add_name(prev().str(), NAME_GLOBAL);
        int super_cls_name_idx = -1;
        if(match(TK("(")) && match(TK("@id"))){
            super_cls_name_idx = co()->add_name(prev().str(), NAME_GLOBAL);
            consume(TK(")"));
        }
        if(super_cls_name_idx == -1) emit(OP_LOAD_NONE);
        else emit(OP_LOAD_NAME, super_cls_name_idx);
        emit(OP_BEGIN_CLASS, cls_name_idx);
        ctx()->is_compiling_class = true;
        compile_block_body();
        ctx()->is_compiling_class = false;
        emit(OP_END_CLASS);
    }

    void _compile_f_args(Function& func, bool enable_type_hints){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state == 3) SyntaxError("**kwargs should be the last argument");
            match_newlines();
            if(match(TK("*"))){
                if(state < 1) state = 1;
                else SyntaxError("*args should be placed before **kwargs");
            }
            else if(match(TK("**"))){
                state = 3;
            }

            consume(TK("@id"));
            const Str& name = prev().str();
            if(func.has_name(name)) SyntaxError("duplicate argument name");

            // eat type hints
            if(enable_type_hints && match(TK(":"))) consume(TK("@id"));

            if(state == 0 && curr().type == TK("=")) state = 2;

            switch (state)
            {
                case 0: func.args.push_back(name); break;
                case 1: func.starred_arg = name; state+=1; break;
                case 2: {
                    consume(TK("="));
                    PyObject* value = read_literal();
                    if(value == nullptr){
                        SyntaxError(Str("expect a literal, not ") + TK_STR(curr().type));
                    }
                    func.kwargs.set(name, value);
                    func.kwargs_order.push_back(name);
                } break;
                case 3: SyntaxError("**kwargs is not supported yet"); break;
            }
        } while (match(TK(",")));
    }

    void compile_function(){
        // TODO: bug, if there are multiple decorators, will cause error
        bool has_decorator = !co()->codes.empty() && co()->codes.back().op == OP_SETUP_DECORATOR;
        Function func;
        StrName obj_name;
        consume(TK("@id"));
        func.name = prev().str();
        if(!ctx()->is_compiling_class && match(TK("::"))){
            consume(TK("@id"));
            obj_name = func.name;
            func.name = prev().str();
        }
        consume(TK("("));
        if (!match(TK(")"))) {
            _compile_f_args(func, true);
            consume(TK(")"));
        }
        if(match(TK("->"))){
            if(!match(TK("None"))) consume(TK("@id"));
        }
        func.code = push_context(lexer->src, func.name.str());
        compile_block_body();
        pop_context();
        emit(OP_LOAD_FUNCTION, co()->add_const(VAR(func)));
        if(name_scope() == NAME_LOCAL) emit(OP_SETUP_CLOSURE);
        if(!ctx()->is_compiling_class){
            if(obj_name.empty()){
                if(has_decorator) emit(OP_CALL, 1);
                emit(OP_STORE_NAME, co()->add_name(func.name, name_scope()));
            } else {
                if(has_decorator) SyntaxError("decorator is not supported here");
                emit(OP_LOAD_NAME, co()->add_name(obj_name, name_scope()));
                int index = co()->add_name(func.name, NAME_ATTR);
                emit(OP_BUILD_ATTR_REF, index);
                emit(OP_ROT_TWO);
                emit(OP_STORE_REF);
            }
        }else{
            if(has_decorator) emit(OP_CALL, 1);
            emit(OP_STORE_CLASS_ATTR, co()->add_name(func.name, name_scope()));
        }
    }

    PyObject* read_literal(){
        if(match(TK("-"))){
            consume(TK("@num"));
            PyObject* val = get_value(prev());
            return vm->num_negated(val);
        }
        if(match(TK("@num"))) return get_value(prev());
        if(match(TK("@str"))) return get_value(prev());
        if(match(TK("True"))) return VAR(true);
        if(match(TK("False"))) return VAR(false);
        if(match(TK("None"))) return vm->None;
        if(match(TK("..."))) return vm->Ellipsis;
        return nullptr;
    }

    void SyntaxError(Str msg){ lexer->throw_err("SyntaxError", msg, curr().line, curr().start); }
    void SyntaxError(){ lexer->throw_err("SyntaxError", "invalid syntax", curr().line, curr().start); }
    void IndentationError(Str msg){ lexer->throw_err("IndentationError", msg, curr().line, curr().start); }

public:
    CodeObject_ compile(){
        if(used) UNREACHABLE();
        used = true;

        tokens = lexer->run();
        // if(lexer->src->filename == "<stdin>"){
        //     for(auto& t: tokens) std::cout << t.info() << std::endl;
        // }

        CodeObject_ code = push_context(lexer->src, lexer->src->filename);

        advance();          // skip @sof, so prev() is always valid
        match_newlines();   // skip possible leading '\n'

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }else if(mode()==JSON_MODE){
            PyObject* value = read_literal();
            if(value != nullptr) emit(OP_LOAD_CONST, code->add_const(value));
            else if(match(TK("{"))) exprMap();
            else if(match(TK("["))) exprList();
            else SyntaxError("expect a JSON object or array");
            consume(TK("@eof"));
            ctx()->emit(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }

        while (!match(TK("@eof"))) {
            if (match(TK("class"))) {
                compile_class();
            } else {
                compile_stmt();
            }
            match_newlines();
        }
        pop_context();
        return code;
    }
};

} // namespace pkpy