#pragma once

#include "codeobject.h"
#include "common.h"
#include "lexer.h"
#include "error.h"
#include "ceval.h"

namespace pkpy{

struct Expression{
    virtual Str to_string() const = 0;
};

struct NameExpr: Expression{
    Str name;
    NameScope scope;
    NameExpr(const Str& name, NameScope scope): name(name), scope(scope) {}
    NameExpr(Str&& name, NameScope scope): name(std::move(name)), scope(scope) {}
    Str to_string() const override { return name; }
};

struct UnaryExpr: Expression{
    TokenIndex op;
    Expression_ child;
    UnaryExpr(TokenIndex op, Expression_&& child): op(op), child(std::move(child)) {}
    Str to_string() const override { return TK_STR(op); }
};

struct NotExpr: Expression{
    Expression_ child;
    NotExpr(Expression_&& child): child(std::move(child)) {}
    Str to_string() const override { return "not"; }
};

struct AndExpr: Expression{
    Expression_ lhs;
    Expression_ rhs;
    AndExpr(Expression_&& lhs, Expression_&& rhs): lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    Str to_string() const override { return "and"; }
};

struct OrExpr: Expression{
    Expression_ lhs;
    Expression_ rhs;
    OrExpr(Expression_&& lhs, Expression_&& rhs): lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    Str to_string() const override { return "or"; }
};

// [None, True, False, ...]
struct SpecialLiteralExpr: Expression{
    TokenIndex token;
    SpecialLiteralExpr(TokenIndex token): token(token) {}
    Str to_string() const override { return TK_STR(token); }

    void gen(){
        // switch (token) {
        //     case TK("None"):    emit(OP_LOAD_NONE);  break;
        //     case TK("True"):    emit(OP_LOAD_TRUE);  break;
        //     case TK("False"):   emit(OP_LOAD_FALSE); break;
        //     case TK("..."):     emit(OP_LOAD_ELLIPSIS); break;
        //     default: UNREACHABLE();
        // }
    }
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expression{
    TokenValue value;
    LiteralExpr(TokenValue value): value(value) {}
    Str to_string() const override { return "literal"; }
};

struct SliceExpr: Expression{
    Expression_ start;
    Expression_ stop;
    Expression_ step;
    SliceExpr(Expression_&& start, Expression_&& stop, Expression_&& step):
        start(std::move(start)), stop(std::move(stop)), step(std::move(step)) {}
    Str to_string() const override { return "slice"; }
};

struct ListExpr: Expression{
    std::vector<Expression_> items;
    Str to_string() const override { return "[]"; }
};

struct DictExpr: Expression{
    std::vector<Expression_> items;     // each item is a DictItemExpr
    Str to_string() const override { return "{}"; }
};

struct SetExpr: Expression{
    std::vector<Expression_> items;
    Str to_string() const override { return "{}"; }
};


struct TupleExpr: Expression{
    std::vector<Expression_> items;
    TupleExpr(std::vector<Expression_>&& items): items(std::move(items)) {}
    Str to_string() const override { return "(a, b, c)"; }
};

struct CompExpr: Expression{
    Expression_ expr;       // loop expr
    Expression_ vars;       // loop vars
    Expression_ iter;       // loop iter
    Expression_ cond;       // optional if condition
    virtual void emit_expr() = 0;
};

// a:b
struct DictItemExpr: Expression{
    Expression_ key;
    Expression_ value;
    DictItemExpr(Expression_&& key, Expression_&& value)
        : key(std::move(key)), value(std::move(value)) {}
    Str to_string() const override { return "dict item"; }
};

struct ListCompExpr: CompExpr{
};

struct DictCompExpr: CompExpr{
};

struct SetCompExpr: CompExpr{
};

struct LambdaExpr: Expression{
    Function func;
    NameScope scope;
    LambdaExpr(Function&& func, NameScope scope): func(std::move(func)), scope(scope) {}
    Str to_string() const override { return "lambda"; }
};

struct FStringExpr: Expression{
    Str src;
    FStringExpr(const Str& src): src(src) {}
    Str to_string() const override { return "@fstr"; }
};

struct SubscrExpr: Expression{
    Expression_ a;
    Expression_ b;
    SubscrExpr(Expression_&& a, Expression_&& b): a(std::move(a)), b(std::move(b)) {}
    Str to_string() const override { return "a[b]"; }
};

struct AttribExpr: Expression{
    Expression_ a;
    Str b;
    AttribExpr(Expression_ a, const Str& b): a(std::move(a)), b(b) {}
    AttribExpr(Expression_ a, Str&& b): a(std::move(a)), b(std::move(b)) {}
    Str to_string() const override { return "."; }
};

struct AssignExpr: Expression{
    Expression_ lhs;
    Expression_ rhs;
    AssignExpr(Expression_&& lhs, Expression_&& rhs): lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    Str to_string() const override { return "="; }
};

struct InplaceAssignExpr: Expression{
    TokenIndex op;
    Expression_ lhs;
    Expression_ rhs;
    InplaceAssignExpr(TokenIndex op, Expression_&& lhs, Expression_&& rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    Str to_string() const override { return TK_STR(op); }
};


struct CallExpr: Expression{
    std::vector<Expression_> args;
    std::vector<std::pair<Str, Expression_>> kwargs;
    Str to_string() const override { return "()"; }
};

struct BinaryExpr: Expression{
    TokenIndex op;
    Expression_ lhs;
    Expression_ rhs;
    BinaryExpr(TokenIndex op, Expression_&& lhs, Expression_&& rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    Str to_string() const override { return TK_STR(op); }
};

struct TernaryExpr: Expression{
    Expression_ cond;
    Expression_ true_expr;
    Expression_ false_expr;
    TernaryExpr(Expression_&& cond, Expression_&& true_expr, Expression_&& false_expr)
        : cond(std::move(cond)), true_expr(std::move(true_expr)), false_expr(std::move(false_expr)) {}
    Str to_string() const override { return "?"; }
};


} // namespace pkpy