#pragma once

#include "pocketpy/compiler/expr.hpp"
#include "pocketpy/objects/error.hpp"

namespace pkpy {

struct Compiler;
typedef Error* (Compiler::*PrattCallback)() noexcept;

struct PrattRule {
    PrattCallback prefix;
    PrattCallback infix;
    Precedence precedence;
};

struct Compiler {
    PK_ALWAYS_PASS_BY_POINTER(Compiler)

    static PrattRule rules[kTokenCount];

    Lexer lexer;
    vector<CodeEmitContext> contexts;
    VM* vm;
    bool unknown_global_scope;  // for eval/exec() call
    // for parsing token stream
    int __i = 0;

    const Token& tk(int i) const noexcept{ return lexer.nexts[i]; }
    const Token& prev() const noexcept{ return tk(__i - 1); }
    const Token& curr() const noexcept{ return tk(__i); }
    const Token& next() const noexcept{ return tk(__i + 1); }

    const Token& err() const noexcept{
        if(__i >= lexer.nexts.size()) return prev();
        return curr();
    }

    void advance(int delta = 1) noexcept{
        __i += delta;
#if PK_DEBUG_COMPILER
        if(__i>=0 && __i<lexer.nexts.size()){
            printf("%s:%d %s %s\n",
                lexer.src->filename.c_str(),
                curr().line,
                TK_STR(curr().type),
                curr().str().escape().c_str()
            );
        }
#endif
    }

    CodeEmitContext* ctx() noexcept{ return &contexts.back(); }
    pkpy_CompileMode mode() const noexcept{ return lexer.src->mode; }

    NameScope name_scope() const noexcept;
    CodeObject_ push_global_context() noexcept;
    FuncDecl_ push_f_context(Str name) noexcept;

    static void init_pratt_rules() noexcept;

    bool match(TokenIndex expected) noexcept;
    bool match_end_stmt() noexcept;
    bool match_newlines(bool* need_more_lines = NULL) noexcept;
    /*************************************************/
    [[nodiscard]] Error* EXPR() noexcept{ return parse_expression(PREC_LOWEST + 1); }
    [[nodiscard]] Error* EXPR_TUPLE(bool allow_slice = false) noexcept;
    [[nodiscard]] Error* EXPR_VARS() noexcept;  // special case for `for loop` and `comp`

    template <typename T, typename... Args>
    T* make_expr(Args&&... args) noexcept{
        static_assert(sizeof(T) <= kPoolExprBlockSize);
        static_assert(std::is_base_of_v<Expr, T>);
        void* p = PoolExpr_alloc();
        T* expr = new (p) T(std::forward<Args>(args)...);
        expr->line = prev().line;
        return expr;
    }

    [[nodiscard]] Error* consume_comp(Opcode op0, Opcode op1) noexcept;
    [[nodiscard]] Error* pop_context() noexcept;

    Error* exprLiteral() noexcept;
    Error* exprLong() noexcept;
    Error* exprImag() noexcept;
    Error* exprBytes() noexcept;
    Error* exprFString() noexcept;
    Error* exprLambda() noexcept;
    Error* exprOr() noexcept;
    Error* exprAnd() noexcept;
    Error* exprTernary() noexcept;
    Error* exprBinaryOp() noexcept;
    Error* exprNot() noexcept;
    Error* exprUnaryOp() noexcept;
    Error* exprGroup() noexcept;
    Error* exprList() noexcept;
    Error* exprMap() noexcept;
    Error* exprCall() noexcept;
    Error* exprName() noexcept;
    Error* exprAttrib() noexcept;
    Error* exprSlice0() noexcept;
    Error* exprSlice1() noexcept;
    Error* exprSubscr() noexcept;
    Error* exprLiteral0() noexcept;

    bool is_expression(bool allow_slice = false) noexcept;

    [[nodiscard]] Error* compile_block_body(PrattCallback callback = NULL) noexcept;
    [[nodiscard]] Error* compile_normal_import() noexcept;
    [[nodiscard]] Error* compile_from_import() noexcept;
    [[nodiscard]] Error* parse_expression(int precedence, bool allow_slice = false) noexcept;
    [[nodiscard]] Error* compile_if_stmt() noexcept;
    [[nodiscard]] Error* compile_while_loop() noexcept;
    [[nodiscard]] Error* compile_for_loop() noexcept;
    [[nodiscard]] Error* compile_try_except() noexcept;
    [[nodiscard]] Error* compile_decorated() noexcept;

    [[nodiscard]] Error* try_compile_assignment(bool* is_assign) noexcept;
    [[nodiscard]] Error* compile_stmt() noexcept;
    [[nodiscard]] Error* consume_type_hints() noexcept;
    [[nodiscard]] Error* _compile_f_args(FuncDecl_ decl, bool enable_type_hints) noexcept;
    [[nodiscard]] Error* compile_function(int decorators = 0) noexcept;
    [[nodiscard]] Error* compile_class(int decorators = 0) noexcept;

    PyVar to_object(const TokenValue& value) noexcept;

    [[nodiscard]] Error* read_literal(PyVar* out) noexcept;

    [[nodiscard]] Error* SyntaxError(const char* msg = "invalid syntax", ...) noexcept;
    [[nodiscard]] Error* IndentationError(const char* msg) noexcept{ return lexer._error(false, "IndentationError", msg, {}); }
    [[nodiscard]] Error* NeedMoreLines() noexcept{
        return lexer._error(false, "NeedMoreLines", "", {}, (i64)ctx()->is_compiling_class);
    }

public:
    Compiler(VM* vm, std::string_view source, const Str& filename, pkpy_CompileMode mode, bool unknown_global_scope = false) noexcept;
    [[nodiscard]] Error* compile(CodeObject_* out) noexcept;
    ~Compiler();
};

}  // namespace pkpy
