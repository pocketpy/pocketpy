#include "pocketpy/compiler.h"

namespace pkpy{

    NameScope Compiler::name_scope() const {
        auto s = contexts.size()>1 ? NAME_LOCAL : NAME_GLOBAL;
        if(unknown_global_scope && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
        return s;
    }

    CodeObject_ Compiler::push_global_context(){
        CodeObject_ co = std::make_shared<CodeObject>(lexer->src, lexer->src->filename);
        contexts.push(CodeEmitContext(vm, co, contexts.size()));
        return co;
    }

    FuncDecl_ Compiler::push_f_context(Str name){
        FuncDecl_ decl = std::make_shared<FuncDecl>();
        decl->code = std::make_shared<CodeObject>(lexer->src, name);
        decl->nested = name_scope() == NAME_LOCAL;
        contexts.push(CodeEmitContext(vm, decl->code, contexts.size()));
        contexts.top().func = decl;
        return decl;
    }

    void Compiler::pop_context(){
        if(!ctx()->s_expr.empty()){
            throw std::runtime_error("!ctx()->s_expr.empty()\n" + ctx()->_log_s_expr());
        }
        // add a `return None` in the end as a guard
        // previously, we only do this if the last opcode is not a return
        // however, this is buggy...since there may be a jump to the end (out of bound) even if the last opcode is a return
        ctx()->emit_(OP_LOAD_NONE, BC_NOARG, BC_KEEPLINE);
        ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        // some check here
        std::vector<Bytecode>& codes = ctx()->co->codes;
        if(ctx()->co->varnames.size() > PK_MAX_CO_VARNAMES){
            SyntaxError("maximum number of local variables exceeded");
        }
        if(ctx()->co->consts.size() > 65535){
            std::map<std::string, int> counts;
            for(PyObject* c: ctx()->co->consts){
                std::string key = obj_type_name(vm, vm->_tp(c)).str();
                counts[key] += 1;
            }
            for(auto pair: counts){
                std::cout << pair.first << ": " << pair.second << std::endl;
            }
            SyntaxError("maximum number of constants exceeded");
        }
        if(codes.size() > 65535 && ctx()->co->src->mode != JSON_MODE){
            // json mode does not contain jump instructions, so it is safe to ignore this check
            SyntaxError("maximum number of opcodes exceeded");
        }
        // pre-compute LOOP_BREAK and LOOP_CONTINUE and FOR_ITER
        for(int i=0; i<codes.size(); i++){
            Bytecode& bc = codes[i];
            if(bc.op == OP_LOOP_CONTINUE){
                bc.arg = ctx()->co->blocks[bc.arg].start;
            }else if(bc.op == OP_LOOP_BREAK){
                bc.arg = ctx()->co->blocks[bc.arg].get_break_end();
            }else if(bc.op == OP_FOR_ITER){
                bc.arg = ctx()->co->_get_block_codei(i).end;
            }
        }
        FuncDecl_ func = contexts.top().func;
        if(func){
            func->is_simple = true;
            if(func->code->is_generator) func->is_simple = false;
            if(func->kwargs.size() > 0) func->is_simple = false;
            if(func->starred_arg >= 0) func->is_simple = false;
            if(func->starred_kwarg >= 0) func->is_simple = false;
        }
        contexts.pop();
    }

    void Compiler::init_pratt_rules(){
        PK_LOCAL_STATIC unsigned int count = 0;
        if(count > 0) return;
        count += 1;
// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define PK_METHOD(name) &Compiler::name
#define PK_NO_INFIX nullptr, PREC_NONE
        for(TokenIndex i=0; i<kTokenCount; i++) rules[i] = { nullptr, PK_NO_INFIX };
        rules[TK(".")] =        { nullptr,                  PK_METHOD(exprAttrib),         PREC_ATTRIB };
        rules[TK("(")] =        { PK_METHOD(exprGroup),     PK_METHOD(exprCall),           PREC_CALL };
        rules[TK("[")] =        { PK_METHOD(exprList),      PK_METHOD(exprSubscr),         PREC_SUBSCRIPT };
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
        rules[TK(",")] =        { nullptr,               PK_METHOD(exprTuple),          PREC_TUPLE };
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
        rules[TK("@bytes")] =   { PK_METHOD(exprBytes),     PK_NO_INFIX };
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
                fmt("expected '", TK_STR(expected), "', got '", TK_STR(curr().type), "'")
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

    void Compiler::EXPR(bool push_stack) {
        parse_expression(PREC_TUPLE+1, push_stack);
    }

    void Compiler::EXPR_TUPLE(bool push_stack) {
        parse_expression(PREC_TUPLE, push_stack);
    }

    // special case for `for loop` and `comp`
    Expr_ Compiler::EXPR_VARS(){
        std::vector<Expr_> items;
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
        // https://github.com/blueloveTH/pocketpy/issues/37
        parse_expression(PREC_LAMBDA + 1, false);
        ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        pop_context();
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprTuple(){
        std::vector<Expr_> items;
        items.push_back(ctx()->s_expr.popx());
        do {
            if(curr().brackets_level) match_newlines_repl();
            if(!is_expression()) break;
            EXPR();
            items.push_back(ctx()->s_expr.popx());
            if(curr().brackets_level) match_newlines_repl();
        } while(match(TK(",")));
        ctx()->s_expr.push(make_expr<TupleExpr>(
            std::move(items)
        ));
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
            default: FATAL_ERROR();
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

    void Compiler::exprList() {
        int line = prev().line;
        std::vector<Expr_> items;
        do {
            match_newlines_repl();
            if (curr().type == TK("]")) break;
            EXPR();
            items.push_back(ctx()->s_expr.popx());
            match_newlines_repl();
            if(items.size()==1 && match(TK("for"))){
                _consume_comp<ListCompExpr>(std::move(items[0]));
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
        std::vector<Expr_> items;
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
                if(parsing_dict) _consume_comp<DictCompExpr>(std::move(items[0]));
                else _consume_comp<SetCompExpr>(std::move(items[0]));
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
            make_expr<AttribExpr>(ctx()->s_expr.popx(), prev().str())
        );
    }
    
    void Compiler::exprSubscr() {
        auto e = make_expr<SubscrExpr>();
        e->a = ctx()->s_expr.popx();
        auto slice = make_expr<SliceExpr>();
        bool is_slice = false;
        // a[<0> <state:1> : state<3> : state<5>]
        int state = 0;
        do{
            match_newlines_repl();
            switch(state){
                case 0:
                    if(match(TK(":"))){
                        is_slice=true;
                        state=2;
                        break;
                    }
                    if(match(TK("]"))) SyntaxError();
                    EXPR_TUPLE();
                    slice->start = ctx()->s_expr.popx();
                    state=1;
                    break;
                case 1:
                    if(match(TK(":"))){
                        is_slice=true;
                        state=2;
                        break;
                    }
                    if(match(TK("]"))) goto __SUBSCR_END;
                    SyntaxError("expected ':' or ']'");
                    break;
                case 2:
                    if(match(TK(":"))){
                        state=4;
                        break;
                    }
                    if(match(TK("]"))) goto __SUBSCR_END;
                    EXPR_TUPLE();
                    slice->stop = ctx()->s_expr.popx();
                    state=3;
                    break;
                case 3:
                    if(match(TK(":"))){
                        state=4;
                        break;
                    }
                    if(match(TK("]"))) goto __SUBSCR_END;
                    SyntaxError("expected ':' or ']'");
                    break;
                case 4:
                    if(match(TK("]"))) goto __SUBSCR_END;
                    EXPR_TUPLE();
                    slice->step = ctx()->s_expr.popx();
                    state=5;
                    break;
                case 5: consume(TK("]")); goto __SUBSCR_END;
            }
            match_newlines_repl();
        }while(true);
__SUBSCR_END:
        if(is_slice){
            e->b = std::move(slice);
        }else{
            if(state != 1) FATAL_ERROR();
            e->b = std::move(slice->start);
        }
        ctx()->s_expr.push(std::move(e));
    }

    void Compiler::exprLiteral0() {
        ctx()->s_expr.push(make_expr<Literal0Expr>(prev().type));
    }

    void Compiler::compile_block_body() {
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

    // import a [as b]
    // import a [as b], c [as d]
    void Compiler::compile_normal_import() {
        do {
            consume(TK("@id"));
            Str name = prev().str();
            ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const(VAR(name)), prev().line);
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

        ctx()->emit_(OP_IMPORT_PATH, ctx()->add_const(VAR(ss.str())), prev().line);
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

    bool Compiler::is_expression(){
        PrattCallback prefix = rules[curr().type].prefix;
        return prefix != nullptr;
    }

    void Compiler::parse_expression(int precedence, bool push_stack) {
        PrattCallback prefix = rules[curr().type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, got ") + TK_STR(curr().type));
        advance();
        (this->*prefix)();
        while (rules[curr().type].precedence >= precedence) {
            TokenIndex op = curr().type;
            advance();
            PrattCallback infix = rules[op].infix;
            PK_ASSERT(infix != nullptr);
            (this->*infix)();
        }
        if(!push_stack) ctx()->emit_expr();
    }

    void Compiler::compile_if_stmt() {
        EXPR(false);   // condition
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
        CodeBlock* block = ctx()->enter_block(WHILE_LOOP);
        EXPR(false);   // condition
        int patch = ctx()->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, prev().line);
        compile_block_body();
        ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE);
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
        EXPR_TUPLE(false);
        ctx()->emit_(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        CodeBlock* block = ctx()->enter_block(FOR_LOOP);
        ctx()->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx());
        if(!ok) SyntaxError();  // this error occurs in `vars` instead of this line, but...nevermind
        compile_block_body();
        ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE);
        ctx()->exit_block();
        // optional else clause
        if (match(TK("else"))) {
            compile_block_body();
            block->end2 = ctx()->co->codes.size();
        }
    }

    void Compiler::compile_try_except() {
        ctx()->enter_block(TRY_EXCEPT);
        compile_block_body();
        std::vector<int> patches = {
            ctx()->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, BC_KEEPLINE)
        };
        ctx()->exit_block();
        do {
            StrName as_name;
            consume(TK("except"));
            if(match(TK("@id"))){
                ctx()->emit_(OP_EXCEPTION_MATCH, StrName(prev().sv()).index, prev().line);
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
        // no match, re-raise
        ctx()->emit_(OP_RE_RAISE, BC_NOARG, BC_KEEPLINE);
        for (int patch : patches) ctx()->patch_jump(patch);
    }

    void Compiler::compile_decorated(){
        std::vector<Expr_> decorators;
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
                EXPR_TUPLE(false);
                // if yield present, mark the function as generator
                ctx()->co->is_generator = true;
                ctx()->emit_(OP_YIELD_VALUE, BC_NOARG, kw_line);
                consume_end_stmt();
                break;
            case TK("yield from"):
                if (contexts.size() <= 1) SyntaxError("'yield from' outside function");
                EXPR_TUPLE(false);
                // if yield from present, mark the function as generator
                ctx()->co->is_generator = true;
                ctx()->emit_(OP_GET_ITER, BC_NOARG, kw_line);
                ctx()->enter_block(FOR_LOOP);
                ctx()->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
                ctx()->emit_(OP_YIELD_VALUE, BC_NOARG, BC_KEEPLINE);
                ctx()->emit_(OP_LOOP_CONTINUE, ctx()->get_loop(), BC_KEEPLINE);
                ctx()->exit_block();
                consume_end_stmt();
                break;
            case TK("return"):
                if (contexts.size() <= 1) SyntaxError("'return' outside function");
                if(match_end_stmt()){
                    ctx()->emit_(OP_LOAD_NONE, BC_NOARG, kw_line);
                }else{
                    EXPR_TUPLE(false);
                    consume_end_stmt();
                }
                ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, kw_line);
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
                EXPR(false);    // condition
                int index = ctx()->emit_(OP_POP_JUMP_IF_TRUE, BC_NOARG, kw_line);
                int has_msg = 0;
                if(match(TK(","))){
                    EXPR(false);    // message
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
                consume(TK("@id"));
                int dummy_t = StrName(prev().sv()).index;
                if(match(TK("(")) && !match(TK(")"))){
                    EXPR(false); consume(TK(")"));
                }else{
                    ctx()->emit_(OP_LOAD_NONE, BC_NOARG, kw_line);
                }
                ctx()->emit_(OP_RAISE, dummy_t, kw_line);
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
                EXPR(false);
                consume(TK("as"));
                consume(TK("@id"));
                Expr_ e = make_expr<NameExpr>(prev().str(), name_scope());
                bool ok = e->emit_store(ctx());
                if(!ok) SyntaxError();
                e->emit_(ctx());
                ctx()->emit_(OP_WITH_ENTER, BC_NOARG, prev().line);
                compile_block_body();
                e->emit_(ctx());
                ctx()->emit_(OP_WITH_EXIT, BC_NOARG, prev().line);
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
                            // add to __annotations__
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

    void Compiler::_add_decorators(const std::vector<Expr_>& decorators){
        // [obj]
        for(auto it=decorators.rbegin(); it!=decorators.rend(); ++it){
            (*it)->emit_(ctx());                                    // [obj, f]
            ctx()->emit_(OP_ROT_TWO, BC_NOARG, (*it)->line);        // [f, obj]
            ctx()->emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);      // [f, obj, NULL]
            ctx()->emit_(OP_ROT_TWO, BC_NOARG, BC_KEEPLINE);        // [obj, NULL, f]
            ctx()->emit_(OP_CALL, 1, (*it)->line);                  // [obj]
        }
    }

    void Compiler::compile_class(const std::vector<Expr_>& decorators){
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

        for(auto& c: this->contexts.data()){
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

    void Compiler::compile_function(const std::vector<Expr_>& decorators){
        const char* _start = curr().start;
        consume(TK("@id"));
        Str decl_name = prev().str();
        FuncDecl_ decl = push_f_context(decl_name);
        consume(TK("("));
        if (!match(TK(")"))) {
            _compile_f_args(decl, true);
            consume(TK(")"));
        }
        if(match(TK("->"))) consume_type_hints();
        const char* _end = curr().start;
        decl->signature = Str(_start, _end-_start);
        compile_block_body();
        pop_context();

        PyObject* docstring = nullptr;
        if(decl->code->codes.size()>=2 && decl->code->codes[0].op == OP_LOAD_CONST && decl->code->codes[1].op == OP_POP_TOP){
            PyObject* c = decl->code->consts[decl->code->codes[0].arg];
            if(is_type(c, vm->tp_str)){
                decl->code->codes[0].op = OP_NO_OP;
                decl->code->codes[1].op = OP_NO_OP;
                docstring = c;
            }
        }
        if(docstring != nullptr){
            decl->docstring = PK_OBJ_GET(Str, docstring);
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
        if(obj == nullptr) FATAL_ERROR();
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
            default: break;
        }
        return nullptr;
    }

    Compiler::Compiler(VM* vm, const Str& source, const Str& filename, CompileMode mode, bool unknown_global_scope){
        this->vm = vm;
        this->used = false;
        this->unknown_global_scope = unknown_global_scope;
        this->lexer = std::make_unique<Lexer>(
            std::make_shared<SourceData>(source, filename, mode)
        );
        init_pratt_rules();
    }


    CodeObject_ Compiler::compile(){
        if(used) FATAL_ERROR();
        used = true;

        tokens = lexer->run();
        CodeObject_ code = push_global_context();

        advance();          // skip @sof, so prev() is always valid
        match_newlines();   // skip possible leading '\n'

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE(false);
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

}   // namespace pkpy