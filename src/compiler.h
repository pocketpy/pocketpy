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

struct Loop {
    int start;
    std::vector<int> breaks;
    Loop(int start) : start(start) {}
};

class Compiler {
public:
    pkpy::unique_ptr<Parser> parser;
    std::stack<_Code> codes;
    std::stack<Loop> loops;
    bool isCompilingClass = false;
    VM* vm;

    emhash8::HashMap<_TokenType, GrammarRule> rules;

    _Code getCode() {
        return codes.top();
    }

    CompileMode mode() {
        return parser->src->mode;
    }

    Loop& getLoop() {
        return loops.top();
    }

    Compiler(VM* vm, const char* source, _Str filename, CompileMode mode){
        this->vm = vm;
        this->parser = pkpy::make_unique<Parser>(
            pkpy::make_shared<SourceMetadata>(source, filename, mode)
        );

// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define METHOD(name) &Compiler::name
#define NO_INFIX nullptr, PREC_NONE
        for(_TokenType i=0; i<__TOKENS_LEN; i++) rules[i] = { nullptr, NO_INFIX };
        rules[TK(".")] =    { nullptr,               METHOD(exprAttrib),         PREC_ATTRIB };
        rules[TK("->")] =   { nullptr,               METHOD(exprAttribPtr),      PREC_ATTRIB };
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
        rules[TK(",")] =        { nullptr,               METHOD(exprComma),          PREC_COMMA };
        rules[TK("<<")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK(">>")] =       { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_SHIFT };
        rules[TK("&")] =        { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_BITWISE_AND };
        rules[TK("|")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_OR };
        rules[TK("^")] =        { nullptr,               METHOD(exprBinaryOp),       PREC_BITWISE_XOR };
#undef METHOD
#undef NO_INFIX

#define EXPR() parsePrecedence(PREC_TERNARY)             // no '=' and ',' just a simple expression
#define EXPR_TUPLE() parsePrecedence(PREC_COMMA)            // no '=', but ',' is allowed
#define EXPR_ANY() parsePrecedence(PREC_ASSIGNMENT)
    }

    _Str eatStringUntil(char quote) {
        std::vector<char> buff;
        while (true) {
            char c = parser->eatCharIncludeNewLine();
            if (c == quote) break;
            if (c == '\0' || c == '\n') syntaxError("EOL while scanning string literal");
            if (c == '\\') {
                switch (parser->eatCharIncludeNewLine()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    case '\n': case '\r': break;
                    default: syntaxError("invalid escape character");
                }
            } else {
                buff.push_back(c);
            }
        }
        return _Str(buff.data(), buff.size());
    }

    void eatString(char quote, bool fstr) {
        _Str s = eatStringUntil(quote);
        if(fstr){
            parser->setNextToken(TK("@fstr"), vm->PyStr(s));
        }else{
            parser->setNextToken(TK("@str"), vm->PyStr(s));
        }
    }

    void eatNumber() {
        static const std::regex pattern("^(0x)?[0-9a-f]+(\\.[0-9]+)?");
        std::smatch m;

        const char* i = parser->token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(parser->token_start, i);

        try{
            if (std::regex_search(s, m, pattern)) {
                // here is m.length()-1, since the first char is eaten by lexToken()
                for(int j=0; j<m.length()-1; j++) parser->eatChar();

                int base = 10;
                size_t size;
                if (m[1].matched) base = 16;
                if (m[2].matched) {
                    if(base == 16) syntaxError("hex literal should not contain a dot");
                    parser->setNextToken(TK("@num"), vm->PyFloat(std::stod(m[0], &size)));
                } else {
                    parser->setNextToken(TK("@num"), vm->PyInt(std::stoll(m[0], &size, base)));
                }
                if (size != m.length()) throw std::runtime_error("length mismatch");
            }
        }catch(std::exception& _){
            syntaxError("invalid number literal");
        } 
    }

