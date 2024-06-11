#include "pocketpy/compiler/compiler.hpp"
#include "pocketpy/common/config.h"
#include "pocketpy/interpreter/vm.hpp"

#include <cstdarg>

namespace pkpy {

#define consume(expected) if(!match(expected)) return SyntaxError("expected '%s', got '%s'", TK_STR(expected), TK_STR(curr().type));
#define consume_end_stmt() if(!match_end_stmt()) return SyntaxError("expected statement end")
#define check_newlines_repl() { bool __nml; match_newlines(&__nml); if(__nml) return NeedMoreLines(); }
#define check(B) if((err = B)) return err

PrattRule Compiler::rules[kTokenCount];

NameScope Compiler::name_scope() const noexcept{
    auto s = contexts.size() > 1 ? NAME_LOCAL : NAME_GLOBAL;
    if(unknown_global_scope && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
    return s;
}

CodeObject_ Compiler::push_global_context() noexcept{
    CodeObject_ co = std::make_shared<CodeObject>(lexer.src, static_cast<const Str&>(lexer.src->filename));
    co->start_line = __i == 0 ? 1 : prev().line;
    contexts.push_back(CodeEmitContext(vm, co, contexts.size()));
    return co;
}

FuncDecl_ Compiler::push_f_context(Str name) noexcept{
    FuncDecl_ decl = std::make_shared<FuncDecl>();
    decl->code = std::make_shared<CodeObject>(lexer.src, name);
    decl->code->start_line = __i == 0 ? 1 : prev().line;
    decl->nested = name_scope() == NAME_LOCAL;
    contexts.push_back(CodeEmitContext(vm, decl->code, contexts.size()));
    contexts.back().func = decl;
    return decl;
}

Error* Compiler::pop_context() noexcept{
    assert(ctx()->s_size() == 0);
    // add a `return None` in the end as a guard
    // previously, we only do this if the last opcode is not a return
    // however, this is buggy...since there may be a jump to the end (out of bound) even if the last opcode is a return
    ctx()->emit_(OP_RETURN_VALUE, 1, BC_KEEPLINE, true);
    // find the last valid token
    int j = __i - 1;
    while(tk(j).type == TK("@eol") || tk(j).type == TK("@dedent") || tk(j).type == TK("@eof"))
        j--;
    ctx()->co->end_line = tk(j).line;

    // some check here
    auto& codes = ctx()->co->codes;
    if(ctx()->co->nlocals > PK_MAX_CO_VARNAMES) {
        return SyntaxError("maximum number of local variables exceeded");
    }
    if(ctx()->co->consts.size() > 65530) {
        return SyntaxError("maximum number of constants exceeded");
    }
    // pre-compute LOOP_BREAK and LOOP_CONTINUE
    for(int i = 0; i < codes.size(); i++) {
        Bytecode& bc = codes[i];
        if(bc.op == OP_LOOP_CONTINUE) {
            bc.set_signed_arg(ctx()->co->blocks[bc.arg].start - i);
        } else if(bc.op == OP_LOOP_BREAK) {
            bc.set_signed_arg(ctx()->co->blocks[bc.arg].get_break_end() - i);
        }
    }
    // pre-compute func->is_simple
    FuncDecl_ func = contexts.back().func;
    if(func) {
        // check generator
        for(Bytecode bc: func->code->codes) {
            if(bc.op == OP_YIELD_VALUE || bc.op == OP_FOR_ITER_YIELD_VALUE) {
                func->type = FuncType::GENERATOR;
                for(Bytecode bc: func->code->codes) {
                    if(bc.op == OP_RETURN_VALUE && bc.arg == BC_NOARG) {
                        return SyntaxError("'return' with argument inside generator function");
                    }
                }
                break;
            }
        }
        if(func->type == FuncType::UNSET) {
            bool is_simple = true;
            if(func->kwargs.size() > 0) is_simple = false;
            if(func->starred_arg >= 0) is_simple = false;
            if(func->starred_kwarg >= 0) is_simple = false;

            if(is_simple) {
                func->type = FuncType::SIMPLE;

                bool is_empty = false;
                if(func->code->codes.size() == 1) {
                    Bytecode bc = func->code->codes[0];
                    if(bc.op == OP_RETURN_VALUE && bc.arg == 1) { is_empty = true; }
                }
                if(is_empty) func->type = FuncType::EMPTY;
            } else
                func->type = FuncType::NORMAL;
        }

        assert(func->type != FuncType::UNSET);
    }
    contexts.pop_back();
    return NULL;
}

void Compiler::init_pratt_rules() noexcept{
    static bool initialized = false;
    if(initialized) return;
    initialized = true;

    // clang-format off
// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define PK_METHOD(name) &Compiler::name
#define PK_NO_INFIX nullptr, PREC_LOWEST
        for(TokenIndex i = 0; i < kTokenCount; i++) rules[i] = { nullptr, PK_NO_INFIX };
        rules[TK(".")] =        { nullptr,                  PK_METHOD(exprAttrib),         PREC_PRIMARY };
        rules[TK("(")] =        { PK_METHOD(exprGroup),     PK_METHOD(exprCall),           PREC_PRIMARY };
        rules[TK("[")] =        { PK_METHOD(exprList),      PK_METHOD(exprSubscr),         PREC_PRIMARY };
        rules[TK("{")] =        { PK_METHOD(exprMap),       PK_NO_INFIX };
        rules[TK("%")] =        { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("+")] =        { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("-")] =        { PK_METHOD(exprUnaryOp),   PK_METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("*")] =        { PK_METHOD(exprUnaryOp),   PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("~")] =        { PK_METHOD(exprUnaryOp),   nullptr,                    PREC_UNARY };
        rules[TK("/")] =        { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("//")] =       { nullptr,                  PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("**")] =       { PK_METHOD(exprUnaryOp),   PK_METHOD(exprBinaryOp),       PREC_EXPONENT };
        rules[TK(">")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("==")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("!=")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK(">=")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<=")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("in")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("is")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("<<")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
        rules[TK("@")] =        { nullptr,               PK_METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("if")] =       { nullptr,               PK_METHOD(exprTernary),        PREC_TERNARY };
        rules[TK("not in")] =   { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("is not")] =   { nullptr,               PK_METHOD(exprBinaryOp),       PREC_COMPARISION };
        rules[TK("and") ] =     { nullptr,               PK_METHOD(exprAnd),            PREC_LOGICAL_AND };
        rules[TK("or")] =       { nullptr,               PK_METHOD(exprOr),             PREC_LOGICAL_OR };
        rules[TK("not")] =      { PK_METHOD(exprNot),       nullptr,                    PREC_LOGICAL_NOT };
        rules[TK("True")] =     { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("False")] =    { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("None")] =     { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("...")] =      { PK_METHOD(exprLiteral0),  PK_NO_INFIX };
        rules[TK("lambda")] =   { PK_METHOD(exprLambda),    PK_NO_INFIX };
        rules[TK("@id")] =      { PK_METHOD(exprName),      PK_NO_INFIX };
        rules[TK("@num")] =     { PK_METHOD(exprLiteral),   PK_NO_INFIX };
        rules[TK("@str")] =     { PK_METHOD(exprLiteral),   PK_NO_INFIX };
        rules[TK("@fstr")] =    { PK_METHOD(exprFString),   PK_NO_INFIX };
        rules[TK("@long")] =    { PK_METHOD(exprLong),      PK_NO_INFIX };
        rules[TK("@imag")] =    { PK_METHOD(exprImag),      PK_NO_INFIX };
        rules[TK("@bytes")] =   { PK_METHOD(exprBytes),     PK_NO_INFIX };
        rules[TK(":")] =        { PK_METHOD(exprSlice0),    PK_METHOD(exprSlice1),      PREC_PRIMARY };
        
#undef PK_METHOD
#undef PK_NO_INFIX
    // clang-format on
}

bool Compiler::match(TokenIndex expected) noexcept{
    if(curr().type != expected) return false;
    advance();
    return true;
}

bool Compiler::match_newlines(bool* need_more_lines) noexcept{
    bool consumed = false;
    if(curr().type == TK("@eol")) {
        while(curr().type == TK("@eol")) advance();
        consumed = true;
    }
    if(need_more_lines) {
        *need_more_lines = (mode() == REPL_MODE && curr().type == TK("@eof"));
    }
    return consumed;
}

bool Compiler::match_end_stmt() noexcept{
    if(match(TK(";"))) {
        match_newlines();
        return true;
    }
    if(match_newlines() || curr().type == TK("@eof")) return true;
    if(curr().type == TK("@dedent")) return true;
    return false;
}

Error* Compiler::EXPR_TUPLE(bool allow_slice) noexcept{
    Error* err;
    check(parse_expression(PREC_LOWEST + 1, allow_slice));
    if(!match(TK(","))) return NULL;
    // tuple expression
    int count = 1;
    do {
        if(curr().brackets_level) check_newlines_repl()
        if(!is_expression(allow_slice)) break;
        check(parse_expression(PREC_LOWEST + 1, allow_slice));
        count += 1;
        if(curr().brackets_level) check_newlines_repl();
    } while(match(TK(",")));
    TupleExpr* e = make_expr<TupleExpr>(count);
    for(int i=count-1; i>=0; i--)
        e->items[i] = ctx()->s_popx();
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::EXPR_VARS() noexcept{
    int count = 0;
    do {
        consume(TK("@id"));
        ctx()->s_push(make_expr<NameExpr>(prev().str(), name_scope()));
        count += 1;
    } while(match(TK(",")));
    if(count > 1){
        TupleExpr* e = make_expr<TupleExpr>(count);
        for(int i=count-1; i>=0; i--)
            e->items[i] = ctx()->s_popx();
        ctx()->s_push(e);
    }
    return NULL;
}

Error* Compiler::exprLiteral() noexcept{
    ctx()->s_push(make_expr<LiteralExpr>(prev().value));
    return NULL;
}

Error* Compiler::exprLong() noexcept{
    ctx()->s_push(make_expr<LongExpr>(prev().str()));
    return NULL;
}

Error* Compiler::exprImag() noexcept{
    ctx()->s_push(make_expr<ImagExpr>(std::get<f64>(prev().value)));
    return NULL;
}

Error* Compiler::exprBytes() noexcept{
    ctx()->s_push(make_expr<BytesExpr>(std::get<Str>(prev().value)));
    return NULL;
}

Error* Compiler::exprFString() noexcept{
    ctx()->s_push(make_expr<FStringExpr>(std::get<Str>(prev().value)));
    return NULL;
}

Error* Compiler::exprLambda() noexcept{
    Error* err;
    FuncDecl_ decl = push_f_context("<lambda>");
    int line = prev().line;     // backup line
    if(!match(TK(":"))) {
        check(_compile_f_args(decl, false));
        consume(TK(":"));
    }
    // https://github.com/pocketpy/pocketpy/issues/37
    check(parse_expression(PREC_LAMBDA + 1));
    ctx()->s_emit_top();
    ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
    check(pop_context());
    LambdaExpr* e = make_expr<LambdaExpr>(decl);
    e->line = line;
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprOr() noexcept{
    int line = prev().line;
    Error* err;
    check(parse_expression(PREC_LOGICAL_OR + 1));
    auto e = make_expr<OrExpr>();
    e->line = line;
    e->rhs = ctx()->s_popx();
    e->lhs = ctx()->s_popx();
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprAnd() noexcept{
    int line = prev().line;
    Error* err;
    check(parse_expression(PREC_LOGICAL_AND + 1));
    auto e = make_expr<AndExpr>();
    e->line = line;
    e->rhs = ctx()->s_popx();
    e->lhs = ctx()->s_popx();
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprTernary() noexcept{
    // [true_expr]
    Error* err;
    int line = prev().line;
    check(parse_expression(PREC_TERNARY + 1));  // [true_expr, cond]
    consume(TK("else"));
    check(parse_expression(PREC_TERNARY + 1));  // [true_expr, cond, false_expr]
    auto e = make_expr<TernaryExpr>();
    e->line = line;
    e->false_expr = ctx()->s_popx();
    e->cond = ctx()->s_popx();
    e->true_expr = ctx()->s_popx();
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprBinaryOp() noexcept{
    Error* err;
    int line = prev().line;
    TokenIndex op = prev().type;
    check(parse_expression(rules[op].precedence + 1));
    BinaryExpr* e = make_expr<BinaryExpr>(op);
    e->line = line;
    e->rhs = ctx()->s_popx();
    e->lhs = ctx()->s_popx();
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprNot() noexcept{
    Error* err;
    check(parse_expression(PREC_LOGICAL_NOT + 1));
    NotExpr* e = make_expr<NotExpr>(ctx()->s_popx());
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprUnaryOp() noexcept{
    Error* err;
    TokenIndex op = prev().type;
    check(parse_expression(PREC_UNARY + 1));
    switch(op) {
        case TK("-"): ctx()->s_push(make_expr<NegatedExpr>(ctx()->s_popx())); break;
        case TK("~"): ctx()->s_push(make_expr<InvertExpr>(ctx()->s_popx())); break;
        case TK("*"): ctx()->s_push(make_expr<StarredExpr>(ctx()->s_popx(), 1)); break;
        case TK("**"): ctx()->s_push(make_expr<StarredExpr>(ctx()->s_popx(), 2)); break;
        default: assert(false);
    }
    return NULL;
}

Error* Compiler::exprGroup() noexcept{
    Error* err;
    check_newlines_repl()
    check(EXPR_TUPLE());  // () is just for change precedence
    check_newlines_repl()
    consume(TK(")"));
    if(ctx()->s_top()->is_tuple()) return NULL;
    Expr* g = make_expr<GroupedExpr>(ctx()->s_popx());
    ctx()->s_push(g);
    return NULL;
}

Error* Compiler::consume_comp(Opcode op0, Opcode op1) noexcept{
    // [expr]
    Error* err;
    bool has_cond = false;
    check(EXPR_VARS());                         // [expr, vars]
    consume(TK("in"));
    check(parse_expression(PREC_TERNARY + 1));  // [expr, vars, iter]
    check_newlines_repl()
    if(match(TK("if"))) {
        check(parse_expression(PREC_TERNARY + 1));  // [expr, vars, iter, cond]
        has_cond = true;
    }
    CompExpr* ce = make_expr<CompExpr>(op0, op1);
    if(has_cond) ce->cond = ctx()->s_popx();
    ce->iter = ctx()->s_popx();
    ce->vars = ctx()->s_popx();
    ce->expr = ctx()->s_popx();
    ctx()->s_push(ce);
    check_newlines_repl()
    return NULL;
}

Error* Compiler::exprList() noexcept{
    Error* err;
    int line = prev().line;
    int count = 0;
    do {
        check_newlines_repl()
        if(curr().type == TK("]")) break;
        check(EXPR()); count += 1;
        check_newlines_repl()
        if(count == 1 && match(TK("for"))) {
            check(consume_comp(OP_BUILD_LIST, OP_LIST_APPEND));
            consume(TK("]"));
            return NULL;
        }
        check_newlines_repl()
    } while(match(TK(",")));
    consume(TK("]"));
    ListExpr* e = make_expr<ListExpr>(count);
    e->line = line;  // override line
    for(int i=count-1; i>=0; i--)
        e->items[i] = ctx()->s_popx();
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprMap() noexcept{
    Error* err;
    bool parsing_dict = false;  // {...} may be dict or set
    int count = 0;
    do {
        check_newlines_repl()
        if(curr().type == TK("}")) break;
        check(EXPR());  // [key]
        int star_level = ctx()->s_top()->star_level();
        if(star_level == 2 || curr().type == TK(":")) { parsing_dict = true; }
        if(parsing_dict) {
            if(star_level == 2) {
                DictItemExpr* dict_item = make_expr<DictItemExpr>();
                dict_item->key = NULL;
                dict_item->value = ctx()->s_popx();
                ctx()->s_push(dict_item);
            } else {
                consume(TK(":"));
                check(EXPR());
                DictItemExpr* dict_item = make_expr<DictItemExpr>();
                dict_item->value = ctx()->s_popx();
                dict_item->key = ctx()->s_popx();
                ctx()->s_push(dict_item);
            }
        }
        count += 1;
        check_newlines_repl()
        if(count == 1 && match(TK("for"))) {
            if(parsing_dict){
                check(consume_comp(OP_BUILD_DICT, OP_DICT_ADD));
            }else{
                check(consume_comp(OP_BUILD_SET, OP_SET_ADD));
            }
            consume(TK("}"));
            return NULL;
        }
        check_newlines_repl()
    } while(match(TK(",")));
    consume(TK("}"));

    SequenceExpr* se;
    if(count == 0 || parsing_dict) {
        se = make_expr<DictExpr>(count);
    } else {
        se = make_expr<SetExpr>(count);
    }
    for(int i=count-1; i>=0; i--)
        se->items[i] = ctx()->s_popx();
    ctx()->s_push(se);
    return NULL;
}

Error* Compiler::exprCall() noexcept{
    Error* err;
    CallExpr* e = make_expr<CallExpr>();
    e->callable = ctx()->s_popx();
    ctx()->s_push(e);     // push onto the stack in advance
    do {
        check_newlines_repl()
        if(curr().type == TK(")")) break;
        if(curr().type == TK("@id") && next().type == TK("=")) {
            consume(TK("@id"));
            StrName key(prev().sv());
            consume(TK("="));
            check(EXPR());
            e->kwargs.push_back({key, ctx()->s_popx()});
        } else {
            check(EXPR());
            if(ctx()->s_top()->star_level() == 2) {
                // **kwargs
                e->kwargs.push_back({"**", ctx()->s_popx()});
            } else {
                // positional argument
                if(!e->kwargs.empty()) return SyntaxError("positional argument follows keyword argument");
                e->args.push_back(ctx()->s_popx());
            }
        }
        check_newlines_repl()
    } while(match(TK(",")));
    consume(TK(")"));
    return NULL;
}

Error* Compiler::exprName() noexcept{
    StrName name(prev().sv());
    NameScope scope = name_scope();
    if(ctx()->global_names.contains(name)) { scope = NAME_GLOBAL; }
    ctx()->s_push(make_expr<NameExpr>(name, scope));
    return NULL;
}

Error* Compiler::exprAttrib() noexcept{
    consume(TK("@id"));
    ctx()->s_push(make_expr<AttribExpr>(ctx()->s_popx(), StrName::get(prev().sv())));
    return NULL;
}

Error* Compiler::exprSlice0() noexcept{
    Error* err;
    SliceExpr* slice = make_expr<SliceExpr>();
    ctx()->s_push(slice);     // push onto the stack in advance
    if(is_expression()) {  // :<stop>
        check(EXPR());
        slice->stop = ctx()->s_popx();
        // try optional step
        if(match(TK(":"))) {  // :<stop>:<step>
            check(EXPR());
            slice->step = ctx()->s_popx();
        }
    } else if(match(TK(":"))) {
        if(is_expression()) {  // ::<step>
            check(EXPR());
            slice->step = ctx()->s_popx();
        }  // else ::
    }  // else :
    return NULL;
}

Error* Compiler::exprSlice1() noexcept{
    Error* err;
    SliceExpr* slice = make_expr<SliceExpr>();
    slice->start = ctx()->s_popx();
    ctx()->s_push(slice);     // push onto the stack in advance
    if(is_expression()) {  // <start>:<stop>
        check(EXPR());
        slice->stop = ctx()->s_popx();
        // try optional step
        if(match(TK(":"))) {  // <start>:<stop>:<step>
            check(EXPR());
            slice->step = ctx()->s_popx();
        }
    } else if(match(TK(":"))) {  // <start>::<step>
        check(EXPR());
        slice->step = ctx()->s_popx();
    }  // else <start>:
    return NULL;
}

Error* Compiler::exprSubscr() noexcept{
    Error* err;
    int line = prev().line;
    check_newlines_repl()
    check(EXPR_TUPLE(true));
    check_newlines_repl()
    consume(TK("]"));           // [lhs, rhs]
    SubscrExpr* e = make_expr<SubscrExpr>();
    e->line = line;
    e->rhs = ctx()->s_popx();   // [lhs]
    e->lhs = ctx()->s_popx();   // []
    ctx()->s_push(e);
    return NULL;
}

Error* Compiler::exprLiteral0() noexcept{
    ctx()->s_push(make_expr<Literal0Expr>(prev().type));
    return NULL;
}

Error* Compiler::compile_block_body(PrattCallback callback) noexcept{
    Error* err;
    if(!callback) callback = &Compiler::compile_stmt;
    consume(TK(":"));
    if(curr().type != TK("@eol") && curr().type != TK("@eof")) {
        while(true) {
            check(compile_stmt());
            bool possible = curr().type != TK("@eol") && curr().type != TK("@eof");
            if(prev().type != TK(";") || !possible) break;
        }
        return NULL;
    }

    bool need_more_lines;
    bool consumed = match_newlines(&need_more_lines);
    if(need_more_lines) return NeedMoreLines();
    if(!consumed) return SyntaxError("expected a new line after ':'");

    consume(TK("@indent"));
    while(curr().type != TK("@dedent")) {
        match_newlines();
        check((this->*callback)());
        match_newlines();
    }
    consume(TK("@dedent"));
    return NULL;
}

// import a [as b]
// import a [as b], c [as d]
Error* Compiler::compile_normal_import() noexcept{
    do {
        consume(TK("@id"));
        Str name = prev().str();
        ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const_string(name.sv()), prev().line);
        if(match(TK("as"))) {
            consume(TK("@id"));
            name = prev().str();
        }
        ctx()->emit_store_name(name_scope(), StrName(name), prev().line);
    } while(match(TK(",")));
    consume_end_stmt();
    return NULL;
}

// from a import b [as c], d [as e]
// from a.b import c [as d]
// from . import a [as b]
// from .a import b [as c]
// from ..a import b [as c]
// from .a.b import c [as d]
// from xxx import *
Error* Compiler::compile_from_import() noexcept{
    int dots = 0;

    while(true) {
        switch(curr().type) {
            case TK("."): dots += 1; break;
            case TK(".."): dots += 2; break;
            case TK("..."): dots += 3; break;
            default: goto __EAT_DOTS_END;
        }
        advance();
    }
__EAT_DOTS_END:
    SStream ss;
    for(int i = 0; i < dots; i++)
        ss << '.';

    if(dots > 0) {
        // @id is optional if dots > 0
        if(match(TK("@id"))) {
            ss << prev().sv();
            while(match(TK("."))) {
                consume(TK("@id"));
                ss << "." << prev().sv();
            }
        }
    } else {
        // @id is required if dots == 0
        consume(TK("@id"));
        ss << prev().sv();
        while(match(TK("."))) {
            consume(TK("@id"));
            ss << "." << prev().sv();
        }
    }

    ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const_string(ss.str().sv()), prev().line);
    consume(TK("import"));

    if(match(TK("*"))) {
        if(name_scope() != NAME_GLOBAL) return SyntaxError("from <module> import * can only be used in global scope");
        // pop the module and import __all__
        ctx()->emit_(OP_POP_IMPORT_STAR, BC_NOARG, prev().line);
        consume_end_stmt();
        return NULL;
    }

    do {
        ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
        consume(TK("@id"));
        Str name = prev().str();
        ctx()->emit_(OP_LOAD_ATTR, StrName(name).index, prev().line);
        if(match(TK("as"))) {
            consume(TK("@id"));
            name = prev().str();
        }
        ctx()->emit_store_name(name_scope(), StrName(name), prev().line);
    } while(match(TK(",")));
    ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
    consume_end_stmt();
    return NULL;
}

bool Compiler::is_expression(bool allow_slice) noexcept{
    PrattCallback prefix = rules[curr().type].prefix;
    return prefix != nullptr && (allow_slice || curr().type != TK(":"));
}

Error* Compiler::parse_expression(int precedence, bool allow_slice) noexcept{
    PrattCallback prefix = rules[curr().type].prefix;
    if(prefix == nullptr || (curr().type == TK(":") && !allow_slice)) {
        return SyntaxError("expected an expression, got %s", TK_STR(curr().type));
    }
    advance();
    Error* err;
    check((this->*prefix)());
    while(rules[curr().type].precedence >= precedence && (allow_slice || curr().type != TK(":"))) {
        TokenIndex op = curr().type;
        advance();
        PrattCallback infix = rules[op].infix;
        assert(infix != nullptr);
        check((this->*infix)());
    }
    return NULL;
}

Error* Compiler::compile_if_stmt() noexcept{
    Error* err;
    check(EXPR());  // condition
    ctx()->s_emit_top();
    int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
    err = compile_block_body();
    if(err) return err;
    if(match(TK("elif"))) {
        int exit_patch = ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, prev().line);
        ctx()->patch_jump(patch);
        check(compile_if_stmt());
        ctx()->patch_jump(exit_patch);
    } else if(match(TK("else"))) {
        int exit_patch = ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, prev().line);
        ctx()->patch_jump(patch);
        check(compile_block_body());
        ctx()->patch_jump(exit_patch);
    } else {
        ctx()->patch_jump(patch);
    }
    return NULL;
}

Error* Compiler::compile_while_loop() noexcept{
    Error* err;
    CodeBlock* block = ctx()->enter_block(CodeBlockType::WHILE_LOOP);
    check(EXPR());  // condition
    ctx()->s_emit_top();
    int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
    check(compile_block_body());
    ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE, true);
    ctx()->patch_jump(patch);
    ctx()->exit_block();
    // optional else clause
    if(match(TK("else"))) {
        check(compile_block_body());
        block->end2 = ctx()->co->codes.size();
    }
    return NULL;
}

Error* Compiler::compile_for_loop() noexcept{
    Error* err;
    check(EXPR_VARS());     // [vars]
    consume(TK("in"));
    check(EXPR_TUPLE());    // [vars, iter]
    ctx()->s_emit_top();     // [vars]
    ctx()->emit_(OP_GET_ITER_NEW, BC_NOARG, BC_KEEPLINE);
    CodeBlock* block = ctx()->enter_block(CodeBlockType::FOR_LOOP);
    int for_codei = ctx()->emit_(OP_FOR_ITER, ctx()->curr_iblock, BC_KEEPLINE);
    Expr* vars = ctx()->s_popx();
    bool ok = vars->emit_store(ctx());
    delete_expr(vars);
    if(!ok) return SyntaxError();  // this error occurs in `vars` instead of this line, but...nevermind
    ctx()->try_merge_for_iter_store(for_codei);
    check(compile_block_body());
    ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE, true);
    ctx()->exit_block();
    // optional else clause
    if(match(TK("else"))) {
        check(compile_block_body());
        block->end2 = ctx()->co->codes.size();
    }
    return NULL;
}

Error* Compiler::compile_try_except() noexcept{
    Error* err;
    ctx()->enter_block(CodeBlockType::TRY_EXCEPT);
    ctx()->emit_(OP_TRY_ENTER, BC_NOARG, prev().line);
    check(compile_block_body());
    small_vector_2<int, 8> patches;
    patches.push_back(ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE));
    ctx()->exit_block();

    int finally_entry = -1;
    if(curr().type != TK("finally")) {
        do {
            StrName as_name;
            consume(TK("except"));
            if(is_expression()) {
                check(EXPR());  // push assumed type on to the stack
                ctx()->s_emit_top();
                ctx()->emit_(OP_EXCEPTION_MATCH, BC_NOARG, prev().line);
                if(match(TK("as"))) {
                    consume(TK("@id"));
                    as_name = StrName(prev().sv());
                }
            } else {
                ctx()->emit_(OP_LOAD_TRUE, BC_NOARG, BC_KEEPLINE);
            }
            int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
            // on match
            if(!as_name.empty()) {
                ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
                ctx()->emit_store_name(name_scope(), as_name, BC_KEEPLINE);
            }
            // pop the exception
            ctx()->emit_(OP_POP_EXCEPTION, BC_NOARG, BC_KEEPLINE);
            check(compile_block_body());
            patches.push_back(ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE));
            ctx()->patch_jump(patch);
        } while(curr().type == TK("except"));
    }

    if(match(TK("finally"))) {
        int patch = ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE);
        finally_entry = ctx()->co->codes.size();
        check(compile_block_body());
        ctx()->emit_(OP_JUMP_ABSOLUTE_TOP, BC_NOARG, BC_KEEPLINE);
        ctx()->patch_jump(patch);
    }
    // no match, re-raise
    if(finally_entry != -1) {
        i64 target = ctx()->co->codes.size() + 2;
        ctx()->emit_(OP_LOAD_CONST, ctx()->add_const(VAR(target)), BC_KEEPLINE);
        int i = ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE);
        ctx()->co->codes[i].set_signed_arg(finally_entry - i);
    }
    ctx()->emit_(OP_RE_RAISE, BC_NOARG, BC_KEEPLINE);

    // no exception or no match, jump to the end
    for(int patch: patches)
        ctx()->patch_jump(patch);
    if(finally_entry != -1) {
        i64 target = ctx()->co->codes.size() + 2;
        ctx()->emit_(OP_LOAD_CONST, ctx()->add_const(VAR(target)), BC_KEEPLINE);
        int i = ctx()->emit_(OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE);
        ctx()->co->codes[i].set_signed_arg(finally_entry - i);
    }
    return NULL;
}

Error* Compiler::compile_decorated() noexcept{
    Error* err;
    int count = 0;
    do {
        check(EXPR());
        count += 1;
        bool need_more_lines;
        bool consumed = match_newlines(&need_more_lines);
        if(need_more_lines) return NeedMoreLines();
        if(!consumed) return SyntaxError("expected a newline after '@'");
    } while(match(TK("@")));

    if(match(TK("class"))) {
        check(compile_class(count));
    } else {
        consume(TK("def"));
        check(compile_function(count));
    }
    return NULL;
}

Error* Compiler::try_compile_assignment(bool* is_assign) noexcept{
    Error* err;
    switch(curr().type) {
        case TK("+="):
        case TK("-="):
        case TK("*="):
        case TK("/="):
        case TK("//="):
        case TK("%="):
        case TK("<<="):
        case TK(">>="):
        case TK("&="):
        case TK("|="):
        case TK("^="): {
            if(ctx()->s_top()->is_starred()) return SyntaxError();
            if(ctx()->is_compiling_class){
                return SyntaxError("can't use inplace operator in class definition");
            }
            advance();
            // a[x] += 1;   a and x should be evaluated only once
            // a.x += 1;    a should be evaluated only once
            // -1 to remove =; inplace=true
            int line = prev().line;
            TokenIndex op = prev().type-1;
            // [lhs]
            check(EXPR_TUPLE());        // [lhs, rhs]
            if(ctx()->s_top()->is_starred()) return SyntaxError();
            BinaryExpr* e = make_expr<BinaryExpr>(op, true);
            e->line = line;
            e->rhs = ctx()->s_popx();   // [lhs]
            e->lhs = ctx()->s_popx();   // []
            e->emit_(ctx());
            bool ok = e->lhs->emit_store_inplace(ctx());
            delete_expr(e);
            if(!ok) return SyntaxError();
            *is_assign = true;
            return NULL;
        }
        case TK("="): {
            int n = 0;
            while(match(TK("="))) {
                check(EXPR_TUPLE());
                n += 1;
            }
            // stack size is n+1
            ctx()->s_emit_top();     // emit and pop
            for(int j = 1; j < n; j++)
                ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
            for(int j = 0; j < n; j++) {
                if(ctx()->s_top()->is_starred()) return SyntaxError();
                bool ok = ctx()->s_top()->emit_store(ctx());
                ctx()->s_pop();
                if(!ok) return SyntaxError();
            }
            *is_assign = true;
            return NULL;
        }
        default: *is_assign = false;
    }
    return NULL;
}

Error* Compiler::compile_stmt() noexcept{
    Error* err;
    if(match(TK("class"))) {
        check(compile_class());
        return NULL;
    }
    advance();
    int kw_line = prev().line;  // backup line number
    int curr_loop_block = ctx()->get_loop();
    switch(prev().type) {
        case TK("break"):
            if(curr_loop_block < 0) return SyntaxError("'break' outside loop");
            ctx()->emit_(OP_LOOP_BREAK, curr_loop_block, kw_line);
            consume_end_stmt();
            break;
        case TK("continue"):
            if(curr_loop_block < 0) return SyntaxError("'continue' not properly in loop");
            ctx()->emit_(OP_LOOP_CONTINUE, curr_loop_block, kw_line);
            consume_end_stmt();
            break;
        case TK("yield"):
            if(contexts.size() <= 1) return SyntaxError("'yield' outside function");
            check(EXPR_TUPLE());
            ctx()->s_emit_top();
            ctx()->emit_(OP_YIELD_VALUE, BC_NOARG, kw_line);
            consume_end_stmt();
            break;
        case TK("yield from"):
            if(contexts.size() <= 1) return SyntaxError("'yield from' outside function");
            check(EXPR_TUPLE());
            ctx()->s_emit_top();

            ctx()->emit_(OP_GET_ITER_NEW, BC_NOARG, kw_line);
            ctx()->enter_block(CodeBlockType::FOR_LOOP);
            ctx()->emit_(OP_FOR_ITER_YIELD_VALUE, BC_NOARG, kw_line);
            ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), kw_line);
            ctx()->exit_block();
            consume_end_stmt();
            break;
        case TK("return"):
            if(contexts.size() <= 1) return SyntaxError("'return' outside function");
            if(match_end_stmt()) {
                ctx()->emit_(OP_RETURN_VALUE, 1, kw_line);
            } else {
                check(EXPR_TUPLE());
                ctx()->s_emit_top();
                consume_end_stmt();
                ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, kw_line);
            }
            break;
        /*************************************************/
        case TK("if"): check(compile_if_stmt()); break;
        case TK("while"): check(compile_while_loop()); break;
        case TK("for"): check(compile_for_loop()); break;
        case TK("import"): check(compile_normal_import()); break;
        case TK("from"): check(compile_from_import()); break;
        case TK("def"): check(compile_function()); break;
        case TK("@"): check(compile_decorated()); break;
        case TK("try"): check(compile_try_except()); break;
        case TK("pass"): consume_end_stmt(); break;
        /*************************************************/
        case TK("assert"): {
            check(EXPR());  // condition
            ctx()->s_emit_top();
            int index = ctx()->emit_(OP_POP_JUMP_IF_TRUE, BC_NOARG, kw_line);
            int has_msg = 0;
            if(match(TK(","))) {
                check(EXPR());  // message
                ctx()->s_emit_top();
                has_msg = 1;
            }
            ctx()->emit_(OP_RAISE_ASSERT, has_msg, kw_line);
            ctx()->patch_jump(index);
            consume_end_stmt();
            break;
        }
        case TK("global"):
            do {
                consume(TK("@id"));
                ctx()->global_names.push_back(StrName(prev().sv()));
            } while(match(TK(",")));
            consume_end_stmt();
            break;
        case TK("raise"): {
            check(EXPR());
            ctx()->s_emit_top();
            ctx()->emit_(OP_RAISE, BC_NOARG, kw_line);
            consume_end_stmt();
        } break;
        case TK("del"): {
            check(EXPR_TUPLE());
            if(!ctx()->s_top()->emit_del(ctx())) return SyntaxError();
            ctx()->s_pop();
            consume_end_stmt();
        } break;
        case TK("with"): {
            check(EXPR());  // [ <expr> ]
            ctx()->s_emit_top();
            ctx()->enter_block(CodeBlockType::CONTEXT_MANAGER);
            Expr* as_name = nullptr;
            if(match(TK("as"))) {
                consume(TK("@id"));
                as_name = make_expr<NameExpr>(prev().str(), name_scope());
            }
            ctx()->emit_(OP_WITH_ENTER, BC_NOARG, prev().line);
            // [ <expr> <expr>.__enter__() ]
            if(as_name) {
                bool ok = as_name->emit_store(ctx());
                delete_expr(as_name);
                if(!ok) return SyntaxError();
            } else {
                ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
            }
            check(compile_block_body());
            ctx()->emit_(OP_WITH_EXIT, BC_NOARG, prev().line);
            ctx()->exit_block();
        } break;
        /*************************************************/
        case TK("=="): {
            consume(TK("@id"));
            if(mode() != EXEC_MODE) return SyntaxError("'label' is only available in EXEC_MODE");
            if(!ctx()->add_label(prev().str())) {
                Str escaped(prev().str().escape());
                return SyntaxError("label %s already exists", escaped.c_str());
            }
            consume(TK("=="));
            consume_end_stmt();
        } break;
        case TK("->"):
            consume(TK("@id"));
            if(mode() != EXEC_MODE) return SyntaxError("'goto' is only available in EXEC_MODE");
            ctx()->emit_(OP_GOTO, StrName(prev().sv()).index, prev().line);
            consume_end_stmt();
            break;
        /*************************************************/
        // handle dangling expression or assignment
        default: {
            advance(-1);  // do revert since we have pre-called advance() at the beginning
            check(EXPR_TUPLE());

            bool is_typed_name = false;  // e.g. x: int
            // eat variable's type hint if it is a single name
            if(ctx()->s_top()->is_name()) {
                if(match(TK(":"))) {
                    check(consume_type_hints());
                    is_typed_name = true;

                    if(ctx()->is_compiling_class) {
                        NameExpr* ne = static_cast<NameExpr*>(ctx()->s_top());
                        ctx()->emit_(OP_ADD_CLASS_ANNOTATION, ne->name.index, BC_KEEPLINE);
                    }
                }
            }
            bool is_assign = false;
            check(try_compile_assignment(&is_assign));
            if(!is_assign) {
                if(ctx()->s_size() > 0 && ctx()->s_top()->is_starred()) { 
                    return SyntaxError();
                }
                if(!is_typed_name) {
                    ctx()->s_emit_top();
                    if((mode() == CELL_MODE || mode() == REPL_MODE) && name_scope() == NAME_GLOBAL) {
                        ctx()->emit_(OP_PRINT_EXPR, BC_NOARG, BC_KEEPLINE);
                    } else {
                        ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                    }
                } else {
                    ctx()->s_pop();
                }
            }
            consume_end_stmt();
            break;
        }
    }
    return NULL;
}

