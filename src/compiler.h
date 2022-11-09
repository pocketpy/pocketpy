#pragma once

#include <vector>
#include <string>
#include <cstring>

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
    bool forLoop;
    int start;
    std::vector<int> breaks;
    Loop(bool forLoop, int start) : forLoop(forLoop), start(start) {}
};

#define ExprCommaSplitArgs(end)     \
    int ARGC = 0;                   \
    do {                            \
        matchNewLines();            \
        if (peek() == TK(end)) break;   \
        EXPR();                     \
        ARGC++;                     \
        matchNewLines();            \
    } while (match(TK(",")));       \
    matchNewLines();                \
    consume(TK(end));

class Compiler {
public:
    std::unique_ptr<Parser> parser;
    std::stack<_Code> codes;
    std::stack<Loop> loops;

    CompileMode mode;

    bool isCompilingClass = false;

    _Str path = "<?>";
    VM* vm;

    std::unordered_map<_TokenType, GrammarRule> rules;

    _Code getCode() {
        return codes.top();
    }

    Loop& getLoop() {
        return loops.top();
    }

    Compiler(VM* vm, const char* source, _Code code){
        this->vm = vm;
        this->codes.push(code);
        this->mode = code->mode;
        if (!code->co_filename.empty()) path = code->co_filename;
        this->parser = std::make_unique<Parser>(source);

// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define METHOD(name) &Compiler::name
#define NO_INFIX nullptr, PREC_NONE
        for(_TokenType i=0; i<__TOKENS_LEN; i++) rules[i] = { nullptr, NO_INFIX };
        rules[TK(".")] =    { nullptr,               METHOD(exprAttrib),         PREC_ATTRIB };
        rules[TK("(")] =    { METHOD(exprGrouping),  METHOD(exprCall),           PREC_CALL };
        rules[TK("[")] =    { METHOD(exprList),      METHOD(exprSubscript),      PREC_SUBSCRIPT };
        rules[TK("{")] =    { METHOD(exprMap),       NO_INFIX };
        rules[TK("%")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
        rules[TK("+")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("-")] =    { METHOD(exprUnaryOp),   METHOD(exprBinaryOp),       PREC_TERM };
        rules[TK("*")] =    { nullptr,               METHOD(exprBinaryOp),       PREC_FACTOR };
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
        rules[TK("@id")] =      { METHOD(exprName),      NO_INFIX };
        rules[TK("@num")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@str")] =     { METHOD(exprLiteral),   NO_INFIX };
        rules[TK("@fstr")] =    { METHOD(exprFString),   NO_INFIX };
        rules[TK("=")] =        { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("+=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("-=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("*=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("/=")] =       { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK("//=")] =      { nullptr,               METHOD(exprAssign),         PREC_ASSIGNMENT };
        rules[TK(",")] =        { nullptr,               METHOD(exprComma),          PREC_COMMA };
#undef METHOD
#undef NO_INFIX

#define EXPR() parsePrecedence(PREC_LOGICAL_OR)             // no '=' and ',' just a simple expression
#define EXPR_TUPLE() parsePrecedence(PREC_COMMA)            // no '=', but ',' is allowed
#define EXPR_ANY() parsePrecedence(PREC_ASSIGNMENT)
    }

    _Str eatStringUntil(char quote) {
        std::vector<char> buff;
        while (true) {
            char c = parser->eatChar();
            if (c == quote) break;
            if (c == '\0')
                throw SyntaxError(path, parser->makeErrToken(), "EOL while scanning string literal");
            if (c == '\\') {
                switch (parser->eatCharIncludeNewLine()) {
                    case '"':  buff.push_back('"');  break;
                    case '\'': buff.push_back('\''); break;
                    case '\\': buff.push_back('\\'); break;
                    case 'n':  buff.push_back('\n'); break;
                    case 'r':  buff.push_back('\r'); break;
                    case 't':  buff.push_back('\t'); break;
                    case '\n': case '\r': break;
                    default: throw SyntaxError(path, parser->makeErrToken(), "invalid escape character");
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
        static const std::regex pattern("^[+-]?([0-9]+)(\\.[0-9]+)?");
        std::smatch m;

        const char* i = parser->token_start;
        while(*i != '\n' && *i != '\0') i++;
        std::string s = std::string(parser->token_start, i);

        try{
            if (std::regex_search(s, m, pattern)) {
                // here is m.length()-1, since the first char is eaten by lexToken()
                for(int j=0; j<m.length()-1; j++) parser->eatChar();
                if (m[2].matched) {
                    parser->setNextToken(TK("@num"), vm->PyFloat(std::stof(m[0])));
                } else {
                    parser->setNextToken(TK("@num"), vm->PyInt(std::stoi(m[0])));
                }  
            }
        }catch(std::exception& e){
            throw SyntaxError(path, parser->makeErrToken(), "invalid number (%s)", e.what());
        }
    }

    // Lex the next token and set it as the next token.
    void lexToken() {
        parser->previous = parser->current;
        parser->current = parser->nextToken();
        //printf("<%s> ", TK_STR(peek()));

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
                case '.': parser->setNextToken(TK(".")); return;                
                case '=': parser->setNextTwoCharToken('=', TK("="), TK("==")); return;
                case '>': parser->setNextTwoCharToken('=', TK(">"), TK(">=")); return;
                case '<': parser->setNextTwoCharToken('=', TK("<"), TK("<=")); return;
                case '+': parser->setNextTwoCharToken('=', TK("+"), TK("+=")); return;
                case '-': {
                    if(isdigit(parser->peekChar())) eatNumber();
                    else parser->setNextTwoCharToken('=', TK("-"), TK("-="));
                    return;
                }
                case '!':
                    if(parser->matchChar('=')) parser->setNextToken(TK("!="));
                    else SyntaxError(path, parser->makeErrToken(), "expected '=' after '!'");
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
                    parser->setNextToken(TK("@eol"));
                    while(parser->matchChar('\n'));
                    if(!parser->eatIndentation())
                        throw SyntaxError(path, parser->makeErrToken(), "unindent does not match any outer indentation level");
                    return;
                }
                default: {
                    if (isdigit(c)) {
                        eatNumber();
                    } else if (isalpha(c) || c=='_') {
                        if(c == 'f'){
                            if(parser->matchChar('\'')) {eatString('\'', true); return;}
                            if(parser->matchChar('"')) {eatString('"', true); return;}
                        }
                        parser->eatName();
                    } else {
                        throw SyntaxError(path, parser->makeErrToken(), "unknown character: %c", c);
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
        lexToken();
        Token prev = parser->previous;
        if (prev.type != expected){
            throw SyntaxError(path, prev, "expected '%s', but got '%s'", TK_STR(expected), TK_STR(prev.type));
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
        if (!matchEndStatement())
            throw SyntaxError(path, parser->current, "expected statement end");
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
            i = m.position() + m.length();
        }
        if (i < s.size()) {
            std::string literal = s.substr(i, s.size() - i);
            emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyStr(literal)));
            size++;
        }
        emitCode(OP_BUILD_STRING, size);
    }

    void exprLambda() {
        _Func func;
        func.name = "<lambda>";
        __compileFunctionArgs(func);
        consume(TK(":"));
        func.code = std::make_shared<CodeObject>();
        func.code->co_name = func.name;
        func.code->co_filename = path;
        this->codes.push(func.code);
        EXPR_TUPLE();
        emitCode(OP_RETURN_VALUE);
        this->codes.pop();
        emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyFunction(func)));
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
            default: UNREACHABLE();
        }
    }

    void exprGrouping() {
        matchNewLines();
        EXPR_TUPLE();
        matchNewLines();
        consume(TK(")"));
    }

    void exprList() {
        int _patch = emitCode(OP_NO_OP);
        int _body_start = getCode()->co_code.size();
        int ARGC = 0;
        do {
            matchNewLines();
            if (peek() == TK("]")) break;
            EXPR(); ARGC++;
            matchNewLines();
            if(ARGC == 1 && match(TK("for"))) goto __LISTCOMP;
        } while (match(TK(",")));
        matchNewLines();
        consume(TK("]"));
        emitCode(OP_BUILD_LIST, ARGC);
        return;

__LISTCOMP:
        int _body_end = getCode()->co_code.size();
        getCode()->co_code[_patch].op = OP_JUMP_ABSOLUTE;
        getCode()->co_code[_patch].arg = _body_end;
        emitCode(OP_BUILD_LIST, 0);
        EXPR_FOR_VARS();consume(TK("in"));EXPR_TUPLE();
        matchNewLines();
        
        int _skipPatch = emitCode(OP_JUMP_ABSOLUTE);
        int _cond_start = getCode()->co_code.size();
        if(match(TK("if"))) EXPR_TUPLE();
        int _cond_end = getCode()->co_code.size();
        patchJump(_skipPatch);

        emitCode(OP_GET_ITER);
        Loop& loop = enterLoop(true);
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
        consume(TK("]"));
    }

    void exprMap() {
        int size = 0;
        do {
            matchNewLines();
            if (peek() == TK("}")) break;
            EXPR();consume(TK(":"));EXPR();
            size++;
            matchNewLines();
        } while (match(TK(",")));
        matchNewLines();
        consume(TK("}"));
        emitCode(OP_BUILD_MAP, size);
    }

    void exprCall() {
        ExprCommaSplitArgs(")");
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
            case TK("None"):  emitCode(OP_LOAD_NONE);  break;
            case TK("True"):  emitCode(OP_LOAD_TRUE);  break;
            case TK("False"): emitCode(OP_LOAD_FALSE); break;
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
        if(!matchNewLines(mode==SINGLE_MODE)){
            throw SyntaxError(path, parser->previous, "expected a new line after ':'");
        }
        consume(TK("@indent"));
        while (peek() != TK("@dedent")) {
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

    // import module1 [as alias1 [, module2 [as alias2 ...]]
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

    void parsePrecedence(Precedence precedence) {
        lexToken();
        GrammarFn prefix = rules[parser->previous.type].prefix;

        if (prefix == nullptr) {
            throw SyntaxError(path, parser->previous, "expected an expression");
        }

        (this->*prefix)();
        while (rules[peek()].precedence >= precedence) {
            lexToken();
            _TokenType op = parser->previous.type;
            GrammarFn infix = rules[op].infix;
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

    Loop& enterLoop(bool forLoop){
        Loop lp(forLoop, (int)getCode()->co_code.size());
        loops.push(lp);
        return loops.top();
    }

    void exitLoop(){
        Loop& lp = loops.top();
        for(int addr : lp.breaks) patchJump(addr);
        loops.pop();
    }

    void compileWhileLoop() {
        Loop& loop = enterLoop(false);
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
        Loop& loop = enterLoop(true);
        int patch = emitCode(OP_FOR_ITER);
        compileBlockBody();
        emitCode(OP_JUMP_ABSOLUTE, loop.start); keepOpcodeLine();
        patchJump(patch);
        exitLoop();
    }

    void compileStatement() {
        if (match(TK("break"))) {
            if (loops.empty()) throw SyntaxError(path, parser->previous, "'break' outside loop");
            consumeEndStatement();
            if(getLoop().forLoop) emitCode(OP_POP_TOP); // pop the iterator of for loop.
            int patch = emitCode(OP_JUMP_ABSOLUTE);
            getLoop().breaks.push_back(patch);
        } else if (match(TK("continue"))) {
            if (loops.empty()) {
                throw SyntaxError(path, parser->previous, "'continue' not properly in loop");
            }
            consumeEndStatement();
            emitCode(OP_JUMP_ABSOLUTE, getLoop().start);
        } else if (match(TK("return"))) {
            if (codes.size() == 1)
                throw SyntaxError(path, parser->previous, "'return' outside function");
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
        } else if(match(TK("pass"))){
            consumeEndStatement();
        } else {
            EXPR_ANY();
            consumeEndStatement();

            // If last op is not an assignment, pop the result.
            uint8_t lastOp = getCode()->co_code.back().op;
            if( lastOp != OP_STORE_NAME_PTR && lastOp != OP_STORE_PTR){
                if(mode==SINGLE_MODE && parser->indents.top() == 0){
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

    void __compileFunctionArgs(_Func& func){
        int state = 0;      // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
        do {
            if(state == 3){
                throw SyntaxError(path, parser->previous, "**kwargs should be the last argument");
            }

            matchNewLines();
            if(match(TK("*"))){
                if(state < 1) state = 1;
                else throw SyntaxError(path, parser->previous, "*args should be placed before **kwargs");
            }
            else if(match(TK("**"))){
                state = 3;
            }

            consume(TK("@id"));
            const _Str& name = parser->previous.str();
            if(func.hasName(name)) throw SyntaxError(path, parser->previous, "duplicate argument name");

            if(state == 0 && peek() == TK("=")) state = 2;

            switch (state)
            {
                case 0: func.args.push_back(name); break;
                case 1: func.starredArg = name; state+=1; break;
                case 2: consume(TK("=")); func.kwArgs[name] = consumeLiteral(); break;
                case 3: func.doubleStarredArg = name; break;
            }
        } while (match(TK(",")));
    }

    void compileFunction(){
        if(isCompilingClass){
            if(match(TK("pass"))) return;
            consume(TK("def"));
        }
        _Func func;
        consume(TK("@id"));
        func.name = parser->previous.str();

        if (match(TK("(")) && !match(TK(")"))) {
            __compileFunctionArgs(func);
            consume(TK(")"));
        }

        func.code = std::make_shared<CodeObject>();
        func.code->co_name = func.name;
        func.code->co_filename = path;
        this->codes.push(func.code);
        compileBlockBody();
        this->codes.pop();
        emitCode(OP_LOAD_CONST, getCode()->addConst(vm->PyFunction(func)));
        if(!isCompilingClass) emitCode(OP_STORE_FUNCTION);
    }

    PyVar consumeLiteral(){
        if(match(TK("@num"))) goto __LITERAL_EXIT;
        if(match(TK("@str"))) goto __LITERAL_EXIT;
        if(match(TK("True"))) goto __LITERAL_EXIT;
        if(match(TK("False"))) goto __LITERAL_EXIT;
        if(match(TK("None"))) goto __LITERAL_EXIT;
        throw SyntaxError(path, parser->previous, "expect a literal, not %s", TK_STR(parser->current.type));
__LITERAL_EXIT:
        return parser->previous.value;
    }

    void compileTopLevelStatement() {
        if (match(TK("class"))) {
            compileClass();
        } else if (match(TK("def"))) {
            compileFunction();
        } else if (match(TK("import"))) {
            compileRegularImport();
        } else {
            compileStatement();
        }
    }

    void __fillCode(){
        // Lex initial tokens. current <-- next.
        lexToken();
        lexToken();
        matchNewLines();

        if(mode == EVAL_MODE) {
            EXPR_TUPLE();
            consume(TK("@eof"));
            return;
        }

        while (!match(TK("@eof"))) {
            compileTopLevelStatement();
            matchNewLines();
        }
    }
};

_Code compile(VM* vm, const char* source, _Str filename, CompileMode mode=EXEC_MODE) {
    // Skip utf8 BOM if there is any.
    if (strncmp(source, "\xEF\xBB\xBF", 3) == 0) source += 3;

    _Code code = std::make_shared<CodeObject>();
    code->co_filename = filename;
    code->mode = mode;

    Compiler compiler(vm, source, code);
    compiler.__fillCode();
    return code;
}