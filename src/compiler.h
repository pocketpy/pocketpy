#pragma once

#include "parser.h"
#include "error.h"
#include "vm.h"

class Compiler;

typedef void (Compiler::*GrammarFn)();
typedef void (Compiler::*CompilerAction)();

struct GrammarRule{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
};

enum StringType { NORMAL_STRING, RAW_STRING, F_STRING };

class Compiler {
    std::unique_ptr<Parser> parser;
    std::stack<CodeObject_> codes;
    bool is_compiling_class = false;
    int lexing_count = 0;
    bool used = false;
    VM* vm;
    emhash8::HashMap<TokenIndex, GrammarRule> rules;

    CodeObject_ co() const{ return codes.top(); }
    CompileMode mode() const{ return parser->src->mode; }

public:
    Compiler(VM* vm, const char* source, Str filename, CompileMode mode){
        this->vm = vm;
        this->parser = std::make_unique<Parser>(
            pkpy::make_shared<SourceData>(source, filename, mode)
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
        rules[TK("not")] =      { METHOD(exprUnaryOp),   nullptr,                    PREC_UNARY };
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
    Str eat_string_until(char quote, bool raw) {
        bool quote3 = parser->match_n_chars(2, quote);
        std::vector<char> buff;
        while (true) {
            char c = parser->eatchar_include_newline();
            if (c == quote){
                if(quote3 && !parser->match_n_chars(2, quote)){
                    buff.push_back(c);
                    continue;
                }
                break;
            }
            if (c == '\0'){
                if(quote3 && parser->src->mode == REPL_MODE){
                    throw NeedMoreLines(false);
                }
                SyntaxError("EOL while scanning string literal");
            }
            if (c == '\n'){
                if(!quote3) SyntaxError("EOL while scanning string literal");
                else{
                    buff.push_back(c);
                    continue;
                }
            }
            if (!raw && c == '\\') {
                switch (parser->eatchar_include_newline()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    default: SyntaxError("invalid escape character");
                }
            } else {
                buff.push_back(c);
            }
        }
        return Str(buff.data(), buff.size());
    }

    void eat_string(char quote, StringType type) {
        Str s = eat_string_until(quote, type == RAW_STRING);
        if(type == F_STRING){
            parser->set_next_token(TK("@fstr"), vm->PyStr(s));
        }else{
            parser->set_next_token(TK("@str"), vm->PyStr(s));
        }
    }

    void eat_number() {
        static const std::regex pattern("^(0x)?[0-9a-fA-F]+(\\.[0-9]+)?");
        std::smatch m;

        const char* i = parser->token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(parser->token_start, i);

        try{
            if (std::regex_search(s, m, pattern)) {
                // here is m.length()-1, since the first char was eaten by lex_token()
                for(int j=0; j<m.length()-1; j++) parser->eatchar();

                int base = 10;
                size_t size;
                if (m[1].matched) base = 16;
                if (m[2].matched) {
                    if(base == 16) SyntaxError("hex literal should not contain a dot");
                    parser->set_next_token(TK("@num"), vm->PyFloat(std::stod(m[0], &size)));
                } else {
                    parser->set_next_token(TK("@num"), vm->PyInt(std::stoll(m[0], &size, base)));
                }
                if (size != m.length()) UNREACHABLE();
            }
        }catch(std::exception& _){
            SyntaxError("invalid number literal");
        } 
    }

    void lex_token(){
        lexing_count++;
        _lex_token();
        lexing_count--;
    }

    // Lex the next token and set it as the next token.
    void _lex_token() {
        parser->prev = parser->curr;
        parser->curr = parser->next_token();

        //Str _info = parser->curr.info(); std::cout << _info << '[' << parser->current_line << ']' << std::endl;

        while (parser->peekchar() != '\0') {
            parser->token_start = parser->curr_char;
            char c = parser->eatchar_include_newline();
            switch (c) {
                case '\'': case '"': eat_string(c, NORMAL_STRING); return;
                case '#': parser->skip_line_comment(); break;
                case '{': parser->set_next_token(TK("{")); return;
                case '}': parser->set_next_token(TK("}")); return;
                case ',': parser->set_next_token(TK(",")); return;
                case ':': parser->set_next_token(TK(":")); return;
                case ';': parser->set_next_token(TK(";")); return;
                case '(': parser->set_next_token(TK("(")); return;
                case ')': parser->set_next_token(TK(")")); return;
                case '[': parser->set_next_token(TK("[")); return;
                case ']': parser->set_next_token(TK("]")); return;
                case '%': parser->set_next_token_2('=', TK("%"), TK("%=")); return;
                case '&': parser->set_next_token_2('=', TK("&"), TK("&=")); return;
                case '|': parser->set_next_token_2('=', TK("|"), TK("|=")); return;
                case '^': parser->set_next_token_2('=', TK("^"), TK("^=")); return;
                case '?': parser->set_next_token(TK("?")); return;
                case '.': {
                    if(parser->matchchar('.')) {
                        if(parser->matchchar('.')) {
                            parser->set_next_token(TK("..."));
                        } else {
                            SyntaxError("invalid token '..'");
                        }
                    } else {
                        parser->set_next_token(TK("."));
                    }
                    return;
                }
                case '=': parser->set_next_token_2('=', TK("="), TK("==")); return;
                case '+': parser->set_next_token_2('=', TK("+"), TK("+=")); return;
                case '>': {
                    if(parser->matchchar('=')) parser->set_next_token(TK(">="));
                    else if(parser->matchchar('>')) parser->set_next_token(TK(">>"));
                    else parser->set_next_token(TK(">"));
                    return;
                }
                case '<': {
                    if(parser->matchchar('=')) parser->set_next_token(TK("<="));
                    else if(parser->matchchar('<')) parser->set_next_token(TK("<<"));
                    else parser->set_next_token(TK("<"));
                    return;
                }
                case '-': {
                    if(parser->matchchar('=')) parser->set_next_token(TK("-="));
                    else if(parser->matchchar('>')) parser->set_next_token(TK("->"));
                    else parser->set_next_token(TK("-"));
                    return;
                }
                case '!':
                    if(parser->matchchar('=')) parser->set_next_token(TK("!="));
                    else SyntaxError("expected '=' after '!'");
                    break;
                case '*':
                    if (parser->matchchar('*')) {
                        parser->set_next_token(TK("**"));  // '**'
                    } else {
                        parser->set_next_token_2('=', TK("*"), TK("*="));
                    }
                    return;
                case '/':
                    if(parser->matchchar('/')) {
                        parser->set_next_token_2('=', TK("//"), TK("//="));
                    } else {
                        parser->set_next_token_2('=', TK("/"), TK("/="));
                    }
                    return;
                case '\r': break;       // just ignore '\r'
                case ' ': case '\t': parser->eat_spaces(); break;
                case '\n': {
                    parser->set_next_token(TK("@eol"));
                    if(!parser->eat_indentation()) IndentationError("unindent does not match any outer indentation level");
                    return;
                }
                default: {
                    if(c == 'f'){
                        if(parser->matchchar('\'')) {eat_string('\'', F_STRING); return;}
                        if(parser->matchchar('"')) {eat_string('"', F_STRING); return;}
                    }else if(c == 'r'){
                        if(parser->matchchar('\'')) {eat_string('\'', RAW_STRING); return;}
                        if(parser->matchchar('"')) {eat_string('"', RAW_STRING); return;}
                    }

                    if (c >= '0' && c <= '9') {
                        eat_number();
                        return;
                    }
                    
                    switch (parser->eat_name())
                    {
                        case 0: break;
                        case 1: SyntaxError("invalid char: " + std::string(1, c));
                        case 2: SyntaxError("invalid utf8 sequence: " + std::string(1, c));
                        case 3: SyntaxError("@id contains invalid char"); break;
                        case 4: SyntaxError("invalid JSON token"); break;
                        default: UNREACHABLE();
                    }
                    return;
                }
            }
        }

        parser->token_start = parser->curr_char;
        parser->set_next_token(TK("@eof"));
    }

    inline TokenIndex peek() {
        return parser->curr.type;
    }

    // not sure this will work
    TokenIndex peek_next() {
        if(parser->nexts.empty()) return TK("@eof");
        return parser->nexts.front().type;
    }

    bool match(TokenIndex expected) {
        if (peek() != expected) return false;
        lex_token();
        return true;
    }

    void consume(TokenIndex expected) {
        if (!match(expected)){
            _StrStream ss;
            ss << "expected '" << TK_STR(expected) << "', but got '" << TK_STR(peek()) << "'";
            SyntaxError(ss.str());
        }
    }

    bool match_newlines(bool repl_throw=false) {
        bool consumed = false;
        if (peek() == TK("@eol")) {
            while (peek() == TK("@eol")) lex_token();
            consumed = true;
        }
        if (repl_throw && peek() == TK("@eof")){
            throw NeedMoreLines(is_compiling_class);
        }
        return consumed;
    }

    bool match_end_stmt() {
        if (match(TK(";"))) { match_newlines(); return true; }
        if (match_newlines() || peek()==TK("@eof")) return true;
        if (peek() == TK("@dedent")) return true;
        return false;
    }

    void consume_end_stmt() {
        if (!match_end_stmt()) SyntaxError("expected statement end");
    }

    void exprLiteral() {
        PyVar value = parser->prev.value;
        int index = co()->add_const(value);
        emit(OP_LOAD_CONST, index);
    }

    void exprFString() {
        static const std::regex pattern(R"(\{(.*?)\})");
        PyVar value = parser->prev.value;
        Str s = vm->PyStr_AS_C(value);
        std::sregex_iterator begin(s.begin(), s.end(), pattern);
        std::sregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::smatch m = *it;
            if (i < m.position()) {
                std::string literal = s.substr(i, m.position() - i);
                emit(OP_LOAD_CONST, co()->add_const(vm->PyStr(literal)));
                size++;
            }
            emit(OP_LOAD_EVAL_FN);
            emit(OP_LOAD_CONST, co()->add_const(vm->PyStr(m[1].str())));
            emit(OP_CALL, 1);
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < s.size()) {
            std::string literal = s.substr(i, s.size() - i);
            emit(OP_LOAD_CONST, co()->add_const(vm->PyStr(literal)));
            size++;
        }
        emit(OP_BUILD_STRING, size);
    }

    void exprLambda() {
        pkpy::Function_ func = pkpy::make_shared<pkpy::Function>();
        func->name = "<lambda>";
        if(!match(TK(":"))){
            _compile_f_args(func, false);
            consume(TK(":"));
        }
        func->code = pkpy::make_shared<CodeObject>(parser->src, func->name);
        this->codes.push(func->code);
        EXPR_TUPLE();
        emit(OP_RETURN_VALUE);
        func->code->optimize();
        this->codes.pop();
        emit(OP_LOAD_LAMBDA, co()->add_const(vm->PyFunction(func)));
    }

    void exprAssign() {
        co()->_rvalue = true;
        TokenIndex op = parser->prev.type;
        if(op == TK("=")) {     // a = (expr)
            EXPR_TUPLE();
            emit(OP_STORE_REF);
        }else{                  // a += (expr) -> a = a + (expr)
            emit(OP_DUP_TOP);
            EXPR();
            switch (op) {
                case TK("+="):      emit(OP_BINARY_OP, 0);  break;
                case TK("-="):      emit(OP_BINARY_OP, 1);  break;
                case TK("*="):      emit(OP_BINARY_OP, 2);  break;
                case TK("/="):      emit(OP_BINARY_OP, 3);  break;
                case TK("//="):     emit(OP_BINARY_OP, 4);  break;
                case TK("%="):      emit(OP_BINARY_OP, 5);  break;
                case TK("&="):      emit(OP_BITWISE_OP, 2);  break;
                case TK("|="):      emit(OP_BITWISE_OP, 3);  break;
                case TK("^="):      emit(OP_BITWISE_OP, 4);  break;
                default: UNREACHABLE();
            }
            emit(OP_STORE_REF);
        }
        co()->_rvalue = false;
    }

    void exprComma() {
        int size = 1;       // an expr is in the stack now
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            size++;
        } while(match(TK(",")));
        emit(OP_BUILD_SMART_TUPLE, size);
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
        TokenIndex op = parser->prev.type;
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

    void exprUnaryOp() {
        TokenIndex op = parser->prev.type;
        parse_expression((Precedence)(PREC_UNARY + 1));
        switch (op) {
            case TK("-"):     emit(OP_UNARY_NEGATIVE); break;
            case TK("not"):   emit(OP_UNARY_NOT);      break;
            case TK("*"):     SyntaxError("cannot use '*' as unary operator"); break;
            default: UNREACHABLE();
        }
    }

    void exprGrouping() {
        match_newlines(mode()==REPL_MODE);
        EXPR_TUPLE();
        match_newlines(mode()==REPL_MODE);
        consume(TK(")"));
    }

    void exprList() {
        int _patch = emit(OP_NO_OP);
        int _body_start = co()->codes.size();
        int ARGC = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (peek() == TK("]")) break;
            EXPR(); ARGC++;
            match_newlines(mode()==REPL_MODE);
            if(ARGC == 1 && match(TK("for"))) goto __LISTCOMP;
        } while (match(TK(",")));
        match_newlines(mode()==REPL_MODE);
        consume(TK("]"));
        emit(OP_BUILD_LIST, ARGC);
        return;

__LISTCOMP:
        int _body_end_return = emit(OP_JUMP_ABSOLUTE, -1);
        int _body_end = co()->codes.size();
        co()->codes[_patch].op = OP_JUMP_ABSOLUTE;
        co()->codes[_patch].arg = _body_end;
        emit(OP_BUILD_LIST, 0);
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
            emit(OP_LIST_APPEND);
            patch_jump(ifpatch);
        }else{
            emit(OP_JUMP_ABSOLUTE, _body_start);
            patch_jump(_body_end_return);
            emit(OP_LIST_APPEND);
        }