Error* Compiler::consume_type_hints() noexcept{
    Error* err;
    check(EXPR());
    ctx()->s_pop();
    return NULL;
}

Error* Compiler::compile_class(int decorators) noexcept{
    Error* err;
    consume(TK("@id"));
    int namei = StrName(prev().sv()).index;
    bool has_base = false;
    if(match(TK("("))) {
        if(is_expression()) {
            check(EXPR());
            has_base = true;    // [base]
        }
        consume(TK(")"));
    }
    if(!has_base) {
        ctx()->emit_(OP_LOAD_NONE, BC_NOARG, prev().line);
    } else {
        ctx()->s_emit_top();    // []
    }
    ctx()->emit_(OP_BEGIN_CLASS, namei, BC_KEEPLINE);

    for(auto& c: this->contexts) {
        if(c.is_compiling_class) return SyntaxError("nested class is not allowed");
    }
    ctx()->is_compiling_class = true;
    check(compile_block_body());
    ctx()->is_compiling_class = false;

    if(decorators > 0) {
        ctx()->emit_(OP_BEGIN_CLASS_DECORATION, BC_NOARG, BC_KEEPLINE);
        ctx()->s_emit_decorators(decorators);
        ctx()->emit_(OP_END_CLASS_DECORATION, BC_NOARG, BC_KEEPLINE);
    }

    ctx()->emit_(OP_END_CLASS, namei, BC_KEEPLINE);
    return NULL;
}