    // Lex the next token and set it as the next token.
    void lexToken() {
        parser->previous = parser->current;
        parser->current = parser->nextToken();

        //_Str _info = parser->current.info(); printf("%s\n", (const char*)_info);

        while (parser->peekChar() != '\0') {
            parser->token_start = parser->current_char;
            char c = parser->eatCharIncludeNewLine();
            switch (c) {
                case '\'': case '"': eatString(c, false); return;
                case '#': parser->skipLineComment(); break;
                case '{': parser->setNextToken(TK("{")); return;
                case '}': parser->setNextToken(TK("}")); return;
                case ',': parser->setNextToken(TK(",")); return;
                case ':': parser->setNextToken(TK(":")); return;
                case ';': parser->setNextToken(TK(";")); return;
                case '(': parser->setNextToken(TK("(")); return;
                case ')': parser->setNextToken(TK(")")); return;
                case '[': parser->setNextToken(TK("[")); return;
                case ']': parser->setNextToken(TK("]")); return;
                case '%': parser->setNextToken(TK("%")); return;
                case '&': parser->setNextToken(TK("&")); return;
                case '|': parser->setNextToken(TK("|")); return;
                case '^': parser->setNextToken(TK("^")); return;
                case '?': parser->setNextToken(TK("?")); return;
                case '.': {
                    if(parser->matchChar('.')) {
                        if(parser->matchChar('.')) {
                            parser->setNextToken(TK("..."));
                        } else {
                            syntaxError("invalid token '..'");
                        }
                    } else {
                        parser->setNextToken(TK("."));
                    }
                    return;
                }
                case '=': parser->setNextTwoCharToken('=', TK("="), TK("==")); return;
                case '+': parser->setNextTwoCharToken('=', TK("+"), TK("+=")); return;
                case '>': {
                    if(parser->matchChar('=')) parser->setNextToken(TK(">="));
                    else if(parser->matchChar('>')) parser->setNextToken(TK(">>"));
                    else parser->setNextToken(TK(">"));
                    return;
                }
                case '<': {
                    if(parser->matchChar('=')) parser->setNextToken(TK("<="));
                    else if(parser->matchChar('<')) parser->setNextToken(TK("<<"));
                    else parser->setNextToken(TK("<"));
                    return;
                }
                case '-': {
                    if(parser->matchChar('=')) parser->setNextToken(TK("-="));
                    else if(parser->matchChar('>')) parser->setNextToken(TK("->"));
                    else parser->setNextToken(TK("-"));
                    return;
                }
                case '!':
                    if(parser->matchChar('=')) parser->setNextToken(TK("!="));
                    else syntaxError("expected '=' after '!'");
                    break;
                case '*':
                    if (parser->matchChar('*')) {
                        parser->setNextToken(TK("**"));  // '**'
                    } else {
                        parser->setNextTwoCharToken('=', TK("*"), TK("*="));
                    }
                    return;
                case '/':
                    if(parser->matchChar('/')) {
                        parser->setNextTwoCharToken('=', TK("//"), TK("//="));
                    } else {
                        parser->setNextTwoCharToken('=', TK("/"), TK("/="));
                    }
                    return;
                case '\r': break;       // just ignore '\r'
                case ' ': case '\t': parser->eatSpaces(); break;
                case '\n': {
                    parser->setNextToken(TK("@eol")); while(parser->matchChar('\n'));
                    if(!parser->eatIndentation()) indentationError("unindent does not match any outer indentation level");
                    return;
                }
                default: {
                    if(c == 'f'){
                        if(parser->matchChar('\'')) {eatString('\'', true); return;}
                        if(parser->matchChar('"')) {eatString('"', true); return;}
                    }
                    if (isdigit(c)) {
                        eatNumber();
                    } else if (parser->isNameStart(c)) {
                        int ret = parser->eatName();
                        if(ret!=0) syntaxError("identifier is illegal, err " + std::to_string(ret));
                    } else {
                        syntaxError("unknown character: " + std::string(1, c));
                    }
                    return;
                }
            }
        }

        parser->token_start = parser->current_char;
        parser->setNextToken(TK("@eof"));
    }

    _TokenType peek() {
        return parser->current.type;
    }

    bool match(_TokenType expected) {
        if (peek() != expected) return false;
        lexToken();
        return true;
    }

    void consume(_TokenType expected) {
        if (!match(expected)){
            _StrStream ss;
            ss << "expected '" << TK_STR(expected) << "', but got '" << TK_STR(peek()) << "'";
            syntaxError(ss.str());
        }
    }

