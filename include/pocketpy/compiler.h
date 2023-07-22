#pragma once

#include "codeobject.h"
#include "common.h"
#include "expr.h"
#include "obj.h"

namespace pkpy{

class Compiler;
typedef void (Compiler::*PrattCallback)();

struct PrattRule{
    PrattCallback prefix;
    PrattCallback infix;
    Precedence precedence;
};

class Compiler: public CompilerBase {
    inline static PrattRule rules[kTokenCount];
    stack<CodeEmitContext> contexts;
    VM* vm;
    bool unknown_global_scope;     // for eval/exec() call
    bool used;

    CodeEmitContext* ctx() { return &contexts.top(); }
    CompileMode mode() const{ return lexer->src->mode; }
    NameScope name_scope() const;
    CodeObject_ push_global_context();
    FuncDecl_ push_f_context(Str name);
    void pop_context();

    static void init_pratt_rules();

    bool match_newlines_repl();
    bool match_newlines(bool repl_throw=false);
    bool match_end_stmt();
    void consume_end_stmt();

    /*************************************************/
    void EXPR(bool push_stack=true);
    void EXPR_TUPLE(bool push_stack=true);
    Expr_ EXPR_VARS();  // special case for `for loop` and `comp`

    template <typename T, typename... Args>
    std::unique_ptr<T> make_expr(Args&&... args) {
        std::unique_ptr<T> expr = std::make_unique<T>(std::forward<Args>(args)...);
        expr->line = prev().line;
        return expr;
    }

    template<typename T>
    void _consume_comp(Expr_ expr){
        static_assert(std::is_base_of<CompExpr, T>::value);
        std::unique_ptr<CompExpr> ce = make_expr<T>();
        ce->expr = std::move(expr);
        ce->vars = EXPR_VARS();
        consume(TK("in"));
        parse_expression(PREC_TERNARY + 1);
        ce->iter = ctx()->s_expr.popx();
        match_newlines_repl();
        if(match(TK("if"))){
            parse_expression(PREC_TERNARY + 1);
            ce->cond = ctx()->s_expr.popx();
        }
        ctx()->s_expr.push(std::move(ce));
        match_newlines_repl();
    }

    void exprLiteral();
    void exprLong();
    void exprBytes();
    void exprFString();
    void exprLambda();
    void exprTuple();
    void exprOr();
    void exprAnd();
    void exprTernary();
    void exprBinaryOp();
    void exprNot();
    void exprUnaryOp();
    void exprGroup();
    void exprList();
    void exprMap();
    void exprCall();
    void exprName();
    void exprAttrib();
    void exprSubscr();
    void exprLiteral0();

    void compile_block_body();
    Str _compile_import();
    void compile_normal_import();
    void compile_from_import();
    bool is_expression();
    void parse_expression(int precedence, bool push_stack=true);
    void compile_if_stmt();
    void compile_while_loop();
    void compile_for_loop();
    void compile_try_except();
    void compile_decorated();

    bool try_compile_assignment();
    void compile_stmt();
    void consume_type_hints();
    void compile_class();
    void _compile_f_args(FuncDecl_ decl, bool enable_type_hints);
    void compile_function(const std::vector<Expr_>& decorators={});

    PyObject* to_object(const TokenValue& value);
    PyObject* read_literal();
public:
    Compiler(VM* vm, const Str& source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    CodeObject_ compile();
};

} // namespace pkpy