Error* Compiler::_compile_f_args(FuncDecl_ decl, bool enable_type_hints) noexcept{
    int state = 0;  // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
    Error* err;
    do {
        if(state > 3) return SyntaxError();
        if(state == 3) return SyntaxError("**kwargs should be the last argument");
        match_newlines();
        if(match(TK("*"))) {
            if(state < 1)
                state = 1;
            else
                return SyntaxError("*args should be placed before **kwargs");
        } else if(match(TK("**"))) {
            state = 3;
        }
        consume(TK("@id"));
        StrName name(prev().sv());

        // check duplicate argument name
        for(int j: decl->args) {
            if(decl->code->varnames[j] == name) return SyntaxError("duplicate argument name");
        }
        for(auto& kv: decl->kwargs) {
            if(decl->code->varnames[kv.index] == name) return SyntaxError("duplicate argument name");
        }
        if(decl->starred_arg != -1 && decl->code->varnames[decl->starred_arg] == name) {
            return SyntaxError("duplicate argument name");
        }
        if(decl->starred_kwarg != -1 && decl->code->varnames[decl->starred_kwarg] == name) {
            return SyntaxError("duplicate argument name");
        }

        // eat type hints
        if(enable_type_hints && match(TK(":"))) check(consume_type_hints());
        if(state == 0 && curr().type == TK("=")) state = 2;
        int index = ctx()->add_varname(name);
        switch(state) {
            case 0: decl->args.push_back(index); break;
            case 1:
                decl->starred_arg = index;
                state += 1;
                break;
            case 2: {
                consume(TK("="));
                PyVar value;
                check(read_literal(&value));
                if(value == nullptr) return SyntaxError("default argument must be a literal");
                decl->add_kwarg(index, name, value);
            } break;
            case 3:
                decl->starred_kwarg = index;
                state += 1;
                break;
        }
    } while(match(TK(",")));
    return NULL;
}

