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

class Compiler {
    PK_ALWAYS_PASS_BY_POINTER(Compiler)

    static PrattRule rules[kTokenCount];

    Lexer lexer;
    stack_no_copy<CodeEmitContext> contexts;
    VM* vm;
    bool unknown_global_scope;     // for eval/exec() call
    // for parsing token stream
    int i = 0;
    std::vector<Token> tokens;

    const Token& prev() const{ return tokens.at(i-1); }
    const Token& curr() const{ return tokens.at(i); }
    const Token& next() const{ return tokens.at(i+1); }
    const Token& err() const{
        if(i >= tokens.size()) return prev();
        return curr();
    }
    void advance(int delta=1) { i += delta; }

    CodeEmitContext* ctx() { return &contexts.top(); }
    CompileMode mode() const{ return lexer.src->mode; }
    NameScope name_scope() const;
    CodeObject_ push_global_context();
    FuncDecl_ push_f_context(Str name);
    void pop_context();

    static void init_pratt_rules();

    bool match(TokenIndex expected);
    void consume(TokenIndex expected);
    bool match_newlines_repl();

    bool match_newlines(bool repl_throw=false);
    bool match_end_stmt();
    void consume_end_stmt();

    /*************************************************/
    void EXPR();
    void EXPR_TUPLE(bool allow_slice=false);
    Expr_ EXPR_VARS();  // special case for `for loop` and `comp`

    template <typename T, typename... Args>
    unique_ptr_128<T> make_expr(Args&&... args) {
        void* p = pool128_alloc(sizeof(T));
        unique_ptr_128<T> expr(new (p) T(std::forward<Args>(args)...));
        expr->line = prev().line;
        return expr;
    }

    void consume_comp(unique_ptr_128<CompExpr> ce, Expr_ expr);

    void exprLiteral();
    void exprLong();
    void exprImag();
    void exprBytes();
    void exprFString();
    void exprLambda();
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
    void exprSlice0();
    void exprSlice1();
    void exprSubscr();
    void exprLiteral0();

    void compile_block_body(void (Compiler::*callback)()=nullptr);
    void compile_normal_import();
    void compile_from_import();
    bool is_expression(bool allow_slice=false);
    void parse_expression(int precedence, bool allow_slice=false);
    void compile_if_stmt();
    void compile_while_loop();
    void compile_for_loop();
    void compile_try_except();
    void compile_decorated();

    bool try_compile_assignment();
    void compile_stmt();
    void consume_type_hints();
    void _add_decorators(const Expr_vector& decorators);
    void compile_class(const Expr_vector& decorators={});
    void _compile_f_args(FuncDecl_ decl, bool enable_type_hints);
    void compile_function(const Expr_vector& decorators={});

    PyObject* to_object(const TokenValue& value);
    PyObject* read_literal();

    void SyntaxError(Str msg){ lexer.throw_err("SyntaxError", msg, err().line, err().start); }
    void SyntaxError(){ lexer.throw_err("SyntaxError", "invalid syntax", err().line, err().start); }
    void IndentationError(Str msg){ lexer.throw_err("IndentationError", msg, err().line, err().start); }

public:
    Compiler(VM* vm, std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope=false);
    Str precompile();
    void from_precompiled(const char* source);
    CodeObject_ compile();
};

struct TokenDeserializer{
    const char* curr;
    const char* source;

    TokenDeserializer(const char* source): curr(source), source(source) {}
    char read_char(){ return *curr++; }
    bool match_char(char c){ if(*curr == c) { curr++; return true; } return false; }
    
    std::string_view read_string(char c);
    Str read_string_from_hex(char c);
    int read_count();
    i64 read_uint(char c);
    f64 read_float(char c);
};

} // namespace pkpy