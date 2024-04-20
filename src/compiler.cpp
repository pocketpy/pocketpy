#include "pocketpy/compiler.h"

namespace pkpy{
    PrattRule Compiler::rules[kTokenCount];

    NameScope Compiler::name_scope() const {
        auto s = contexts.size()>1 ? NAME_LOCAL : NAME_GLOBAL;
        if(unknown_global_scope && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
        return s;
    }

    CodeObject_ Compiler::push_global_context(){
        CodeObject_ co = std::make_shared<CodeObject>(lexer.src, lexer.src->filename);
        co->start_line = i==0 ? 1 : prev().line;
        contexts.push(CodeEmitContext(vm, co, contexts.size()));
        return co;
    }

    FuncDecl_ Compiler::push_f_context(Str name){
        FuncDecl_ decl = std::make_shared<FuncDecl>();
        decl->code = std::make_shared<CodeObject>(lexer.src, name);
        decl->code->start_line = i==0 ? 1 : prev().line;
        decl->nested = name_scope() == NAME_LOCAL;
        contexts.push(CodeEmitContext(vm, decl->code, contexts.size()));
        contexts.top().func = decl;
        return decl;
    }

    void Compiler::pop_context(){
        if(!ctx()->s_expr.empty()){
            throw std::runtime_error("!ctx()->s_expr.empty()");
        }
        // add a `return None` in the end as a guard
        // previously, we only do this if the last opcode is not a return
        // however, this is buggy...since there may be a jump to the end (out of bound) even if the last opcode is a return
        ctx()->emit_(OP_RETURN_VALUE, 1, BC_KEEPLINE, true);
        // find the last valid token
        int j = i-1;
        while(tokens[j].type == TK("@eol") || tokens[j].type == TK("@dedent") || tokens[j].type == TK("@eof")) j--;
        ctx()->co->end_line = tokens[j].line;

        // some check here
        auto& codes = ctx()->co->codes;
        if(ctx()->co->varnames.size() > PK_MAX_CO_VARNAMES){
            SyntaxError("maximum number of local variables exceeded");
        }
        if(ctx()->co->consts.size() > 65530){
            SyntaxError("maximum number of constants exceeded");
        }
        if(codes.size() > 65530 && ctx()->co->src->mode != JSON_MODE){
            // json mode does not contain jump instructions, so it is safe to ignore this check
            SyntaxError("maximum number of opcodes exceeded");
        }
        // pre-compute LOOP_BREAK and LOOP_CONTINUE
        for(int i=0; i<codes.size(); i++){
            Bytecode& bc = codes[i];
            if(bc.op == OP_LOOP_CONTINUE){
                bc.arg = ctx()->co->blocks[bc.arg].start;
            }else if(bc.op == OP_LOOP_BREAK){
                bc.arg = ctx()->co->blocks[bc.arg].get_break_end();
            }
        }
        // pre-compute func->is_simple
        FuncDecl_ func = contexts.top().func;
        if(func){
            // check generator
            for(Bytecode bc: func->code->codes){
                if(bc.op == OP_YIELD_VALUE || bc.op == OP_FOR_ITER_YIELD_VALUE){
                    func->type = FuncType::GENERATOR;
                    for(Bytecode bc: func->code->codes){
                        if(bc.op == OP_RETURN_VALUE && bc.arg == BC_NOARG){
                            SyntaxError("'return' with argument inside generator function");
                        }
                    }
                    break;
                }
            }
            if(func->type == FuncType::UNSET){
                bool is_simple = true;
                if(func->kwargs.size() > 0) is_simple = false;
                if(func->starred_arg >= 0) is_simple = false;
                if(func->starred_kwarg >= 0) is_simple = false;

                if(is_simple){
                    func->type = FuncType::SIMPLE;

                    bool is_empty = false;
                    if(func->code->codes.size() == 1){
                        Bytecode bc = func->code->codes[0];
                        if(bc.op == OP_RETURN_VALUE && bc.arg == 1){
                            is_empty = true;
                        }
                    }
                    if(is_empty) func->type = FuncType::EMPTY;
                }
                else func->type = FuncType::NORMAL;
            }

            PK_ASSERT(func->type != FuncType::UNSET);
        }
        contexts.pop();
    }

    void Compiler::init_pratt_rules(){
        PK_LOCAL_STATIC bool initialized = false;
        if(initialized) return;
        initialized = true;

// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define PK_METHOD(name) &Compiler::name
#define PK_NO_INFIX nullptr, PREC_LOWEST
        for(TokenIndex i=0; i<kTokenCount; i++) rules[i] = { nullptr, PK_NO_INFIX };
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
    }

    bool Compiler::match(TokenIndex expected) {
        if (curr().type != expected) return false;
        advance();
        return true;
    }

    void Compiler::consume(TokenIndex expected) {
        if (!match(expected)){
            SyntaxError(
                _S("expected '", TK_STR(expected), "', got '", TK_STR(curr().type), "'")
            );
        }
    }

    bool Compiler::match_newlines_repl(){
        return match_newlines(mode()==REPL_MODE);
    }

    bool Compiler::match_newlines(bool repl_throw) {
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

    bool Compiler::match_end_stmt() {
        if (match(TK(";"))) { match_newlines(); return true; }
        if (match_newlines() || curr().type == TK("@eof")) return true;
        if (curr().type == TK("@dedent")) return true;
        return false;
    }

    void Compiler::consume_end_stmt() {
        if (!match_end_stmt()) SyntaxError("expected statement end");
    }

    void Compiler::EXPR() {
        parse_expression(PREC_LOWEST+1);
    }

    void Compiler::EXPR_TUPLE(bool allow_slice) {
        parse_expression(PREC_LOWEST+1, allow_slice);
        if(!match(TK(","))) return;
        // tuple expression
        Expr_vector items;
        items.push_back(ctx()->s_expr.popx());
        do {
            if(curr().brackets_level) match_newlines_repl();
            if(!is_expression(allow_slice)) break;
            parse_expression(PREC_LOWEST+1, allow_slice);
            items.push_back(ctx()->s_expr.popx());
            if(curr().brackets_level) match_newlines_repl();
        } while(match(TK(",")));
        ctx()->s_expr.push(make_expr<TupleExpr>(std::move(items)));
    }

    // special case for `for loop` and `comp`
    Expr_ Compiler::EXPR_VARS(){
        Expr_vector items;
        do {
            consume(TK("@id"));
            items.push_back(make_expr<NameExpr>(prev().str(), name_scope()));
        } while(match(TK(",")));
        if(items.size()==1) return std::move(items[0]);
        return make_expr<TupleExpr>(std::move(items));
    }

    void Compiler::exprLiteral(){
        ctx()->s_expr.push(make_expr<LiteralExpr>(prev().value));
    }

    void Compiler::exprLong(){
        ctx()->s_expr.push(make_expr<LongExpr>(prev().str()));
    }

    void Compiler::exprImag(){
        ctx()->s_expr.push(make_expr<ImagExpr>(std::get<f64>(prev().value)));
    }

    void Compiler::exprBytes(){
        ctx()->s_expr.push(make_expr<BytesExpr>(std::get<Str>(prev().value)));
    }

    void Compiler::exprFString(){
        ctx()->s_expr.push(make_expr<FStringExpr>(std::get<Str>(prev().value)));
    }

    void Compiler::exprLambda(){
        FuncDecl_ decl = push_f_context("<lambda>");
        auto e = make_expr<LambdaExpr>(decl);
        if(!match(TK(":"))){
            _compile_f_args(e->decl, false);
            consume(TK(":"));
        }
        // https://github.com/pocketpy/pocketpy/issues/37
        parse_expression(PREC_LAMBDA + 1);
        ctx()->emit_expr();
        ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        pop_context();
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprOr(){
        auto e = make_expr<OrExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_OR + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void Compiler::exprAnd(){
        auto e = make_expr<AndExpr>();
        e->lhs = ctx()->s_expr.popx();
        parse_expression(PREC_LOGICAL_AND + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void Compiler::exprTernary(){
        auto e = make_expr<TernaryExpr>();
        e->true_expr = ctx()->s_expr.popx();
        // cond
        parse_expression(PREC_TERNARY + 1);
        e->cond = ctx()->s_expr.popx();
        consume(TK("else"));
        // if false
        parse_expression(PREC_TERNARY + 1);
        e->false_expr = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }
    
    void Compiler::exprBinaryOp(){
        auto e = make_expr<BinaryExpr>();
        e->op = prev().type;
        e->lhs = ctx()->s_expr.popx();
        parse_expression(rules[e->op].precedence + 1);
        e->rhs = ctx()->s_expr.popx();
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprNot() {
        parse_expression(PREC_LOGICAL_NOT + 1);
        ctx()->s_expr.push(make_expr<NotExpr>(ctx()->s_expr.popx()));
    }
    
    void Compiler::exprUnaryOp(){
        TokenIndex op = prev().type;
        parse_expression(PREC_UNARY + 1);
        switch(op){
            case TK("-"):
                ctx()->s_expr.push(make_expr<NegatedExpr>(ctx()->s_expr.popx()));
                break;
            case TK("~"):
                ctx()->s_expr.push(make_expr<InvertExpr>(ctx()->s_expr.popx()));
                break;
            case TK("*"):
                ctx()->s_expr.push(make_expr<StarredExpr>(1, ctx()->s_expr.popx()));
                break;
            case TK("**"):
                ctx()->s_expr.push(make_expr<StarredExpr>(2, ctx()->s_expr.popx()));
                break;
            default: PK_FATAL_ERROR();
        }
    }

    void Compiler::exprGroup(){
        match_newlines_repl();
        EXPR_TUPLE();   // () is just for change precedence
        match_newlines_repl();
        consume(TK(")"));
        if(ctx()->s_expr.top()->is_tuple()) return;
        Expr_ g = make_expr<GroupedExpr>(ctx()->s_expr.popx());
        ctx()->s_expr.push(std::move(g));
    }

    void Compiler::consume_comp(unique_ptr_128<CompExpr> ce, Expr_ expr){
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

    void Compiler::exprList() {
        int line = prev().line;
        Expr_vector items;
        do {
            match_newlines_repl();
            if (curr().type == TK("]")) break;
            EXPR();
            items.push_back(ctx()->s_expr.popx());
            match_newlines_repl();
            if(items.size()==1 && match(TK("for"))){
                consume_comp(make_expr<ListCompExpr>(), std::move(items[0]));
                consume(TK("]"));
                return;
            }
            match_newlines_repl();
        } while (match(TK(",")));
        consume(TK("]"));
        auto e = make_expr<ListExpr>(std::move(items));
        e->line = line;     // override line
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprMap() {
        bool parsing_dict = false;  // {...} may be dict or set
        Expr_vector items;
        do {
            match_newlines_repl();
            if (curr().type == TK("}")) break;
            EXPR();
            int star_level = ctx()->s_expr.top()->star_level();
            if(star_level==2 || curr().type == TK(":")){
                parsing_dict = true;
            }
            if(parsing_dict){
                auto dict_item = make_expr<DictItemExpr>();
                if(star_level == 2){
                    dict_item->key = nullptr;
                    dict_item->value = ctx()->s_expr.popx();
                }else{
                    consume(TK(":"));
                    EXPR();
                    dict_item->key = ctx()->s_expr.popx();
                    dict_item->value = ctx()->s_expr.popx();
                }
                items.push_back(std::move(dict_item));
            }else{
                items.push_back(ctx()->s_expr.popx());
            }
            match_newlines_repl();
            if(items.size()==1 && match(TK("for"))){
                if(parsing_dict) consume_comp(make_expr<DictCompExpr>(), std::move(items[0]));
                else consume_comp(make_expr<SetCompExpr>(), std::move(items[0]));
                consume(TK("}"));
                return;
            }
            match_newlines_repl();
        } while (match(TK(",")));
        consume(TK("}"));
        if(items.size()==0 || parsing_dict){
            auto e = make_expr<DictExpr>(std::move(items));
            ctx()->s_expr.push(std::move(e));
        }else{
            auto e = make_expr<SetExpr>(std::move(items));
            ctx()->s_expr.push(std::move(e));
        }
    }

    void Compiler::exprCall() {
        auto e = make_expr<CallExpr>();
        e->callable = ctx()->s_expr.popx();
        do {
            match_newlines_repl();
            if (curr().type==TK(")")) break;
            if(curr().type==TK("@id") && next().type==TK("=")) {
                consume(TK("@id"));
                Str key = prev().str();
                consume(TK("="));
                EXPR();
                e->kwargs.push_back({key, ctx()->s_expr.popx()});
            } else{
                EXPR();
                if(ctx()->s_expr.top()->star_level() == 2){
                    // **kwargs
                    e->kwargs.push_back({"**", ctx()->s_expr.popx()});
                }else{
                    // positional argument
                    if(!e->kwargs.empty()) SyntaxError("positional argument follows keyword argument");
                    e->args.push_back(ctx()->s_expr.popx());
                }
            }
            match_newlines_repl();
        } while (match(TK(",")));
        consume(TK(")"));
        if(e->args.size() > 32767) SyntaxError("too many positional arguments");
        if(e->kwargs.size() > 32767) SyntaxError("too many keyword arguments");
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprName(){
        Str name = prev().str();
        NameScope scope = name_scope();
        if(ctx()->global_names.count(name)){
            scope = NAME_GLOBAL;
        }
        ctx()->s_expr.push(make_expr<NameExpr>(name, scope));
    }

    void Compiler::exprAttrib() {
        consume(TK("@id"));
        ctx()->s_expr.push(
            make_expr<AttribExpr>(ctx()->s_expr.popx(), StrName::get(prev().sv()))
        );
    }

    void Compiler::exprSlice0() {
        auto slice = make_expr<SliceExpr>();
        if(is_expression()){        // :<stop>
            EXPR();
            slice->stop = ctx()->s_expr.popx();
            // try optional step
            if(match(TK(":"))){     // :<stop>:<step>
                EXPR();
                slice->step = ctx()->s_expr.popx();
            }
        }else if(match(TK(":"))){
            if(is_expression()){    // ::<step>
                EXPR();
                slice->step = ctx()->s_expr.popx();
            }   // else ::
        }   // else :
        ctx()->s_expr.push(std::move(slice));
    }

    void Compiler::exprSlice1() {
        auto slice = make_expr<SliceExpr>();
        slice->start = ctx()->s_expr.popx();
        if(is_expression()){        // <start>:<stop>
            EXPR();
            slice->stop = ctx()->s_expr.popx();
            // try optional step
            if(match(TK(":"))){     // <start>:<stop>:<step>
                EXPR();
                slice->step = ctx()->s_expr.popx();
            }
        }else if(match(TK(":"))){   // <start>::<step>
            EXPR();
            slice->step = ctx()->s_expr.popx();
        }   // else <start>:
        ctx()->s_expr.push(std::move(slice));
    }
    
    void Compiler::exprSubscr() {
        auto e = make_expr<SubscrExpr>();
        match_newlines_repl();
        e->a = ctx()->s_expr.popx();        // a
        EXPR_TUPLE(true);
        e->b = ctx()->s_expr.popx();        // a[<expr>]
        match_newlines_repl();
        consume(TK("]"));
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprLiteral0() {
        ctx()->s_expr.push(make_expr<Literal0Expr>(prev().type));
    }

    void Compiler::compile_block_body(void (Compiler::*callback)()) {
        if(callback == nullptr) callback = &Compiler::compile_stmt;
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
            (this->*callback)();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    // import a [as b]
    // import a [as b], c [as d]
    void Compiler::compile_normal_import() {
        do {
            consume(TK("@id"));
            Str name = prev().str();
            ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const_string(name.sv()), prev().line);
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            ctx()->emit_store_name(name_scope(), StrName(name), prev().line);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b [as c], d [as e]
    // from a.b import c [as d]
    // from . import a [as b]
    // from .a import b [as c]
    // from ..a import b [as c]
    // from .a.b import c [as d]
    // from xxx import *
    void Compiler::compile_from_import() {
        int dots = 0;

        while(true){
            switch(curr().type){
                case TK("."): dots+=1; break;
                case TK(".."): dots+=2; break;
                case TK("..."): dots+=3; break;
                default: goto __EAT_DOTS_END;
            }
            advance();
        }
__EAT_DOTS_END:
        SStream ss;
        for(int i=0; i<dots; i++) ss << '.';

        if(dots > 0){
            // @id is optional if dots > 0
            if(match(TK("@id"))){
                ss << prev().sv();
                while (match(TK("."))) {
                    consume(TK("@id"));
                    ss << "." << prev().sv();
                }
            }
        }else{
            // @id is required if dots == 0
            consume(TK("@id"));
            ss << prev().sv();
            while (match(TK("."))) {
                consume(TK("@id"));
                ss << "." << prev().sv();
            }
        }

        ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const_string(ss.str().sv()), prev().line);
        consume(TK("import"));

        if (match(TK("*"))) {
            if(name_scope() != NAME_GLOBAL) SyntaxError("from <module> import * can only be used in global scope");
            // pop the module and import __all__
            ctx()->emit_(OP_POP_IMPORT_STAR, BC_NOARG, prev().line);
            consume_end_stmt();
            return;
        }

        do {
            ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
            consume(TK("@id"));
            Str name = prev().str();
            ctx()->emit_(OP_LOAD_ATTR, StrName(name).index, prev().line);
            if (match(TK("as"))) {
                consume(TK("@id"));
                name = prev().str();
            }
            ctx()->emit_store_name(name_scope(), StrName(name), prev().line);
        } while (match(TK(",")));
        ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
        consume_end_stmt();
    }

    bool Compiler::is_expression(bool allow_slice){
        PrattCallback prefix = rules[curr().type].prefix;
        return prefix != nullptr && (allow_slice || curr().type!=TK(":"));
    }

    void Compiler::parse_expression(int precedence, bool allow_slice) {
        PrattCallback prefix = rules[curr().type].prefix;
        if (prefix==nullptr || (curr().type==TK(":") && !allow_slice)){
            SyntaxError(Str("expected an expression, got ") + TK_STR(curr().type));
        }
        advance();
        (this->*prefix)();
        while (rules[curr().type].precedence >= precedence && (allow_slice || curr().type!=TK(":"))) {
            TokenIndex op = curr().type;
            advance();
            PrattCallback infix = rules[op].infix;
            PK_ASSERT(infix != nullptr);
            (this->*infix)();
        }
    }

    void Compiler::compile_if_stmt() {
        EXPR();   // condition
        ctx()->emit_expr();
        int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        if (match(TK("elif"))) {
            int exit_patch = ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_if_stmt();
            ctx()->patch_jump(exit_patch);
        } else if (match(TK("else"))) {
            int exit_patch = ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, prev().line);
            ctx()->patch_jump(patch);
            compile_block_body();
            ctx()->patch_jump(exit_patch);
        } else {
            ctx()->patch_jump(patch);
        }
    }

    void Compiler::compile_while_loop() {
        CodeBlock* block = ctx()->enter_block(CodeBlockType::WHILE_LOOP);
        EXPR();   // condition
        ctx()->emit_expr();
        int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE, true);
        ctx()->patch_jump(patch);
        ctx()->exit_block();
        // optional else clause
        if (match(TK("else"))) {
            compile_block_body();
            block->end2 = ctx()->co->codes.size();
        }
    }

    void Compiler::compile_for_loop() {
        Expr_ vars = EXPR_VARS();
        consume(TK("in"));
        EXPR_TUPLE(); ctx()->emit_expr();
        ctx()->emit_(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        CodeBlock* block = ctx()->enter_block(CodeBlockType::FOR_LOOP);
        int for_codei = ctx()->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx());
        if(!ok) SyntaxError();  // this error occurs in `vars` instead of this line, but...nevermind
        ctx()->try_merge_for_iter_store(for_codei);
        compile_block_body();
        ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE, true);
        ctx()->exit_block();
        // optional else clause
        if (match(TK("else"))) {
            compile_block_body();
            block->end2 = ctx()->co->codes.size();
        }
    }

    void Compiler::compile_try_except() {
        ctx()->enter_block(CodeBlockType::TRY_EXCEPT);
        compile_block_body();
        pod_vector<int> patches = {
            ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE)
        };
        ctx()->exit_block();

        int finally_entry = -1;
        if(curr().type != TK("finally")){
            do {
                StrName as_name;
                consume(TK("except"));
                if(is_expression()){
                    EXPR();      // push assumed type on to the stack
                    ctx()->emit_expr();
                    ctx()->emit_(OP_EXCEPTION_MATCH, BC_NOARG, prev().line);
                    if(match(TK("as"))){
                        consume(TK("@id"));
                        as_name = StrName(prev().sv());
                    }
                }else{
                    ctx()->emit_(OP_LOAD_TRUE, BC_NOARG, BC_KEEPLINE);
                }
                int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
                // on match
                if(!as_name.empty()){
                    ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
                    ctx()->emit_store_name(name_scope(), as_name, BC_KEEPLINE);
                }
                // pop the exception 
                ctx()->emit_(OP_POP_EXCEPTION, BC_NOARG, BC_KEEPLINE);
                compile_block_body();
                patches.push_back(ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE));
                ctx()->patch_jump(patch);
            }while(curr().type == TK("except"));
        }

        if(match(TK("finally"))){
            int patch = ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE);
            finally_entry = ctx()->co->codes.size();
            compile_block_body();
            ctx()->emit_(OP_JUMP_ABSOLUTE_TOP, BC_NOARG, BC_KEEPLINE);
            ctx()->patch_jump(patch);
        }
        // no match, re-raise
        if(finally_entry != -1){
            i64 target = ctx()->co->codes.size()+2;
            ctx()->emit_(OP_LOAD_CONST, ctx()->add_const(VAR(target)), BC_KEEPLINE);
            ctx()->emit_(OP_JUMP_ABSOLUTE, finally_entry, BC_KEEPLINE);
        }
        ctx()->emit_(OP_RE_RAISE, BC_NOARG, BC_KEEPLINE);

        // no exception or no match, jump to the end
        for (int patch : patches) ctx()->patch_jump(patch);
        if(finally_entry != -1){
            i64 target = ctx()->co->codes.size()+2;
            ctx()->emit_(OP_LOAD_CONST, ctx()->add_const(VAR(target)), BC_KEEPLINE);
            ctx()->emit_(OP_JUMP_ABSOLUTE, finally_entry, BC_KEEPLINE);
        }
    }

    void Compiler::compile_decorated(){
        Expr_vector decorators;
        do{
            EXPR();
            decorators.push_back(ctx()->s_expr.popx());
            if(!match_newlines_repl()) SyntaxError();
        }while(match(TK("@")));

        if(match(TK("class"))){
            compile_class(decorators);
        }else{
            consume(TK("def"));
            compile_function(decorators);
        }
    }

    bool Compiler::try_compile_assignment(){
        switch (curr().type) {
            case TK("+="): case TK("-="): case TK("*="): case TK("/="): case TK("//="): case TK("%="):
            case TK("<<="): case TK(">>="): case TK("&="): case TK("|="): case TK("^="): {
                Expr* lhs_p = ctx()->s_expr.top().get();
                if(lhs_p->is_starred()) SyntaxError();
                if(ctx()->is_compiling_class) SyntaxError("can't use inplace operator in class definition");
                advance();
                auto e = make_expr<BinaryExpr>();
                e->op = prev().type - 1; // -1 to remove =
                e->lhs = ctx()->s_expr.popx();
                EXPR_TUPLE();
                e->rhs = ctx()->s_expr.popx();
                if(e->is_starred()) SyntaxError();
                e->emit_(ctx());
                bool ok = lhs_p->emit_store(ctx());
                if(!ok) SyntaxError();
            } return true;
            case TK("="): {
                int n = 0;
                while(match(TK("="))){
                    EXPR_TUPLE();
                    n += 1;
                }
                // stack size is n+1
                Expr_ val = ctx()->s_expr.popx();
                val->emit_(ctx());
                for(int j=1; j<n; j++) ctx()->emit_(OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
                for(int j=0; j<n; j++){
                    auto e = ctx()->s_expr.popx();
                    if(e->is_starred()) SyntaxError();
                    bool ok = e->emit_store(ctx());
                    if(!ok) SyntaxError();
                }
            } return true;
            default: return false;
        }
    }

    void Compiler::compile_stmt() {
        if(match(TK("class"))){
            compile_class();
            return;
        }
        advance();
        int kw_line = prev().line;  // backup line number
        int curr_loop_block = ctx()->get_loop();
        switch(prev().type){
            case TK("break"):
                if (curr_loop_block < 0) SyntaxError("'break' outside loop");
                ctx()->emit_(OP_LOOP_BREAK, curr_loop_block, kw_line);
                consume_end_stmt();
                break;
            case TK("continue"):
                if (curr_loop_block < 0) SyntaxError("'continue' not properly in loop");
                ctx()->emit_(OP_LOOP_CONTINUE, curr_loop_block, kw_line);
                consume_end_stmt();
                break;
            case TK("yield"): 
                if (contexts.size() <= 1) SyntaxError("'yield' outside function");
                EXPR_TUPLE(); ctx()->emit_expr();
                ctx()->emit_(OP_YIELD_VALUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("yield from"):
                if (contexts.size() <= 1) SyntaxError("'yield from' outside function");
                EXPR_TUPLE(); ctx()->emit_expr();

                ctx()->emit_(OP_GET_ITER, BC_NOARG, kw_line);
                ctx()->enter_block(CodeBlockType::FOR_LOOP);
                ctx()->emit_(OP_FOR_ITER_YIELD_VALUE, BC_NOARG, kw_line);
                ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), kw_line);
                ctx()->exit_block();
                consume_end_stmt();
                break;
            case TK("return"):
                if (contexts.size() <= 1) SyntaxError("'return' outside function");
                if(match_end_stmt()){
                    ctx()->emit_(OP_RETURN_VALUE, 1, kw_line);
                }else{
                    EXPR_TUPLE(); ctx()->emit_expr();
                    consume_end_stmt();
                    ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, kw_line);
                }
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
            case TK("++"):{
                consume(TK("@id"));
                StrName name(prev().sv());
                NameScope scope = name_scope();
                bool is_global = ctx()->global_names.count(name.sv());
                if(is_global) scope = NAME_GLOBAL;
                switch(scope){
                    case NAME_LOCAL:
                        ctx()->emit_(OP_INC_FAST, ctx()->add_varname(name), prev().line);
                        break;
                    case NAME_GLOBAL:
                        ctx()->emit_(OP_INC_GLOBAL, name.index, prev().line);
                        break;
                    default: SyntaxError(); break;
                }
                consume_end_stmt();
                break;
            }
            case TK("--"):{
                consume(TK("@id"));
                StrName name(prev().sv());
                switch(name_scope()){
                    case NAME_LOCAL:
                        ctx()->emit_(OP_DEC_FAST, ctx()->add_varname(name), prev().line);
                        break;
                    case NAME_GLOBAL:
                        ctx()->emit_(OP_DEC_GLOBAL, name.index, prev().line);
                        break;
                    default: SyntaxError(); break;
                }
                consume_end_stmt();
                break;
            }
            case TK("assert"):{
                EXPR();    // condition
                ctx()->emit_expr();
                int index = ctx()->emit_(OP_POP_JUMP_IF_TRUE, BC_NOARG, kw_line);
                int has_msg = 0;
                if(match(TK(","))){
                    EXPR();    // message
                    ctx()->emit_expr();
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
                    ctx()->global_names.insert(prev().str());
                } while (match(TK(",")));
                consume_end_stmt();
                break;
            case TK("raise"): {
                EXPR(); ctx()->emit_expr();
                ctx()->emit_(OP_RAISE, BC_NOARG, kw_line);
                consume_end_stmt();
            } break;
            case TK("del"): {
                EXPR_TUPLE();
                Expr_ e = ctx()->s_expr.popx();
                bool ok = e->emit_del(ctx());
                if(!ok) SyntaxError();
                consume_end_stmt();
            } break;
            case TK("with"): {
                EXPR();    // [ <expr> ]
                ctx()->emit_expr();
                ctx()->enter_block(CodeBlockType::CONTEXT_MANAGER);
                Expr_ as_name;
                if(match(TK("as"))){
                    consume(TK("@id"));
                    as_name = make_expr<NameExpr>(prev().str(), name_scope());
                }
                ctx()->emit_(OP_WITH_ENTER, BC_NOARG, prev().line);
                // [ <expr> <expr>.__enter__() ]
                if(as_name != nullptr){
                    bool ok = as_name->emit_store(ctx());
                    if(!ok) SyntaxError();
                }else{
                    ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                }
                compile_block_body();
                ctx()->emit_(OP_WITH_EXIT, BC_NOARG, prev().line);
                ctx()->exit_block();
            } break;
            /*************************************************/
            case TK("=="): {
                consume(TK("@id"));
                if(mode()!=EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
                bool ok = ctx()->add_label(prev().str());
                consume(TK("=="));
                if(!ok) SyntaxError("label " + prev().str().escape() + " already exists");
                consume_end_stmt();
            } break;
            case TK("->"):
                consume(TK("@id"));
                if(mode()!=EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
                ctx()->emit_(OP_GOTO, StrName(prev().sv()).index, prev().line);
                consume_end_stmt();
                break;
            /*************************************************/
            // handle dangling expression or assignment
            default: {
                advance(-1);    // do revert since we have pre-called advance() at the beginning
                EXPR_TUPLE();

                bool is_typed_name = false;     // e.g. x: int
                // eat variable's type hint if it is a single name
                if(ctx()->s_expr.top()->is_name()){
                    if(match(TK(":"))){
                        consume_type_hints();
                        is_typed_name = true;

                        if(ctx()->is_compiling_class){
                            NameExpr* ne = static_cast<NameExpr*>(ctx()->s_expr.top().get());
                            ctx()->emit_(OP_ADD_CLASS_ANNOTATION, ne->name.index, BC_KEEPLINE);
                        }
                    }
                }
                if(!try_compile_assignment()){
                    if(!ctx()->s_expr.empty() && ctx()->s_expr.top()->is_starred()){
                        SyntaxError();
                    }
                    if(!is_typed_name){
                        ctx()->emit_expr();
                        if((mode()==CELL_MODE || mode()==REPL_MODE) && name_scope()==NAME_GLOBAL){
                            ctx()->emit_(OP_PRINT_EXPR, BC_NOARG, BC_KEEPLINE);
                        }else{
                            ctx()->emit_(OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                        }
                    }else{
                        PK_ASSERT(ctx()->s_expr.size() == 1)
                        ctx()->s_expr.pop();
                    }
                }
                consume_end_stmt();
            }
        }
    }

    void Compiler::consume_type_hints(){
        EXPR();
        ctx()->s_expr.pop();
    }

    void Compiler::_add_decorators(const Expr_vector& decorators){
        // [obj]
        for(auto it=decorators.rbegin(); it!=decorators.rend(); ++it){
            (*it)->emit_(ctx());                                    // [obj, f]
            ctx()->emit_(OP_ROT_TWO, BC_NOARG, (*it)->line);        // [f, obj]
            ctx()->emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);      // [f, obj, NULL]
            ctx()->emit_(OP_ROT_TWO, BC_NOARG, BC_KEEPLINE);        // [obj, NULL, f]
            ctx()->emit_(OP_CALL, 1, (*it)->line);                  // [obj]
        }
    }

    void Compiler::compile_class(const Expr_vector& decorators){
        consume(TK("@id"));
        int namei = StrName(prev().sv()).index;
        Expr_ base = nullptr;
        if(match(TK("("))){
            if(is_expression()){
                EXPR();
                base = ctx()->s_expr.popx();
            }
            consume(TK(")"));
        }
        if(base == nullptr){
            ctx()->emit_(OP_LOAD_NONE, BC_NOARG, prev().line);
        }else {
            base->emit_(ctx());
        }
        ctx()->emit_(OP_BEGIN_CLASS, namei, BC_KEEPLINE);

        for(auto& c: this->contexts.container()){
            if(c.is_compiling_class){
                SyntaxError("nested class is not allowed");
            }
        }
        ctx()->is_compiling_class = true;
        compile_block_body();
        ctx()->is_compiling_class = false;

        if(!decorators.empty()){
            ctx()->emit_(OP_BEGIN_CLASS_DECORATION, BC_NOARG, BC_KEEPLINE);
            _add_decorators(decorators);
            ctx()->emit_(OP_END_CLASS_DECORATION, BC_NOARG, BC_KEEPLINE);
        }

        ctx()->emit_(OP_END_CLASS, namei, BC_KEEPLINE);
    }

    void Compiler::_compile_f_args(FuncDecl_ decl, bool enable_type_hints){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state > 3) SyntaxError();
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
            StrName name = prev().str();

            // check duplicate argument name
            for(int j: decl->args){
                if(decl->code->varnames[j] == name) {
                    SyntaxError("duplicate argument name");
                }
            }
            for(auto& kv: decl->kwargs){
                if(decl->code->varnames[kv.index] == name){
                    SyntaxError("duplicate argument name");
                }
            }
            if(decl->starred_arg!=-1 && decl->code->varnames[decl->starred_arg] == name){
                SyntaxError("duplicate argument name");
            }
            if(decl->starred_kwarg!=-1 && decl->code->varnames[decl->starred_kwarg] == name){
                SyntaxError("duplicate argument name");
            }

            // eat type hints
            if(enable_type_hints && match(TK(":"))) consume_type_hints();
            if(state == 0 && curr().type == TK("=")) state = 2;
            int index = ctx()->add_varname(name);
            switch (state)
            {
                case 0:
                    decl->args.push_back(index);
                    break;
                case 1:
                    decl->starred_arg = index;
                    state+=1;
                    break;
                case 2: {
                    consume(TK("="));
                    PyObject* value = read_literal();
                    if(value == nullptr){
                        SyntaxError(Str("default argument must be a literal"));
                    }
                    decl->add_kwarg(index, name, value);
                } break;
                case 3:
                    decl->starred_kwarg = index;
                    state+=1;
                    break;
            }
        } while (match(TK(",")));
    }

    void Compiler::compile_function(const Expr_vector& decorators){
        consume(TK("@id"));
        Str decl_name = prev().str();
        FuncDecl_ decl = push_f_context(decl_name);
        consume(TK("("));
        if (!match(TK(")"))) {
            _compile_f_args(decl, true);
            consume(TK(")"));
        }
        if(match(TK("->"))) consume_type_hints();
        compile_block_body();
        pop_context();

        decl->docstring = nullptr;
        if(decl->code->codes.size()>=2 && decl->code->codes[0].op == OP_LOAD_CONST && decl->code->codes[1].op == OP_POP_TOP){
            PyObject* c = decl->code->consts[decl->code->codes[0].arg];
            if(is_type(c, vm->tp_str)){
                decl->code->codes[0].op = OP_NO_OP;
                decl->code->codes[1].op = OP_NO_OP;
                decl->docstring = PK_OBJ_GET(Str, c).c_str();
            }
        }
        ctx()->emit_(OP_LOAD_FUNCTION, ctx()->add_func_decl(decl), prev().line);

        _add_decorators(decorators);

        if(!ctx()->is_compiling_class){
            auto e = make_expr<NameExpr>(decl_name, name_scope());
            e->emit_store(ctx());
        }else{
            int index = StrName(decl_name).index;
            ctx()->emit_(OP_STORE_CLASS_ATTR, index, prev().line);
        }
    }

    PyObject* Compiler::to_object(const TokenValue& value){
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
        PK_ASSERT(obj != nullptr)
        return obj;
    }

    PyObject* Compiler::read_literal(){
        advance();
        switch(prev().type){
            case TK("-"): {
                consume(TK("@num"));
                PyObject* val = to_object(prev().value);
                return vm->py_negate(val);
            }
            case TK("@num"): return to_object(prev().value);
            case TK("@str"): return to_object(prev().value);
            case TK("True"): return VAR(true);
            case TK("False"): return VAR(false);
            case TK("None"): return vm->None;
            case TK("..."): return vm->Ellipsis;
            case TK("("): {
                List cpnts;
                while(true) {
                    cpnts.push_back(read_literal());
                    if(curr().type == TK(")")) break;
                    consume(TK(","));
                    if(curr().type == TK(")")) break;
                }
                consume(TK(")"));
                return VAR(Tuple(std::move(cpnts)));
            }
            default: break;
        }
        return nullptr;
    }

    Compiler::Compiler(VM* vm, std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope)
            :lexer(vm, std::make_shared<SourceData>(source, filename, mode)){
        this->vm = vm;
        this->unknown_global_scope = unknown_global_scope;
        init_pratt_rules();
    }

    Str Compiler::precompile(){
        auto tokens = lexer.run();
        SStream ss;
        ss << "pkpy:" PK_VERSION << '\n';           // L1: version string
        ss << (int)mode() << '\n';                  // L2: mode

        std::map<std::string_view, int> token_indices;
        for(auto token: tokens){
            if(is_raw_string_used(token.type)){
                auto it = token_indices.find(token.sv());
                if(it == token_indices.end()){
                    token_indices[token.sv()] = 0;
                    // assert no '\n' in token.sv()
                    for(char c: token.sv()) if(c=='\n') PK_FATAL_ERROR();
                }
            }
        }
        ss << "=" << (int)token_indices.size() << '\n';         // L3: raw string count
        int index = 0;
        for(auto& kv: token_indices){
            ss << kv.first << '\n';    // L4: raw strings
            kv.second = index++;
        }
        
        ss << "=" << (int)tokens.size() << '\n';    // L5: token count
        for(int i=0; i<tokens.size(); i++){
            const Token& token = tokens[i];
            ss << (int)token.type << ',';
            if(is_raw_string_used(token.type)){
                ss << token_indices[token.sv()] << ',';
            }
            if(i>0 && tokens[i-1].line == token.line) ss << ',';
            else ss << token.line << ',';
            if(i>0 && tokens[i-1].brackets_level == token.brackets_level) ss << ',';
            else ss << token.brackets_level << ',';
            // visit token value
            std::visit([&ss](auto&& arg){
                using T = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<T, i64>){
                    ss << 'I' << arg;
                }else if constexpr(std::is_same_v<T, f64>){
                    ss << 'F' << arg;
                }else if constexpr(std::is_same_v<T, Str>){
                    ss << 'S';
                    for(char c: arg) ss.write_hex((unsigned char)c);
                }
                ss << '\n';
            }, token.value);
        }
        return ss.str();
    }

    void Compiler::from_precompiled(const char* source){
        TokenDeserializer deserializer(source);
        deserializer.curr += 5;     // skip "pkpy:"
        std::string_view version = deserializer.read_string('\n');

        if(version != PK_VERSION){
            SyntaxError(_S("precompiled version mismatch: ", version, "!=" PK_VERSION));
        }
        if(deserializer.read_uint('\n') != (i64)mode()){
            SyntaxError("precompiled mode mismatch");
        }

        int count = deserializer.read_count();
        std::vector<Str>& precompiled_tokens = lexer.src->_precompiled_tokens;
        for(int i=0; i<count; i++){
            precompiled_tokens.push_back(deserializer.read_string('\n'));
        }

        count = deserializer.read_count();
        for(int i=0; i<count; i++){
            Token t;
            t.type = (unsigned char)deserializer.read_uint(',');
            if(is_raw_string_used(t.type)){
                i64 index = deserializer.read_uint(',');
                t.start = precompiled_tokens[index].c_str();
                t.length = precompiled_tokens[index].size;
            }else{
                t.start = nullptr;
                t.length = 0;
            }

            if(deserializer.match_char(',')){
                t.line = tokens.back().line;
            }else{
                t.line = (int)deserializer.read_uint(',');
            }

            if(deserializer.match_char(',')){
                t.brackets_level = tokens.back().brackets_level;
            }else{
                t.brackets_level = (int)deserializer.read_uint(',');
            }

            char type = deserializer.read_char();
            switch(type){
                case 'I': t.value = deserializer.read_uint('\n'); break;
                case 'F': t.value = deserializer.read_float('\n'); break;
                case 'S': t.value = deserializer.read_string_from_hex('\n'); break;
                default: t.value = {}; break;
            }
            tokens.push_back(t);
        }
    }

    CodeObject_ Compiler::compile(){
        PK_ASSERT(i == 0)       // make sure it is the first time to compile

        if(lexer.src->is_precompiled){
            from_precompiled(lexer.src->source.c_str());
        }else{
            this->tokens = lexer.run();
        }

        CodeObject_ code = push_global_context();

        advance();          // skip @sof, so prev() is always valid
        match_newlines();   // skip possible leading '\n'

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE(); ctx()->emit_expr();
            consume(TK("@eof"));
            ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }else if(mode()==JSON_MODE){
            EXPR();
            Expr_ e = ctx()->s_expr.popx();
            if(!e->is_json_object()) SyntaxError("expect a JSON object, literal or array");
            consume(TK("@eof"));
            e->emit_(ctx());
            ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
            pop_context();
            return code;
        }

        while (!match(TK("@eof"))) {
            compile_stmt();
            match_newlines();
        }
        pop_context();
        return code;
    }

    // TODO: refactor this
    void Lexer::throw_err(StrName type, Str msg, int lineno, const char* cursor){
        PyObject* e_obj = vm->call(vm->builtins->attr(type), VAR(msg));
        Exception& e = PK_OBJ_GET(Exception, e_obj);
        e.st_push(src, lineno, cursor, "");
        throw e;
    }

    std::string_view TokenDeserializer::read_string(char c){
        const char* start = curr;
        while(*curr != c) curr++;
        std::string_view retval(start, curr-start);
        curr++;     // skip the delimiter
        return retval;
    }

    Str TokenDeserializer::read_string_from_hex(char c){
        std::string_view s = read_string(c);
        char* buffer = (char*)pool64_alloc(s.size()/2 + 1);
        for(int i=0; i<s.size(); i+=2){
            char c = 0;
            if(s[i]>='0' && s[i]<='9') c += s[i]-'0';
            else if(s[i]>='a' && s[i]<='f') c += s[i]-'a'+10;
            else PK_FATAL_ERROR();
            c <<= 4;
            if(s[i+1]>='0' && s[i+1]<='9') c += s[i+1]-'0';
            else if(s[i+1]>='a' && s[i+1]<='f') c += s[i+1]-'a'+10;
            else PK_FATAL_ERROR();
            buffer[i/2] = c;
        }
        buffer[s.size()/2] = 0;
        return std::pair<char*, int>(buffer, s.size()/2);
    }

    int TokenDeserializer::read_count(){
        PK_ASSERT(*curr == '=')
        curr++;
        return read_uint('\n');
    }

    i64 TokenDeserializer::read_uint(char c){
        i64 out = 0;
        while(*curr != c){
            out = out*10 + (*curr-'0');
            curr++;
        }
        curr++;     // skip the delimiter
        return out;
    }

    f64 TokenDeserializer::read_float(char c){
        std::string_view sv = read_string(c);
        return std::stod(std::string(sv));
    }
}   // namespace pkpy