Error* Compiler::compile_function(int decorators) noexcept{
    Error* err;
    consume(TK("@id"));
    Str decl_name = prev().str();
    FuncDecl_ decl = push_f_context(decl_name);
    consume(TK("("));
    if(!match(TK(")"))) {
        check(_compile_f_args(decl, true));
        consume(TK(")"));
    }
    if(match(TK("->"))) check(consume_type_hints());
    check(compile_block_body());
    check(pop_context());

    decl->docstring = nullptr;
    if(decl->code->codes.size() >= 2 && decl->code->codes[0].op == OP_LOAD_CONST &&
       decl->code->codes[1].op == OP_POP_TOP) {
        PyVar c = decl->code->consts[decl->code->codes[0].arg];
        if(is_type(c, vm->tp_str)) {
            decl->code->codes[0].op = OP_NO_OP;
            decl->code->codes[1].op = OP_NO_OP;
            decl->docstring = PK_OBJ_GET(Str, c).c_str();
        }
    }
    ctx()->emit_(OP_LOAD_FUNCTION, ctx()->add_func_decl(decl), prev().line);

    ctx()->s_emit_decorators(decorators);

    if(!ctx()->is_compiling_class) {
        NameExpr* e = make_expr<NameExpr>(decl_name, name_scope());
        e->emit_store(ctx());
        delete_expr(e);
    } else {
        int index = StrName(decl_name).index;
        ctx()->emit_(OP_STORE_CLASS_ATTR, index, prev().line);
    }
    return NULL;
}

