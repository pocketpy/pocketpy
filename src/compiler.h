#pragma once

#include "codeobject.h"
#include "common.h"
#include "lexer.h"
#include "error.h"
#include "ceval.h"

namespace pkpy{

class Compiler;
typedef void (Compiler::*GrammarFn)();
typedef void (Compiler::*CompilerAction)();

struct GrammarRule{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
};

class Compiler {
    std::unique_ptr<Lexer> lexer;
    stack<CodeObject_> codes;
    bool used = false;
    VM* vm;
    std::map<TokenIndex, GrammarRule> rules;

    CodeObject_ co() const{ return codes.top(); }
    CompileMode mode() const{ return lexer->src->mode; }
    NameScope name_scope() const { return codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL; }

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
        rules[TK("(")] =    { METHOD(exprGrouping),  METHOD(exprCall),           PREC_CALL };
        rules[TK("[")] =    { METHOD(exprList),      METHOD(exprSubscript),      PREC_SUBSCRIPT };
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
        rules[TK("True")] =     { METHOD(exprValue),     NO_INFIX };
        rules[TK("False")] =    { METHOD(exprValue),     NO_INFIX };
        rules[TK("lambda")] =   { METHOD(exprLambda),    NO_INFIX };
        rules[TK("None")] =     { METHOD(exprValue),     NO_INFIX };
        rules[TK("...")] =      { METHOD(exprValue),     NO_INFIX };
        rules[TK("@id")] =      { METHOD(exprName),      NO_INFIX };
        rules[TK("@num")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@str")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@fstr")] =    { METHOD(exprFString),   NO_INFIX };
        rules[TK("?")] =        { nullptr,               METHOD(exprTernary),        PREC_TERNARY };
        rules[TK("=")] =        { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("+=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("-=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("*=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("/=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("//=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("%=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("&=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("|=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("^=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK(">>=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("<<=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK(",")] =        { nullptr,               METHOD(exprComma),          PREC_COMMA };
        rules[TK("<<")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
#undef METHOD
#undef NO_INFIX

#define EXPR() parse_expression(PREC_TERNARY)             // no '=' and ',' just a simple expression
#define EXPR_TUPLE() parse_expression(PREC_COMMA)         // no '=', but ',' is allowed
#define EXPR_ANY() parse_expression(PREC_ASSIGNMENT)
    }

private:
    int i = 0;
    std::vector<Token> tokens;

    const Token& prev() { return tokens.at(i-1); }
    const Token& curr() { return tokens.at(i); }
    const Token& next() { return tokens.at(i+1); }
    const Token& peek(int offset=0) { return tokens.at(i+offset); }
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
            throw NeedMoreLines(co()->_is_compiling_class);
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

    PyObject* get_value(const Token& token) {
        switch (token.type) {
            case TK("@num"):
                if(std::holds_alternative<i64>(token.value)) return VAR(std::get<i64>(token.value));
                if(std::holds_alternative<f64>(token.value)) return VAR(std::get<f64>(token.value));
                UNREACHABLE();
            case TK("@str"): case TK("@fstr"):
                return VAR(std::get<Str>(token.value));
            default: throw std::runtime_error(Str("invalid token type: ") + TK_STR(token.type));
        }
    }

    void exprLiteral() {
        PyObject* value = get_value(prev());
        int index = co()->add_const(value);
        emit(OP_LOAD_CONST, index);
    }

    void exprFString() {
        static const std::regex pattern(R"(\{(.*?)\})");
        PyObject* value = get_value(prev());
        Str s = CAST(Str, value);
        std::sregex_iterator begin(s.begin(), s.end(), pattern);
        std::sregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::smatch m = *it;
            if (i < m.position()) {
                std::string literal = s.substr(i, m.position() - i);
                emit(OP_LOAD_CONST, co()->add_const(VAR(literal)));
                size++;
            }
            emit(OP_LOAD_EVAL_FN);
            emit(OP_LOAD_CONST, co()->add_const(VAR(m[1].str())));
            emit(OP_CALL, 1);
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < s.size()) {
            std::string literal = s.substr(i, s.size() - i);
            emit(OP_LOAD_CONST, co()->add_const(VAR(literal)));
            size++;
        }
        emit(OP_BUILD_STRING, size);
    }

    void exprLambda() {
        Function func;
        func.name = "<lambda>";
        if(!match(TK(":"))){
            _compile_f_args(func, false);
            consume(TK(":"));
        }
        func.code = make_sp<CodeObject>(lexer->src, func.name.str());
        this->codes.push(func.code);
        co()->_rvalue += 1; EXPR(); co()->_rvalue -= 1;
        emit(OP_RETURN_VALUE);
        func.code->optimize(vm);
        this->codes.pop();
        emit(OP_LOAD_FUNCTION, co()->add_const(VAR(func)));
        if(name_scope() == NAME_LOCAL) emit(OP_SETUP_CLOSURE);
    }

    void exprAssign() {
        if(co()->codes.empty()) UNREACHABLE();
        bool is_load_name_ref = co()->codes.back().op == OP_LOAD_NAME_REF;
        int _name_arg = co()->codes.back().arg;
        // if the last op is OP_LOAD_NAME_REF, remove it
        // because we will emit OP_STORE_NAME or OP_STORE_CLASS_ATTR
        if(is_load_name_ref) co()->codes.pop_back();

        co()->_rvalue += 1;
        TokenIndex op = prev().type;
        if(op == TK("=")) {     // a = (expr)
            EXPR_TUPLE();
            if(is_load_name_ref){
                auto op = co()->_is_compiling_class ? OP_STORE_CLASS_ATTR : OP_STORE_NAME;
                emit(op, _name_arg);
            }else{
                if(co()->_is_compiling_class) SyntaxError();
                emit(OP_STORE_REF);
            }
        }else{                  // a += (expr) -> a = a + (expr)
            if(co()->_is_compiling_class) SyntaxError();
            if(is_load_name_ref){
                emit(OP_LOAD_NAME, _name_arg);
            }else{
                emit(OP_DUP_TOP_VALUE);
            }
            EXPR();
            switch (op) {
                case TK("+="):      emit(OP_BINARY_OP, 0);  break;
                case TK("-="):      emit(OP_BINARY_OP, 1);  break;
                case TK("*="):      emit(OP_BINARY_OP, 2);  break;
                case TK("/="):      emit(OP_BINARY_OP, 3);  break;
                case TK("//="):     emit(OP_BINARY_OP, 4);  break;
                case TK("%="):      emit(OP_BINARY_OP, 5);  break;
                case TK("<<="):     emit(OP_BITWISE_OP, 0);  break;
                case TK(">>="):     emit(OP_BITWISE_OP, 1);  break;
                case TK("&="):      emit(OP_BITWISE_OP, 2);  break;
                case TK("|="):      emit(OP_BITWISE_OP, 3);  break;
                case TK("^="):      emit(OP_BITWISE_OP, 4);  break;
                default: UNREACHABLE();
            }
            if(is_load_name_ref){
                emit(OP_STORE_NAME, _name_arg);
            }else{
                emit(OP_STORE_REF);
            }
        }
        co()->_rvalue -= 1;
    }

    void exprComma() {
        int size = 1;       // an expr is in the stack now
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            size++;
        } while(match(TK(",")));
        emit(co()->_rvalue ? OP_BUILD_TUPLE : OP_BUILD_TUPLE_REF, size);
    }

    void exprOr() {
        int patch = emit(OP_JUMP_IF_TRUE_OR_POP);
        parse_expression(PREC_LOGICAL_OR);
        patch_jump(patch);
    }

    void exprAnd() {
        int patch = emit(OP_JUMP_IF_FALSE_OR_POP);
        parse_expression(PREC_LOGICAL_AND);
        patch_jump(patch);
    }

    void exprTernary() {
        int patch = emit(OP_POP_JUMP_IF_FALSE);
        EXPR();         // if true
        int patch2 = emit(OP_JUMP_ABSOLUTE);
        consume(TK(":"));
        patch_jump(patch);
        EXPR();         // if false
        patch_jump(patch2);
    }

    void exprBinaryOp() {
        TokenIndex op = prev().type;
        parse_expression((Precedence)(rules[op].precedence + 1));

        switch (op) {
            case TK("+"):   emit(OP_BINARY_OP, 0);  break;
            case TK("-"):   emit(OP_BINARY_OP, 1);  break;
            case TK("*"):   emit(OP_BINARY_OP, 2);  break;
            case TK("/"):   emit(OP_BINARY_OP, 3);  break;
            case TK("//"):  emit(OP_BINARY_OP, 4);  break;
            case TK("%"):   emit(OP_BINARY_OP, 5);  break;
            case TK("**"):  emit(OP_BINARY_OP, 6);  break;

            case TK("<"):   emit(OP_COMPARE_OP, 0);    break;
            case TK("<="):  emit(OP_COMPARE_OP, 1);    break;
            case TK("=="):  emit(OP_COMPARE_OP, 2);    break;
            case TK("!="):  emit(OP_COMPARE_OP, 3);    break;
            case TK(">"):   emit(OP_COMPARE_OP, 4);    break;
            case TK(">="):  emit(OP_COMPARE_OP, 5);    break;
            case TK("in"):      emit(OP_CONTAINS_OP, 0);   break;
            case TK("not in"):  emit(OP_CONTAINS_OP, 1);   break;
            case TK("is"):      emit(OP_IS_OP, 0);         break;
            case TK("is not"):  emit(OP_IS_OP, 1);         break;

            case TK("<<"):  emit(OP_BITWISE_OP, 0);    break;
            case TK(">>"):  emit(OP_BITWISE_OP, 1);    break;
            case TK("&"):   emit(OP_BITWISE_OP, 2);    break;
            case TK("|"):   emit(OP_BITWISE_OP, 3);    break;
            case TK("^"):   emit(OP_BITWISE_OP, 4);    break;
            default: UNREACHABLE();
        }
    }

    void exprNot() {
        parse_expression((Precedence)(PREC_LOGICAL_NOT + 1));
        emit(OP_UNARY_NOT);
    }

    void exprUnaryOp() {
        TokenIndex op = prev().type;
        parse_expression((Precedence)(PREC_UNARY + 1));
        switch (op) {
            case TK("-"):     emit(OP_UNARY_NEGATIVE); break;
            case TK("*"):     emit(OP_UNARY_STAR, co()->_rvalue);   break;
            default: UNREACHABLE();
        }
    }

    void exprGrouping() {
        match_newlines(mode()==REPL_MODE);
        EXPR_TUPLE();
        match_newlines(mode()==REPL_MODE);
        consume(TK(")"));
    }

    void _consume_comp(Opcode op0, Opcode op1, int _patch, int _body_start){
        int _body_end_return = emit(OP_JUMP_ABSOLUTE, -1);
        int _body_end = co()->codes.size();
        co()->codes[_patch].op = OP_JUMP_ABSOLUTE;
        co()->codes[_patch].arg = _body_end;
        emit(op0, 0);
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        match_newlines(mode()==REPL_MODE);
        
        int _skipPatch = emit(OP_JUMP_ABSOLUTE);
        int _cond_start = co()->codes.size();
        int _cond_end_return = -1;
        if(match(TK("if"))) {
            EXPR_TUPLE();
            _cond_end_return = emit(OP_JUMP_ABSOLUTE, -1);
        }
        patch_jump(_skipPatch);

        emit(OP_GET_ITER);
        co()->_enter_block(FOR_LOOP);
        emit(OP_FOR_ITER);

        if(_cond_end_return != -1) {      // there is an if condition
            emit(OP_JUMP_ABSOLUTE, _cond_start);
            patch_jump(_cond_end_return);
            int ifpatch = emit(OP_POP_JUMP_IF_FALSE);
            emit(OP_JUMP_ABSOLUTE, _body_start);
            patch_jump(_body_end_return);
            emit(op1);
            patch_jump(ifpatch);
        }else{
            emit(OP_JUMP_ABSOLUTE, _body_start);
            patch_jump(_body_end_return);
            emit(op1);
        }

        emit(OP_LOOP_CONTINUE, -1, true);
        co()->_exit_block();
        match_newlines(mode()==REPL_MODE);
    }

    void exprList() {
        int _patch = emit(OP_NO_OP);
        int _body_start = co()->codes.size();
        int ARGC = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (curr().type == TK("]")) break;
            EXPR(); ARGC++;
            match_newlines(mode()==REPL_MODE);
            if(ARGC == 1 && match(TK("for"))){
                _consume_comp(OP_BUILD_LIST, OP_LIST_APPEND, _patch, _body_start);
                consume(TK("]"));
                return;
            }
        } while (match(TK(",")));
        match_newlines(mode()==REPL_MODE);
        consume(TK("]"));
        emit(OP_BUILD_LIST, ARGC);
    }

    void exprMap() {
        int _patch = emit(OP_NO_OP);
        int _body_start = co()->codes.size();
        bool parsing_dict = false;
        int ARGC = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (curr().type == TK("}")) break;
            EXPR();
            if(curr().type == TK(":")) parsing_dict = true;
            if(parsing_dict){
                consume(TK(":"));
                EXPR();
            }
            ARGC++;
            match_newlines(mode()==REPL_MODE);
            if(ARGC == 1 && match(TK("for"))){
                if(parsing_dict) _consume_comp(OP_BUILD_MAP, OP_MAP_ADD, _patch, _body_start);
                else _consume_comp(OP_BUILD_SET, OP_SET_ADD, _patch, _body_start);
                consume(TK("}"));
                return;
            }
        } while (match(TK(",")));
        consume(TK("}"));

        if(ARGC == 0 || parsing_dict) emit(OP_BUILD_MAP, ARGC);
        else emit(OP_BUILD_SET, ARGC);
    }

    void exprCall() {
        int ARGC = 0;
        int KWARGC = 0;
        bool need_unpack = false;
        do {
            match_newlines(mode()==REPL_MODE);
            if (curr().type == TK(")")) break;
            if(curr().type == TK("@id") && next().type == TK("=")) {
                consume(TK("@id"));
                const Str& key = prev().str();
                emit(OP_LOAD_CONST, co()->add_const(VAR(key)));
                consume(TK("="));
                co()->_rvalue += 1; EXPR(); co()->_rvalue -= 1;
                KWARGC++;
            } else{
                if(KWARGC > 0) SyntaxError("positional argument follows keyword argument");
                co()->_rvalue += 1; EXPR(); co()->_rvalue -= 1;
                if(co()->codes.back().op == OP_UNARY_STAR) need_unpack = true;
                ARGC++;
            }
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK(")"));
        if(ARGC > 32767) SyntaxError("too many positional arguments");
        if(KWARGC > 32767) SyntaxError("too many keyword arguments");
        if(KWARGC > 0){
            emit(need_unpack ? OP_CALL_KWARGS_UNPACK : OP_CALL_KWARGS, (KWARGC << 16) | ARGC);
        }else{
            emit(need_unpack ? OP_CALL_UNPACK : OP_CALL, ARGC);
        }
    }

    void exprName(){ _exprName(false); }

    void _exprName(bool force_lvalue) {
        const Token& tkname = prev();
        int index = co()->add_name(tkname.str(), name_scope());
        bool fast_load = !force_lvalue && co()->_rvalue>0;
        emit(fast_load ? OP_LOAD_NAME : OP_LOAD_NAME_REF, index);
    }

    void exprAttrib() {
        consume(TK("@id"));
        const Str& name = prev().str();
        int index = co()->add_name(name, NAME_ATTR);
        emit(co()->_rvalue ? OP_BUILD_ATTR : OP_BUILD_ATTR_REF, index);
    }

    // [:], [:b]
    // [a], [a:], [a:b]
    void exprSubscript() {
        if(match(TK(":"))){
            emit(OP_LOAD_NONE);
            if(match(TK("]"))){
                emit(OP_LOAD_NONE);
            }else{
                EXPR_TUPLE();
                consume(TK("]"));
            }
            emit(OP_BUILD_SLICE);
        }else{
            EXPR_TUPLE();
            if(match(TK(":"))){
                if(match(TK("]"))){
                    emit(OP_LOAD_NONE);
                }else{
                    EXPR_TUPLE();
                    consume(TK("]"));
                }
                emit(OP_BUILD_SLICE);
            }else{
                consume(TK("]"));
            }
        }

        emit(OP_BUILD_INDEX, (int)(co()->_rvalue>0));
    }

    void exprValue() {
        TokenIndex op = prev().type;
        switch (op) {
            case TK("None"):    emit(OP_LOAD_NONE);  break;
            case TK("True"):    emit(OP_LOAD_TRUE);  break;
            case TK("False"):   emit(OP_LOAD_FALSE); break;
            case TK("..."):     emit(OP_LOAD_ELLIPSIS); break;
            default: UNREACHABLE();
        }
    }

    int emit(Opcode opcode, int arg=-1, bool keepline=false) {
        int line = prev().line;
        co()->codes.push_back(
            Bytecode{(uint8_t)opcode, (uint16_t)co()->_curr_block_i, arg, line}
        );
        int i = co()->codes.size() - 1;
        if(keepline && i>=1) co()->codes[i].line = co()->codes[i-1].line;
        return i;
    }

    inline void patch_jump(int addr_index) {
        int target = co()->codes.size();
        co()->codes[addr_index].arg = target;
    }

    void compile_block_body(CompilerAction action=nullptr) {
        if(action == nullptr) action = &Compiler::compile_stmt;
        consume(TK(":"));
        if(curr().type!=TK("@eol") && curr().type!=TK("@eof")){
            (this->*action)();  // inline block
            return;
        }
        if(!match_newlines(mode()==REPL_MODE)){
            SyntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (curr().type != TK("@dedent")) {
            match_newlines();
            (this->*action)();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    Token _compile_import() {
        consume(TK("@id"));
        Token tkmodule = prev();
        int index = co()->add_name(tkmodule.str(), NAME_SPECIAL);
        emit(OP_IMPORT_NAME, index);
        return tkmodule;
    }

    // import a as b
    void compile_normal_import() {
        do {
            Token tkmodule = _compile_import();
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkmodule = prev();
            }
            int index = co()->add_name(tkmodule.str(), name_scope());
            emit(OP_STORE_NAME, index);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b as c, d as e
    void compile_from_import() {
        _compile_import();
        consume(TK("import"));
        if (match(TK("*"))) {
            if(name_scope() != NAME_GLOBAL) SyntaxError("import * can only be used in global scope");
            emit(OP_STORE_ALL_NAMES);
            consume_end_stmt();
            return;
        }
        do {
            emit(OP_DUP_TOP_VALUE);
            consume(TK("@id"));
            Token tkname = prev();
            int index = co()->add_name(tkname.str(), NAME_ATTR);
            emit(OP_BUILD_ATTR, index);
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkname = prev();
            }
            index = co()->add_name(tkname.str(), name_scope());
            emit(OP_STORE_NAME, index);
        } while (match(TK(",")));
        emit(OP_POP_TOP);
        consume_end_stmt();
    }

    // a = 1 + 2
    // ['a', '1', '2', '+', '=']
    // 
    void parse_expression(Precedence precedence) {
        advance();
        GrammarFn prefix = rules[prev().type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, but got ") + TK_STR(prev().type));
        (this->*prefix)();
        bool meet_assign_token = false;
        while (rules[curr().type].precedence >= precedence) {
            advance();
            TokenIndex op = prev().type;
            if (op == TK("=")){
                if(meet_assign_token) SyntaxError();
                meet_assign_token = true;
            }
            GrammarFn infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
    }

    void compile_if_stmt() {
        match_newlines();
        co()->_rvalue += 1;
        EXPR_TUPLE();   // condition
        co()->_rvalue -= 1;
        int ifpatch = emit(OP_POP_JUMP_IF_FALSE);
        compile_block_body();

        if (match(TK("elif"))) {
            int exit_jump = emit(OP_JUMP_ABSOLUTE);
            patch_jump(ifpatch);
            compile_if_stmt();
            patch_jump(exit_jump);
        } else if (match(TK("else"))) {
            int exit_jump = emit(OP_JUMP_ABSOLUTE);
            patch_jump(ifpatch);
            compile_block_body();
            patch_jump(exit_jump);
        } else {
            patch_jump(ifpatch);
        }
    }

    void compile_while_loop() {
        co()->_enter_block(WHILE_LOOP);
        co()->_rvalue += 1;
        EXPR_TUPLE();   // condition
        co()->_rvalue -= 1;
        int patch = emit(OP_POP_JUMP_IF_FALSE);
        compile_block_body();
        emit(OP_LOOP_CONTINUE, -1, true);
        patch_jump(patch);
        co()->_exit_block();
    }

    void EXPR_FOR_VARS(){
        int size = 0;
        do {
            consume(TK("@id"));
            _exprName(true); size++;
        } while (match(TK(",")));
        if(size > 1) emit(OP_BUILD_TUPLE_REF, size);
    }

    void compile_for_loop() {
        EXPR_FOR_VARS();consume(TK("in"));
        co()->_rvalue += 1; EXPR_TUPLE(); co()->_rvalue -= 1;
        emit(OP_GET_ITER);
        co()->_enter_block(FOR_LOOP);
        emit(OP_FOR_ITER);
        compile_block_body();
        emit(OP_LOOP_CONTINUE, -1, true);
        co()->_exit_block();
    }

    void compile_try_except() {
        co()->_enter_block(TRY_EXCEPT);
        emit(OP_TRY_BLOCK_ENTER);
        compile_block_body();
        emit(OP_TRY_BLOCK_EXIT);
        std::vector<int> patches = { emit(OP_JUMP_ABSOLUTE) };
        co()->_exit_block();

        do {
            consume(TK("except"));
            if(match(TK("@id"))){
                int name_idx = co()->add_name(prev().str(), NAME_SPECIAL);
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

    void compile_stmt() {
        if (match(TK("break"))) {
            if (!co()->_is_curr_block_loop()) SyntaxError("'break' outside loop");
            consume_end_stmt();
            emit(OP_LOOP_BREAK);
        } else if (match(TK("continue"))) {
            if (!co()->_is_curr_block_loop()) SyntaxError("'continue' not properly in loop");
            consume_end_stmt();
            emit(OP_LOOP_CONTINUE);
        } else if (match(TK("yield"))) {
            if (codes.size() == 1) SyntaxError("'yield' outside function");
            co()->_rvalue += 1;
            EXPR_TUPLE();
            co()->_rvalue -= 1;
            consume_end_stmt();
            co()->is_generator = true;
            emit(OP_YIELD_VALUE, -1, true);
        } else if (match(TK("return"))) {
            if (codes.size() == 1) SyntaxError("'return' outside function");
            if(match_end_stmt()){
                emit(OP_LOAD_NONE);
            }else{
                co()->_rvalue += 1;
                EXPR_TUPLE();   // return value
                co()->_rvalue -= 1;
                consume_end_stmt();
            }
            emit(OP_RETURN_VALUE, -1, true);
        } else if (match(TK("if"))) {
            compile_if_stmt();
        } else if (match(TK("while"))) {
            compile_while_loop();
        } else if (match(TK("for"))) {
            compile_for_loop();
        } else if (match(TK("import"))){
            compile_normal_import();
        } else if (match(TK("from"))){
            compile_from_import();
        } else if (match(TK("def"))){
            compile_function();
        } else if (match(TK("@"))){
            EXPR();
            if(!match_newlines(mode()==REPL_MODE)){
                SyntaxError("expected a new line after '@'");
            }
            emit(OP_SETUP_DECORATOR);
            consume(TK("def"));
            compile_function();
        } else if (match(TK("try"))) {
            compile_try_except();
        } else if(match(TK("assert"))) {
            co()->_rvalue += 1;
            EXPR();
            if (match(TK(","))) EXPR();
            else emit(OP_LOAD_CONST, co()->add_const(VAR("")));
            co()->_rvalue -= 1;
            emit(OP_ASSERT);
            consume_end_stmt();
        } else if(match(TK("with"))){
            EXPR();
            consume(TK("as"));
            consume(TK("@id"));
            Token tkname = prev();
            int index = co()->add_name(tkname.str(), name_scope());
            emit(OP_STORE_NAME, index);
            emit(OP_LOAD_NAME_REF, index);
            emit(OP_WITH_ENTER);
            compile_block_body();
            emit(OP_LOAD_NAME_REF, index);
            emit(OP_WITH_EXIT);
        } else if(match(TK("label"))){
            if(mode() != EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            Str label = prev().str();
            bool ok = co()->add_label(label);
            if(!ok) SyntaxError("label '" + label + "' already exists");
            consume_end_stmt();
        } else if(match(TK("goto"))){ // https://entrian.com/goto/
            if(mode() != EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            emit(OP_GOTO, co()->add_name(prev().str(), NAME_SPECIAL));
            consume_end_stmt();
        } else if(match(TK("raise"))){
            consume(TK("@id"));
            int dummy_t = co()->add_name(prev().str(), NAME_SPECIAL);
            if(match(TK("(")) && !match(TK(")"))){
                EXPR(); consume(TK(")"));
            }else{
                emit(OP_LOAD_NONE);
            }
            emit(OP_RAISE, dummy_t);
            consume_end_stmt();
        } else if(match(TK("del"))){
            EXPR_TUPLE();
            emit(OP_DELETE_REF);
            consume_end_stmt();
        } else if(match(TK("global"))){
            do {
                consume(TK("@id"));
                co()->global_names[prev().str()] = 1;
            } while (match(TK(",")));
            consume_end_stmt();
        } else if(match(TK("pass"))){
            consume_end_stmt();
        } else {
            int begin = co()->codes.size();
            EXPR_ANY();
            int end = co()->codes.size();
            consume_end_stmt();
            // If last op is not an assignment, pop the result.
            uint8_t last_op = co()->codes.back().op;
            if( last_op!=OP_STORE_NAME && last_op!=OP_STORE_REF &&
            last_op!=OP_STORE_ALL_NAMES && last_op!=OP_STORE_CLASS_ATTR){
                for(int i=begin; i<end; i++){
                    if(co()->codes[i].op==OP_BUILD_TUPLE_REF) co()->codes[i].op = OP_BUILD_TUPLE;
                }
                if(mode()==REPL_MODE && name_scope() == NAME_GLOBAL) emit(OP_PRINT_EXPR, -1, true);
                emit(OP_POP_TOP, -1, true);
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
        co()->_is_compiling_class = true;
        compile_block_body();
        co()->_is_compiling_class = false;
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
        bool has_decorator = !co()->codes.empty() && co()->codes.back().op == OP_SETUP_DECORATOR;
        Function func;
        StrName obj_name;
        consume(TK("@id"));
        func.name = prev().str();
        if(!co()->_is_compiling_class && match(TK("::"))){
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
        func.code = make_sp<CodeObject>(lexer->src, func.name.str());
        this->codes.push(func.code);
        compile_block_body();
        func.code->optimize(vm);
        this->codes.pop();
        emit(OP_LOAD_FUNCTION, co()->add_const(VAR(func)));
        if(name_scope() == NAME_LOCAL) emit(OP_SETUP_CLOSURE);
        if(!co()->_is_compiling_class){
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

public:
    CodeObject_ compile(){
        // can only be called once
        if(used) UNREACHABLE();
        used = true;

        tokens = lexer->run();
        // if(lexer->src->filename == "tests/01_int.py"){
        //     for(auto& t: tokens) std::cout << t.info() << std::endl;
        // }

        CodeObject_ code = make_sp<CodeObject>(lexer->src, lexer->src->filename);
        codes.push(code);

        advance();          // skip @sof, so prev() is always valid
        match_newlines();   // skip leading '\n'

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            code->optimize(vm);
            return code;
        }else if(mode()==JSON_MODE){
            PyObject* value = read_literal();
            if(value != nullptr) emit(OP_LOAD_CONST, code->add_const(value));
            else if(match(TK("{"))) exprMap();
            else if(match(TK("["))) exprList();
            else SyntaxError("expect a JSON object or array");
            consume(TK("@eof"));
            return code;    // no need to optimize for JSON decoding
        }

        while (!match(TK("@eof"))) {
            if (match(TK("class"))) {
                compile_class();
            } else {
                compile_stmt();
            }
            match_newlines();
        }
        code->optimize(vm);
        return code;
    }
};

} // namespace pkpy