    bool matchNewLines(bool repl_throw=false) {
        bool consumed = false;
        if (peek() == TK("@eol")) {
            while (peek() == TK("@eol")) lexToken();
            consumed = true;
        }
        if (repl_throw && peek() == TK("@eof")){
            throw NeedMoreLines(isCompilingClass);
        }
        return consumed;
    }

    bool matchEndStatement() {
        if (match(TK(";"))) {
            matchNewLines();
            return true;
        }
        if (matchNewLines() || peek() == TK("@eof"))
            return true;
        if (peek() == TK("@dedent")) return true;
        return false;
    }

    void consumeEndStatement() {
        if (!matchEndStatement()) syntaxError("expected statement end");
    }

    void exprLiteral() {
        PyVar value = parser->previous.value;
        int index = getCode()->addConst(value);
        emitCode(OP_LOAD_CONST, index);
    }

    void exprFString() {
        static const std::regex pattern(R"(\{(.*?)\})");
        PyVar value = parser->previous.value;
        std::string s = vm->PyStr_AS_C(value).str();
        std::sregex_iterator begin(s.begin(), s.end(), pattern);
        std::sregex_iterator end;
        int size = 0;
        int i = 0;
        for(auto it = begin; it != end; it++) {
            std::smatch m = *it;
            if (i < m.position()) {
                std::string literal = s.substr(i, m.position() - i);
                emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(literal)));
                size++;
            }
            emitCode(OP_LOAD_EVAL_FN);
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(m[1].str())));
            emitCode(OP_CALL, 1);
            size++;
            i = (int)(m.position() + m.length());
        }
        if (i < s.size()) {
            std::string literal = s.substr(i, s.size() - i);
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(literal)));
            size++;
        }
        emitCode(OP_BUILD_STRING, size);
    }

    void exprLambda() {
        _Func func = pkpy::make_shared<Function>();
        func->name = "<lambda>";
        if(!match(TK(":"))){
            __compileFunctionArgs(func);
            consume(TK(":"));
        }
        func->code = pkpy::make_shared<CodeObject>(parser->src, func->name);
        this->codes.push(func->code);
        EXPR_TUPLE();
        emitCode(OP_RETURN_VALUE);
        this->codes.pop();
        emitCode(OP_LOAD_LAMBDA, getCode()->addConst(vm->PyFunction(func)));
    }

    void exprAssign() {
        _TokenType op = parser->previous.type;
        if(op == TK("=")) {     // a = (expr)
            EXPR_TUPLE();
            emitCode(OP_STORE_PTR);
        }else{                  // a += (expr) -> a = a + (expr)
            // TODO: optimization is needed for inplace operators
            emitCode(OP_DUP_TOP);
            EXPR();
            switch (op) {
                case TK("+="):      emitCode(OP_BINARY_OP, 0);  break;
                case TK("-="):      emitCode(OP_BINARY_OP, 1);  break;
                case TK("*="):      emitCode(OP_BINARY_OP, 2);  break;
                case TK("/="):      emitCode(OP_BINARY_OP, 3);  break;
                case TK("//="):     emitCode(OP_BINARY_OP, 4);  break;
                default: UNREACHABLE();
            }
            emitCode(OP_STORE_PTR);
        }
    }

    void exprComma() {
        int size = 1;       // an expr is in the stack now
        do {
            EXPR();         // NOTE: "1," will fail, "1,2" will be ok
            size++;
        } while(match(TK(",")));
        emitCode(OP_BUILD_SMART_TUPLE, size);
    }

    void exprOr() {
        int patch = emitCode(OP_JUMP_IF_TRUE_OR_POP);
        parsePrecedence(PREC_LOGICAL_OR);
        patchJump(patch);
    }

    void exprAnd() {
        int patch = emitCode(OP_JUMP_IF_FALSE_OR_POP);
        parsePrecedence(PREC_LOGICAL_AND);
        patchJump(patch);
    }

    void exprTernary() {
        int patch = emitCode(OP_POP_JUMP_IF_FALSE);
        EXPR();         // if true
        int patch2 = emitCode(OP_JUMP_ABSOLUTE);
        consume(TK(":"));
        patchJump(patch);
        EXPR();         // if false
        patchJump(patch2);
    }

    void exprBinaryOp() {
        _TokenType op = parser->previous.type;
        parsePrecedence((Precedence)(rules[op].precedence + 1));

        switch (op) {
            case TK("+"):   emitCode(OP_BINARY_OP, 0);  break;
            case TK("-"):   emitCode(OP_BINARY_OP, 1);  break;
            case TK("*"):   emitCode(OP_BINARY_OP, 2);  break;
            case TK("/"):   emitCode(OP_BINARY_OP, 3);  break;
            case TK("//"):  emitCode(OP_BINARY_OP, 4);  break;
            case TK("%"):   emitCode(OP_BINARY_OP, 5);  break;
            case TK("**"):  emitCode(OP_BINARY_OP, 6);  break;

            case TK("<"):   emitCode(OP_COMPARE_OP, 0);    break;
            case TK("<="):  emitCode(OP_COMPARE_OP, 1);    break;
            case TK("=="):  emitCode(OP_COMPARE_OP, 2);    break;
            case TK("!="):  emitCode(OP_COMPARE_OP, 3);    break;
            case TK(">"):   emitCode(OP_COMPARE_OP, 4);    break;
            case TK(">="):  emitCode(OP_COMPARE_OP, 5);    break;
            case TK("in"):      emitCode(OP_CONTAINS_OP, 0);   break;
            case TK("not in"):  emitCode(OP_CONTAINS_OP, 1);   break;
            case TK("is"):      emitCode(OP_IS_OP, 0);         break;
            case TK("is not"):  emitCode(OP_IS_OP, 1);         break;

            case TK("<<"):  emitCode(OP_BITWISE_OP, 0);    break;
            case TK(">>"):  emitCode(OP_BITWISE_OP, 1);    break;
            case TK("&"):   emitCode(OP_BITWISE_OP, 2);    break;
            case TK("|"):   emitCode(OP_BITWISE_OP, 3);    break;
            case TK("^"):   emitCode(OP_BITWISE_OP, 4);    break;
            default: UNREACHABLE();
        }
    }

    void exprUnaryOp() {
        _TokenType op = parser->previous.type;
        matchNewLines();
        parsePrecedence((Precedence)(PREC_UNARY + 1));

        switch (op) {
            case TK("-"):     emitCode(OP_UNARY_NEGATIVE); break;
            case TK("not"):   emitCode(OP_UNARY_NOT);      break;
            case TK("&"):     emitCode(OP_UNARY_REF);      break;
            case TK("*"):     emitCode(OP_UNARY_DEREF);    break;
            default: UNREACHABLE();
        }
    }

    void exprGrouping() {
        matchNewLines(mode()==SINGLE_MODE);
        EXPR_TUPLE();
        matchNewLines(mode()==SINGLE_MODE);
        consume(TK(")"));
    }

    void exprList() {
        int _patch = emitCode(OP_NO_OP);
        int _body_start = getCode()->co_code.size();
        int ARGC = 0;
        do {
            matchNewLines(mode()==SINGLE_MODE);
            if (peek() == TK("]")) break;
            EXPR(); ARGC++;
            matchNewLines(mode()==SINGLE_MODE);
            if(ARGC == 1 && match(TK("for"))) goto __LISTCOMP;
        } while (match(TK(",")));
        matchNewLines(mode()==SINGLE_MODE);
        consume(TK("]"));
        emitCode(OP_BUILD_LIST, ARGC);
        return;

__LISTCOMP:
        int _body_end = getCode()->co_code.size();
        getCode()->co_code[_patch].op = OP_JUMP_ABSOLUTE;
        getCode()->co_code[_patch].arg = _body_end;
        emitCode(OP_BUILD_LIST, 0);
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        matchNewLines(mode()==SINGLE_MODE);
        
        int _skipPatch = emitCode(OP_JUMP_ABSOLUTE);
        int _cond_start = getCode()->co_code.size();
        if(match(TK("if"))) EXPR_TUPLE();
        int _cond_end = getCode()->co_code.size();
        patchJump(_skipPatch);

        emitCode(OP_GET_ITER);
        Loop& loop = enterLoop();
        int patch = emitCode(OP_FOR_ITER);

        if(_cond_end != _cond_start) {      // there is an if condition
            getCode()->__moveToEnd(_cond_start, _cond_end);
            int ifpatch = emitCode(OP_POP_JUMP_IF_FALSE);
            getCode()->__moveToEnd(_body_start, _body_end);
            emitCode(OP_LIST_APPEND);
            patchJump(ifpatch);
        }else{
            getCode()->__moveToEnd(_body_start, _body_end);
            emitCode(OP_LIST_APPEND);
        }

        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
        matchNewLines(mode()==SINGLE_MODE);
        consume(TK("]"));
    }

    void exprMap() {
        int size = 0;
        do {
            matchNewLines(mode()==SINGLE_MODE);
            if (peek() == TK("}")) break;
            EXPR();consume(TK(":"));EXPR();
            size++;
            matchNewLines(mode()==SINGLE_MODE);
        } while (match(TK(",")));
        matchNewLines();
        consume(TK("}"));
        emitCode(OP_BUILD_MAP, size);
    }

    void exprCall() {
        int ARGC = 0;
        do {
            matchNewLines(mode()==SINGLE_MODE);
            if (peek() == TK(")")) break;
            EXPR();
            ARGC++;
            matchNewLines(mode()==SINGLE_MODE);
        } while (match(TK(",")));
        consume(TK(")"));
        emitCode(OP_CALL, ARGC);
    }

    void exprName() {
        Token tkname = parser->previous;
        int index = getCode()->addName(
            tkname.str(),
            codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL
        );
        emitCode(OP_LOAD_NAME_PTR, index);
    }

    void exprAttrib() {
        consume(TK("@id"));
        const _Str& name = parser->previous.str();
        int index = getCode()->addName(name, NAME_ATTR);
        emitCode(OP_BUILD_ATTR_PTR, index);
    }

    void exprAttribPtr(){
        consume(TK("@id"));
        const _Str& name = parser->previous.str();
        int index = getCode()->addName(name, NAME_ATTR);
        emitCode(OP_BUILD_ATTR_PTR_PTR, index);
    }

    // [:], [:b]
    // [a], [a:], [a:b]
    void exprSubscript() {
        if(match(TK(":"))){
            emitCode(OP_LOAD_NONE);
            if(match(TK("]"))){
                emitCode(OP_LOAD_NONE);
            }else{
                EXPR();
                consume(TK("]"));
            }
            emitCode(OP_BUILD_SLICE);
        }else{
            EXPR();
            if(match(TK(":"))){
                if(match(TK("]"))){
                    emitCode(OP_LOAD_NONE);
                }else{
                    EXPR();
                    consume(TK("]"));
                }
                emitCode(OP_BUILD_SLICE);
            }else{
                consume(TK("]"));
            }
        }

        emitCode(OP_BUILD_INDEX_PTR);
    }

    void exprValue() {
        _TokenType op = parser->previous.type;
        switch (op) {
            case TK("None"):    emitCode(OP_LOAD_NONE);  break;
            case TK("True"):    emitCode(OP_LOAD_TRUE);  break;
            case TK("False"):   emitCode(OP_LOAD_FALSE); break;
            case TK("..."):     emitCode(OP_LOAD_ELLIPSIS); break;
            default: UNREACHABLE();
        }
    }

    void keepOpcodeLine(){
        int i = getCode()->co_code.size() - 1;
        getCode()->co_code[i].line = getCode()->co_code[i-1].line;
    }

    int emitCode(Opcode opcode, int arg=-1) {
        int line = parser->previous.line;
        getCode()->co_code.push_back(
            ByteCode{(uint8_t)opcode, arg, (uint16_t)line}
        );
        return getCode()->co_code.size() - 1;
    }

    inline void patchJump(int addr_index) {
        int target = getCode()->co_code.size();
        getCode()->co_code[addr_index].arg = target;
    }

    void compileBlockBody(){
        __compileBlockBody(&Compiler::compileStatement);
    }
    
    void __compileBlockBody(CompilerAction action) {
        consume(TK(":"));
        if(!matchNewLines(mode()==SINGLE_MODE)){
            syntaxError("expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (peek() != TK("@dedent")) {
            matchNewLines();
            (this->*action)();
            matchNewLines();
        }
        consume(TK("@dedent"));
    }

    Token compileImportPath() {
        consume(TK("@id"));
        Token tkmodule = parser->previous;
        int index = getCode()->addName(tkmodule.str(), NAME_GLOBAL);
        emitCode(OP_IMPORT_NAME, index);
        return tkmodule;
    }

    // import a as b
    void compileRegularImport() {
        do {
            Token tkmodule = compileImportPath();
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkmodule = parser->previous;
            }
            int index = getCode()->addName(tkmodule.str(), NAME_GLOBAL);
            emitCode(OP_STORE_NAME_PTR, index);
        } while (match(TK(",")));
        consumeEndStatement();
    }

    // from a import b as c, d as e
    void compileFromImport() {
        Token tkmodule = compileImportPath();
        consume(TK("import"));
        do {
            emitCode(OP_DUP_TOP);
            consume(TK("@id"));
            Token tkname = parser->previous;
            int index = getCode()->addName(tkname.str(), NAME_GLOBAL);
            emitCode(OP_BUILD_ATTR_PTR, index);
            if (match(TK("as"))) {
                consume(TK("@id"));
                tkname = parser->previous;
            }
            index = getCode()->addName(tkname.str(), NAME_GLOBAL);
            emitCode(OP_STORE_NAME_PTR, index);
        } while (match(TK(",")));
        emitCode(OP_POP_TOP);
        consumeEndStatement();
    }

    void parsePrecedence(Precedence precedence) {
        lexToken();
        GrammarFn prefix = rules[parser->previous.type].prefix;
        if (prefix == nullptr) syntaxError(_Str("expected an expression, but got ") + TK_STR(parser->previous.type));
        (this->*prefix)();
        while (rules[peek()].precedence >= precedence) {
            lexToken();
            _TokenType op = parser->previous.type;
            GrammarFn infix = rules[op].infix;
            if(infix == nullptr) throw std::runtime_error("(infix == nullptr) is true");
            (this->*infix)();
        }
    }

    void compileIfStatement() {
        matchNewLines();
        EXPR_TUPLE();

        int ifpatch = emitCode(OP_POP_JUMP_IF_FALSE);
        compileBlockBody();

        if (match(TK("elif"))) {
            int exit_jump = emitCode(OP_JUMP_ABSOLUTE);
            patchJump(ifpatch);
            compileIfStatement();
            patchJump(exit_jump);
        } else if (match(TK("else"))) {
            int exit_jump = emitCode(OP_JUMP_ABSOLUTE);
            patchJump(ifpatch);
            compileBlockBody();
            patchJump(exit_jump);
        } else {
            patchJump(ifpatch);
        }
    }

    Loop& enterLoop(){
        Loop lp((int)getCode()->co_code.size());
        loops.push(lp);
        return loops.top();
    }

    void exitLoop(){
        Loop& lp = loops.top();
        for(int addr : lp.breaks) patchJump(addr);
        loops.pop();
    }

    void compileWhileLoop() {
        Loop& loop = enterLoop();
        EXPR_TUPLE();
        int patch = emitCode(OP_POP_JUMP_IF_FALSE);
        compileBlockBody();
        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
    }

    void EXPR_FOR_VARS(){
        int size = 0;
        do {
            consume(TK("@id"));
            exprName(); size++;
        } while (match(TK(",")));
        if(size > 1) emitCode(OP_BUILD_SMART_TUPLE, size);
    }

    void compileForLoop() {
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        emitCode(OP_GET_ITER);
        Loop& loop = enterLoop();
        int patch = emitCode(OP_FOR_ITER);
        compileBlockBody();
        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
    }

    void compileStatement() {
        if (match(TK("break"))) {
            if (loops.empty()) syntaxError("'break' outside loop");
            consumeEndStatement();
            int patch = emitCode(OP_SAFE_JUMP_ABSOLUTE);
            getLoop().breaks.push_back(patch);
        } else if (match(TK("continue"))) {
            if (loops.empty()) syntaxError("'continue' not properly in loop");
            consumeEndStatement();
            emitCode(OP_JUMP_ABSOLUTE, getLoop().start);
        } else if (match(TK("return"))) {
            if (codes.size() == 1)
                syntaxError("'return' outside function");
            if(matchEndStatement()){
                emitCode(OP_LOAD_NONE);
            }else{
                EXPR_TUPLE();
                consumeEndStatement();
            }
            emitCode(OP_RETURN_VALUE);
        } else if (match(TK("if"))) {
            compileIfStatement();
        } else if (match(TK("while"))) {
            compileWhileLoop();
        } else if (match(TK("for"))) {
            compileForLoop();
        } else if(match(TK("assert"))){
            EXPR();
            emitCode(OP_ASSERT);
            consumeEndStatement();
        } else if(match(TK("with"))){
            EXPR();
            consume(TK("as"));
            consume(TK("@id"));
            Token tkname = parser->previous;
            int index = getCode()->addName(
                tkname.str(),
                codes.size()>1 ? NAME_LOCAL : NAME_GLOBAL
            );
            emitCode(OP_STORE_NAME_PTR, index);
            emitCode(OP_LOAD_NAME_PTR, index);
            emitCode(OP_WITH_ENTER);
            compileBlockBody();
            emitCode(OP_LOAD_NAME_PTR, index);
            emitCode(OP_WITH_EXIT);
        } else if(match(TK("label"))){
            if(mode() != EXEC_MODE) syntaxError("'label' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            getCode()->addLabel(parser->previous.str());
            consumeEndStatement();
        } else if(match(TK("goto"))){
            // https://entrian.com/goto/
            if(mode() != EXEC_MODE) syntaxError("'goto' is only available in EXEC_MODE");
            consume(TK(".")); consume(TK("@id"));
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(parser->previous.str())));
            emitCode(OP_GOTO);
            consumeEndStatement();
        } else if(match(TK("raise"))){
            consume(TK("@id"));         // dummy exception type
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(parser->previous.str())));
            consume(TK("("));EXPR();consume(TK(")"));
            emitCode(OP_RAISE_ERROR);
            consumeEndStatement();
        } else if(match(TK("del"))){
            EXPR();
            emitCode(OP_DELETE_PTR);
            consumeEndStatement();
        } else if(match(TK("global"))){
            consume(TK("@id"));
            getCode()->co_global_names.push_back(parser->previous.str());
            consumeEndStatement();
        } else if(match(TK("pass"))){
            consumeEndStatement();
        } else {
            EXPR_ANY();
            consumeEndStatement();

            // If last op is not an assignment, pop the result.
            uint8_t lastOp = getCode()->co_code.back().op;
            if( lastOp != OP_STORE_NAME_PTR && lastOp != OP_STORE_PTR){
                if(mode()==SINGLE_MODE && parser->indents.top() == 0){
                    emitCode(OP_PRINT_EXPR);
                }
                emitCode(OP_POP_TOP);
            }
        }
    }

    void compileClass(){
        consume(TK("@id"));
        int clsNameIdx = getCode()->addName(parser->previous.str(), NAME_GLOBAL);
        int superClsNameIdx = -1;
        if(match(TK("("))){
            consume(TK("@id"));
            superClsNameIdx = getCode()->addName(parser->previous.str(), NAME_GLOBAL);
            consume(TK(")"));
        }
        emitCode(OP_LOAD_NONE);
        isCompilingClass = true;
        __compileBlockBody(&Compiler::compileFunction);
        isCompilingClass = false;

        if(superClsNameIdx == -1) emitCode(OP_LOAD_NONE);
        else emitCode(OP_LOAD_NAME_PTR, superClsNameIdx);
        emitCode(OP_BUILD_CLASS, clsNameIdx);
    }

    void __compileFunctionArgs(_Func func){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state == 3) syntaxError("**kwargs should be the last argument");
            matchNewLines();
            if(match(TK("*"))){
                if(state < 1) state = 1;
                else syntaxError("*args should be placed before **kwargs");
            }
            else if(match(TK("**"))){
                state = 3;
            }

            consume(TK("@id"));
            const _Str& name = parser->previous.str();
            if(func->hasName(name)) syntaxError("duplicate argument name");

            if(state == 0 && peek() == TK("=")) state = 2;

            switch (state)
            {
                case 0: func->args.push_back(name); break;
                case 1: func->starredArg = name; state+=1; break;
                case 2: {
                    consume(TK("="));
                    PyVarOrNull value = readLiteral();
                    if(value == nullptr){
                        syntaxError(_Str("expect a literal, not ") + TK_STR(parser->current.type));
                    }
                    func->kwArgs[name] = value;
                    func->kwArgsOrder.push_back(name);
                } break;
                case 3: syntaxError("**kwargs is not supported yet"); break;
            }
        } while (match(TK(",")));
    }

    void compileFunction(){
        if(isCompilingClass){
            if(match(TK("pass"))) return;
            consume(TK("def"));
        }
        _Func func = pkpy::make_shared<Function>();
        consume(TK("@id"));
        func->name = parser->previous.str();

        if (match(TK("(")) && !match(TK(")"))) {
            __compileFunctionArgs(func);
            consume(TK(")"));
        }

        func->code = pkpy::make_shared<CodeObject>(parser->src, func->name);
        this->codes.push(func->code);
        compileBlockBody();
        this->codes.pop();
        emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyFunction(func)));
        if(!isCompilingClass) emitCode(OP_STORE_FUNCTION);
    }

    PyVarOrNull readLiteral(){
        if(match(TK("-"))){
            consume(TK("@num"));
            PyVar val = parser->previous.value;
            return vm->numNegated(val);
        }
        if(match(TK("@num"))) return parser->previous.value;
        if(match(TK("@str"))) return parser->previous.value;
        if(match(TK("True"))) return vm->PyBool(true);
        if(match(TK("False"))) return vm->PyBool(false);
        if(match(TK("None"))) return vm->None;
        if(match(TK("..."))) return vm->Ellipsis;
        return nullptr;
    }

    void compileTopLevelStatement() {
        if (match(TK("class"))) {
            compileClass();
        } else if (match(TK("def"))) {
            compileFunction();
        } else if (match(TK("import"))) {
            compileRegularImport();
        } else if (match(TK("from"))) {
            compileFromImport();
        } else {
            compileStatement();
        }
    }

    _Code __fillCode(){
        _Code code = pkpy::make_shared<CodeObject>(parser->src, _Str("<module>"));
        codes.push(code);

        // Lex initial tokens. current <-- next.
        lexToken();
        lexToken();
        matchNewLines();

        if(mode()==EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            return code;
        }else if(mode()==JSON_MODE){
            PyVarOrNull value = readLiteral();
            if(value != nullptr) emitCode(OP_LOAD_CONST, code->addConst(value));
            else if(match(TK("{"))) exprMap();
            else if(match(TK("["))) exprList();
            else syntaxError("expect a JSON object or array");
            consume(TK("@eof"));
            return code;
        }

        while (!match(TK("@eof"))) {
            compileTopLevelStatement();
            matchNewLines();
        }
        return code;
    }

    /***** Error Reporter *****/
    _Str getLineSnapshot(){
        int lineno = parser->current_line;
        if(parser->peekChar() == '\n') lineno--;
        return parser->src->snapshot(lineno);
    }

    void syntaxError(_Str msg){
        throw CompileError("SyntaxError", msg, getLineSnapshot());
    }

    void indentationError(_Str msg){
        throw CompileError("IndentationError", msg, getLineSnapshot());
    }

    void unexpectedError(_Str msg){
        throw CompileError("UnexpectedError", msg, getLineSnapshot());
    }
};

_Code compile(VM* vm, const char* source, _Str filename, CompileMode mode=EXEC_MODE, bool noThrow=true) {
    Compiler compiler(vm, source, filename, mode);
    if(!noThrow) return compiler.__fillCode();
    try{
        return compiler.__fillCode();
    }catch(std::exception& e){
        if(const _Error* _ = dynamic_cast<const _Error*>(&e)){
            (*vm->_stderr) << e.what() << '\n';
        }else{
            auto ce = CompileError("UnexpectedError", e.what(), compiler.getLineSnapshot());
            (*vm->_stderr) << ce.what() << '\n';
        }
        return nullptr;
    }
}