PyVar Compiler::to_object(const TokenValue& value) noexcept{
    PyVar obj = nullptr;
    if(std::holds_alternative<i64>(value)) { obj = VAR(std::get<i64>(value)); }
    if(std::holds_alternative<f64>(value)) { obj = VAR(std::get<f64>(value)); }
    if(std::holds_alternative<Str>(value)) { obj = VAR(std::get<Str>(value)); }
    assert(obj != nullptr);
    return obj;
}

Error* Compiler::read_literal(PyVar* out) noexcept{
    Error* err;
    advance();
    switch(prev().type) {
        case TK("-"): {
            consume(TK("@num"));
            PyVar val = to_object(prev().value);
            *out = vm->py_negate(val);
            return NULL;
        }
        case TK("@num"): *out = to_object(prev().value); return NULL;
        case TK("@str"): *out = to_object(prev().value); return NULL;
        case TK("True"): *out = VAR(true);  return NULL;
        case TK("False"): *out = VAR(false); return NULL;
        case TK("None"): *out = vm->None; return NULL;
        case TK("..."): *out = vm->Ellipsis; return NULL;
        case TK("("): {
            List cpnts;
            while(true) {
                PyVar elem;
                check(read_literal(&elem));
                cpnts.push_back(elem);
                if(curr().type == TK(")")) break;
                consume(TK(","));
                if(curr().type == TK(")")) break;
            }
            consume(TK(")"));
            *out = VAR(cpnts.to_tuple());
            return NULL;
        }
        default: *out = nullptr; return NULL;
    }
}