        emit(OP_LOOP_CONTINUE, -1, true);
        co()->_exit_block();
        match_newlines(mode()==REPL_MODE);
        consume(TK("]"));
    }

    void exprMap() {
        bool parsing_dict = false;
        int size = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (peek() == TK("}")) break;
            EXPR();
            if(peek() == TK(":")) parsing_dict = true;
            if(parsing_dict){
                consume(TK(":"));
                EXPR();
            }
            size++;
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK("}"));

        if(size == 0 || parsing_dict) emit(OP_BUILD_MAP, size);
        else emit(OP_BUILD_SET, size);
    }

    void exprCall() {
        int ARGC = 0;
        int KWARGC = 0;
        do {
            match_newlines(mode()==REPL_MODE);
            if (peek() == TK(")")) break;
            if(peek() == TK("@id") && peek_next() == TK("=")) {
                consume(TK("@id"));
                const Str& key = parser->prev.str();
                emit(OP_LOAD_CONST, co()->add_const(vm->PyStr(key)));
                consume(TK("="));
                co()->_rvalue=true; EXPR(); co()->_rvalue=false;
                KWARGC++;
            } else{
                if(KWARGC > 0) SyntaxError("positional argument follows keyword argument");
                co()->_rvalue=true; EXPR(); co()->_rvalue=false;
                ARGC++;
            }
            match_newlines(mode()==REPL_MODE);
        } while (match(TK(",")));
        consume(TK(")"));
        emit(OP_CALL, (KWARGC << 16) | ARGC);
    }

    void exprName(){ _exprName(false); }

    void _exprName(bool force_lvalue) {
        Token tkname = parser->prev;
        int index = co()->add_name(
            tkname.str(),
            codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL
        );
        bool fast_load = !force_lvalue && co()->_rvalue;
        emit(fast_load ? OP_LOAD_NAME : OP_LOAD_NAME_REF, index);
    }

    void exprAttrib() {
        consume(TK("@id"));
        const Str& name = parser->prev.str();
        int index = co()->add_name(name, NAME_ATTR);
        emit(OP_BUILD_ATTR_REF, index);
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
        emit(OP_BUILD_INDEX_REF);
    }

    void exprValue() {
        TokenIndex op = parser->prev.type;
        switch (op) {
            case TK("None"):    emit(OP_LOAD_NONE);  break;
            case TK("True"):    emit(OP_LOAD_TRUE);  break;
            case TK("False"):   emit(OP_LOAD_FALSE); break;
            case TK("..."):     emit(OP_LOAD_ELLIPSIS); break;
            default: UNREACHABLE();
        }
    }

    int emit(Opcode opcode, int arg=-1, bool keepline=false) {
        int line = parser->prev.line;
        co()->codes.push_back(
            Bytecode{(uint8_t)opcode, arg, line, (uint16_t)co()->_curr_block_i}
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
        if(!match_newlines(mode()==REPL_MODE)){
            SyntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (peek() != TK("@dedent")) {
            match_newlines();
            (this->*action)();
            match_newlines();
        }
        consume(TK("@dedent"));
    }

    Token _compile_import() {
        consume(TK("@id"));
        Token tkmodule = parser->prev;
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
                tkmodule = parser->prev;
            }
            int index = co()->add_name(tkmodule.str(), NAME_GLOBAL);
            emit(OP_STORE_NAME, index);
        } while (match(TK(",")));
        consume_end_stmt();
    }

    // from a import b as c, d as e
    void compile_from_import() {
        Token tkmodule = _compile_import();
        consume(TK("import"));
        do {
            emit(OP_DUP_TOP);
            consume(TK("@id"));
            Token tkname = parser->prev;
            int index = co()->add_name(tkname.str(), NAME_ATTR);
            emit(OP_BUILD_ATTR_REF, index);
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkname = parser->prev;
            }
            index = co()->add_name(tkname.str(), NAME_GLOBAL);
            emit(OP_STORE_NAME, index);
        } while (match(TK(",")));
        emit(OP_POP_TOP);
        consume_end_stmt();
    }

    void parse_expression(Precedence precedence) {
        lex_token();
        GrammarFn prefix = rules[parser->prev.type].prefix;
        if (prefix == nullptr) SyntaxError(Str("expected an expression, but got ") + TK_STR(parser->prev.type));
        (this->*prefix)();
        while (rules[peek()].precedence >= precedence) {
            lex_token();
            TokenIndex op = parser->prev.type;
            GrammarFn infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
    }

    void compile_if_stmt() {
        match_newlines();
        co()->_rvalue = true;
        EXPR_TUPLE();   // condition
        co()->_rvalue = false;
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
        co()->_rvalue = true;
        EXPR_TUPLE();   // condition
        co()->_rvalue = false;
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
        if(size > 1) emit(OP_BUILD_SMART_TUPLE, size);
    }

    void compile_for_loop() {
        EXPR_FOR_VARS();consume(TK("in")); EXPR_TUPLE();
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
                int name_idx = co()->add_name(parser->prev.str(), NAME_SPECIAL);
                emit(OP_EXCEPTION_MATCH, name_idx);
            }else{
                emit(OP_LOAD_TRUE);
            }
            int patch = emit(OP_POP_JUMP_IF_FALSE);
            emit(OP_POP_TOP);       // pop the exception on match
            compile_block_body();
            patches.push_back(emit(OP_JUMP_ABSOLUTE));
            patch_jump(patch);
        }while(peek() == TK("except"));
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
        } else if (match(TK("return"))) {
            if (codes.size() == 1)
                SyntaxError("'return' outside function");
            if(match_end_stmt()){
                emit(OP_LOAD_NONE);
            }else{
                co()->_rvalue = true;
                EXPR_TUPLE();   // return value
                co()->_rvalue = false;
                consume_end_stmt();
            }
            emit(OP_RETURN_VALUE, -1, true);
        } else if (match(TK("if"))) {
            compile_if_stmt();
        } else if (match(TK("while"))) {
            compile_while_loop();
        } else if (match(TK("for"))) {
            compile_for_loop();
        } else if (match(TK("try"))) {
            compile_try_except();
        }else if(match(TK("assert"))){
            EXPR();
            emit(OP_ASSERT);
            consume_end_stmt();
        } else if(match(TK("with"))){
            EXPR();
            consume(TK("as"));
            consume(TK("@id"));
            Token tkname = parser->prev;
            int index = co()->add_name(
                tkname.str(),
                codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL
            );
            emit(OP_STORE_NAME, index);
            emit(OP_LOAD_NAME_REF, index);
            emit(OP_WITH_ENTER);
            compile_block_body();
            emit(OP_LOAD_NAME_REF, index);
            emit(OP_WITH_EXIT);
        } else if(match(TK("label"))){
            if(mode() != EXEC_MODE) SyntaxError("'label' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            Str label = parser->prev.str();
            bool ok = co()->add_label(label);
            if(!ok) SyntaxError("label '" + label + "' already exists");
            consume_end_stmt();
        } else if(match(TK("goto"))){ // https://entrian.com/goto/
            if(mode() != EXEC_MODE) SyntaxError("'goto' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            emit(OP_GOTO, co()->add_name(parser->prev.str(), NAME_SPECIAL));
            consume_end_stmt();
        } else if(match(TK("raise"))){
            consume(TK("@id"));
            int dummy_t = co()->add_name(parser->prev.str(), NAME_SPECIAL);
            if(match(TK("("))){
                EXPR(); consume(TK(")"));
            }else{
                emit(OP_LOAD_NONE);
            }
            emit(OP_RAISE, dummy_t);
            consume_end_stmt();
        } else if(match(TK("del"))){
            EXPR();
            emit(OP_DELETE_REF);
            consume_end_stmt();
        } else if(match(TK("global"))){
            do {
                consume(TK("@id"));
                co()->global_names[parser->prev.str()] = 1;
            } while (match(TK(",")));
            consume_end_stmt();
        } else if(match(TK("pass"))){
            consume_end_stmt();
        } else {
            EXPR_ANY();
            consume_end_stmt();
            // If last op is not an assignment, pop the result.
            uint8_t last_op = co()->codes.back().op;
            if( last_op!=OP_STORE_NAME && last_op!=OP_STORE_REF){
                if(mode()==REPL_MODE && parser->indents.top()==0) emit(OP_PRINT_EXPR, -1, true);
                emit(OP_POP_TOP, -1, true);
            }
        }
    }

    void compile_class(){
        consume(TK("@id"));
        int cls_name_idx = co()->add_name(parser->prev.str(), NAME_GLOBAL);
        int super_cls_name_idx = -1;
        if(match(TK("(")) && match(TK("@id"))){
            super_cls_name_idx = co()->add_name(parser->prev.str(), NAME_GLOBAL);
            consume(TK(")"));
        }
        emit(OP_LOAD_NONE);
        is_compiling_class = true;
        compile_block_body(&Compiler::compile_function);
        is_compiling_class = false;
        if(super_cls_name_idx == -1) emit(OP_LOAD_NONE);
        else emit(OP_LOAD_NAME_REF, super_cls_name_idx);
        emit(OP_BUILD_CLASS, cls_name_idx);
    }

    void _compile_f_args(pkpy::Function_ func, bool enable_type_hints){
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
            const Str& name = parser->prev.str();
            if(func->hasName(name)) SyntaxError("duplicate argument name");

            // eat type hints
            if(enable_type_hints && match(TK(":"))) consume(TK("@id"));

            if(state == 0 && peek() == TK("=")) state = 2;

            switch (state)
            {
                case 0: func->args.push_back(name); break;
                case 1: func->starredArg = name; state+=1; break;
                case 2: {
                    consume(TK("="));
                    PyVarOrNull value = read_literal();
                    if(value == nullptr){
                        SyntaxError(Str("expect a literal, not ") + TK_STR(parser->curr.type));
                    }
                    func->kwArgs[name] = value;
                    func->kwArgsOrder.push_back(name);
                } break;
                case 3: SyntaxError("**kwargs is not supported yet"); break;
            }
        } while (match(TK(",")));
    }

    void compile_function(){
        if(is_compiling_class){
            if(match(TK("pass"))) return;
            consume(TK("def"));
        }
        pkpy::Function_ func = pkpy::make_shared<pkpy::Function>();
        consume(TK("@id"));
        func->name = parser->prev.str();

        if (match(TK("(")) && !match(TK(")"))) {
            _compile_f_args(func, true);
            consume(TK(")"));
        }

        // eat type hints
        if(match(TK("->"))) consume(TK("@id"));

        func->code = pkpy::make_shared<CodeObject>(parser->src, func->name);
        this->codes.push(func->code);
        compile_block_body();
        func->code->optimize();
        this->codes.pop();
        emit(OP_LOAD_CONST, co()->add_const(vm->PyFunction(func)));
        if(!is_compiling_class) emit(OP_STORE_FUNCTION);
    }

    PyVarOrNull read_literal(){
        if(match(TK("-"))){
            consume(TK("@num"));
            PyVar val = parser->prev.value;
            return vm->num_negated(val);
        }
        if(match(TK("@num"))) return parser->prev.value;
        if(match(TK("@str"))) return parser->prev.value;
        if(match(TK("True"))) return vm->PyBool(true);
        if(match(TK("False"))) return vm->PyBool(false);
        if(match(TK("None"))) return vm->None;
        if(match(TK("..."))) return vm->Ellipsis;
        return nullptr;
    }

    /***** Error Reporter *****/
    void throw_err(Str type, Str msg){
        int lineno = parser->curr.line;
        const char* cursor = parser->curr.start;
        // if error occurs in lexing, lineno should be `parser->current_line`
        if(lexing_count > 0){
            lineno = parser->current_line;
            cursor = parser->curr_char;
        }
        if(parser->peekchar() == '\n') lineno--;
        auto e = pkpy::Exception("SyntaxError", msg);
        e.st_push(parser->src->snapshot(lineno, cursor));
        throw e;
    }
    void SyntaxError(Str msg){ throw_err("SyntaxError", msg); }
    void IndentationError(Str msg){ throw_err("IndentationError", msg); }

public:
    CodeObject_ compile(){
        // can only be called once
        if(used) UNREACHABLE();
        used = true;

        CodeObject_ code = pkpy::make_shared<CodeObject>(parser->src, Str("<module>"));
        codes.push(code);

        lex_token(); lex_token();
        match_newlines();

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            code->optimize();
            return code;
        }else if(mode()==JSON_MODE){
            PyVarOrNull value = read_literal();
            if(value != nullptr) emit(OP_LOAD_CONST, code->add_const(value));
            else if(match(TK("{"))) exprMap();
            else if(match(TK("["))) exprList();
            else SyntaxError("expect a JSON object or array");
            consume(TK("@eof"));
            return code;    // no need to optimize for JSON decoding
        }

        while (!match(TK("@eof"))) {
            // compile top-level statement
            if (match(TK("class"))) {
                compile_class();
            } else if (match(TK("def"))) {
                compile_function();
            } else if (match(TK("import"))) {
                compile_normal_import();
            } else if (match(TK("from"))) {
                compile_from_import();
            } else {
                compile_stmt();
            }
            match_newlines();
        }
        code->optimize();
        return code;
    }
};