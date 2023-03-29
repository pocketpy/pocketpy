#pragma once

#include "codeobject.h"
#include "common.h"
#include "parser.h"
#include "error.h"
#include "ceval.h"
#include <memory>

namespace pkpy{

struct Expression;
typedef std::unique_ptr<Expression> Expression_;

struct Expression{
    std::vector<Expression_> children;
    virtual Str to_string() const = 0;
};

struct NameExpr: Expression{
    Str name;
    NameScope scope;
    NameExpr(Str name, NameScope scope): name(name), scope(scope) {}
    Str to_string() const override { return name; }
};

struct GroupExpr: Expression{
    Expression_ expr;
    GroupExpr(Expression_ expr): expr(std::move(expr)) {}
    Str to_string() const override { return "()"; }
};

struct UnaryExpr: Expression{
    TokenIndex op;
    UnaryExpr(TokenIndex op): op(op) {}
    Str to_string() const override { return TK_STR(op); }
};

struct NotExpr: Expression{
    Str to_string() const override { return "not"; }
};

struct AndExpr: Expression{
    Str to_string() const override { return "and"; }
};

struct OrExpr: Expression{
    Str to_string() const override { return "or"; }
};

// None, True, False, ...
struct SpecialValueExpr: Expression{
    TokenIndex token;
    SpecialValueExpr(TokenIndex token): token(token) {}
    Str to_string() const override { return TK_STR(token); }
};

// @num, @str which needs to invoke OP_LOAD_CONST
struct LiteralExpr: Expression{
    PyObject* value;
    LiteralExpr(PyObject* value): value(value) {}
    Str to_string() const override { return "literal"; }
};

struct ListExpr: Expression{
    Str to_string() const override { return "[]"; }
};

struct DictExpr: Expression{
    Str to_string() const override { return "{}"; }
};

struct LambdaExpr: Expression{
    Str to_string() const override { return "lambda"; }
};

struct FStringExpr: Expression{
    Str to_string() const override { return "@fstr"; }
};

struct AttribExpr: Expression{
    Str to_string() const override { return "."; }
};

struct CallExpr: Expression{
    Str to_string() const override { return "()"; }
};

struct BinaryExpr: Expression{
    TokenIndex op;
    BinaryExpr(TokenIndex op): op(op) {}
    Str to_string() const override { return TK_STR(op); }
};

struct TernaryExpr: Expression{
    Str to_string() const override { return "?"; }
};

struct AssignExpr: Expression{
    Str to_string() const override { return "="; }
};

struct CommaExpr: Expression{
    Str to_string() const override { return ","; }
};


} // namespace pkpy