Compiler::Compiler(VM* vm, std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope) noexcept:
    lexer(vm, std::make_shared<SourceData>(source, filename, mode)){
    this->vm = vm;
    this->unknown_global_scope = unknown_global_scope;
    init_pratt_rules();
}

Error* Compiler::compile(CodeObject_* out) noexcept{
    assert(__i == 0);  // make sure it is the first time to compile

    Error* err;
    check(lexer.run());

    // if(lexer.src.filename()[0] != '<'){
    //     printf("%s\n", lexer.src.filename().c_str());
    //     for(int i=0; i<lexer.nexts.size(); i++){
    //         printf("%s: %s\n", TK_STR(tk(i).type), tk(i).str().escape().c_str());
    //     }
    // }

    CodeObject_ code = push_global_context();

    assert(curr().type == TK("@sof"));
    advance();         // skip @sof, so prev() is always valid
    match_newlines();  // skip possible leading '\n'

    if(mode() == EVAL_MODE) {
        check(EXPR_TUPLE());
        ctx()->s_emit_top();
        consume(TK("@eof"));
        ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        check(pop_context());
        *out = code;
        return NULL;
    } else if(mode() == JSON_MODE) {
        check(EXPR());
        Expr* e = ctx()->s_popx();
        if(!e->is_json_object()) return SyntaxError("expect a JSON object, literal or array");
        consume(TK("@eof"));
        e->emit_(ctx());
        ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        check(pop_context());
        *out = code;
        return NULL;
    }

    while(!match(TK("@eof"))) {
        check(compile_stmt());
        match_newlines();
    }
    check(pop_context());
    *out = code;
    return NULL;
}

Compiler::~Compiler(){
    for(CodeEmitContext& ctx: contexts){
        ctx.s_clean();
    }
}

Error* Compiler::SyntaxError(const char* msg, ...) noexcept{
    va_list args;
    va_start(args, msg);
    Error* e = lexer._error(false, "SyntaxError", msg, &args);
    e->lineno = err().line;
    e->cursor = err().start;
    va_end(args);
    return e;
}

#undef consume
#undef consume_end_stmt
#undef check
#undef check_newlines_repl

}  // namespace pkpy
