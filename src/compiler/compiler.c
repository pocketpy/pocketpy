#include "pocketpy/compiler/compiler.h"
#include "pocketpy/compiler/expr.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/compiler/context.h"

typedef struct pk_Compiler pk_Compiler;
typedef Error* (*PrattCallback)(pk_Compiler* self);

typedef struct PrattRule {
    PrattCallback prefix;
    PrattCallback infix;
    enum Precedence precedence;
} PrattRule;

const static PrattRule rules[TK__COUNT__];

typedef struct pk_Compiler {
    pk_SourceData_ src;         // weakref
    pk_TokenArray tokens;
    int i;
    c11_vector/*T=CodeEmitContext*/ contexts;
} pk_Compiler;

static void pk_Compiler__ctor(pk_Compiler *self, pk_SourceData_ src, pk_TokenArray tokens){
    self->src = src;
    self->tokens = tokens;
    self->i = 0;
    c11_vector__ctor(&self->contexts, sizeof(pk_CodeEmitContext));
}

static void pk_Compiler__dtor(pk_Compiler *self){
    pk_TokenArray__dtor(&self->tokens);
    c11_vector__dtor(&self->contexts);
}

/**************************************/
#define tk(i)       c11__getitem(Token, &self->tokens, i)
#define prev()      tk(self->i - 1)
#define curr()      tk(self->i)
#define next()      tk(self->i + 1)
#define err()       (self->i == self->tokens.count ? prev() : curr())

#define advance()   self->i++
#define mode()      self->src->mode
#define ctx()       c11_vector__back(pk_CodeEmitContext, &self->contexts)

#define match_newlines() match_newlines_repl(self, NULL)

#define consume(expected) if(!match(expected)) return SyntaxError("expected '%s', got '%s'", pk_TokenSymbols[expected], pk_TokenSymbols[curr().type]);
#define consume_end_stmt() if(!match_end_stmt()) return SyntaxError("expected statement end")
#define check_newlines_repl() { bool __nml; match_newlines_repl(self, &__nml); if(__nml) return NeedMoreLines(); }
#define check(B) if((err = B)) return err

static NameScope name_scope(pk_Compiler* self) {
    NameScope s = self->contexts.count > 1 ? NAME_LOCAL : NAME_GLOBAL;
    if(self->src->is_dynamic && s == NAME_GLOBAL) s = NAME_GLOBAL_UNKNOWN;
    return s;
}

static Error* SyntaxError(const char* fmt, ...){
    return NULL;
}

static Error* NeedMoreLines(){
    return NULL;
}

/* Matchers */
static bool is_expression(pk_Compiler* self, bool allow_slice){
    PrattCallback prefix = rules[curr().type].prefix;
    return prefix && (allow_slice || curr().type != TK_COLON);
}

#define match(expected) (curr().type == expected ? (++self->i) : 0)

static bool match_newlines_repl(pk_Compiler* self, bool* need_more_lines){
    bool consumed = false;
    if(curr().type == TK_EOL) {
        while(curr().type == TK_EOL) advance();
        consumed = true;
    }
    if(need_more_lines) {
        *need_more_lines = (mode() == REPL_MODE && curr().type == TK_EOF);
    }
    return consumed;
}

static bool match_end_stmt(pk_Compiler* self) {
    if(match(TK_SEMICOLON)) {
        match_newlines();
        return true;
    }
    if(match_newlines() || curr().type == TK_EOF) return true;
    if(curr().type == TK_DEDENT) return true;
    return false;
}

/* Expression Callbacks */
static Error* exprLiteral(pk_Compiler* self);
static Error* exprLong(pk_Compiler* self);
static Error* exprImag(pk_Compiler* self);
static Error* exprBytes(pk_Compiler* self);
static Error* exprFString(pk_Compiler* self);
static Error* exprLambda(pk_Compiler* self);
static Error* exprOr(pk_Compiler* self);
static Error* exprAnd(pk_Compiler* self);
static Error* exprTernary(pk_Compiler* self);
static Error* exprBinaryOp(pk_Compiler* self);
static Error* exprNot(pk_Compiler* self);
static Error* exprUnaryOp(pk_Compiler* self);
static Error* exprGroup(pk_Compiler* self);
static Error* exprList(pk_Compiler* self);
static Error* exprMap(pk_Compiler* self);
static Error* exprCall(pk_Compiler* self);
static Error* exprName(pk_Compiler* self);
static Error* exprAttrib(pk_Compiler* self);
static Error* exprSlice0(pk_Compiler* self);
static Error* exprSlice1(pk_Compiler* self);
static Error* exprSubscr(pk_Compiler* self);
static Error* exprLiteral0(pk_Compiler* self);

/* Expression */
static Error* parse_expression(pk_Compiler* self, int precedence, bool allow_slice){
    PrattCallback prefix = rules[curr().type].prefix;
    if(!prefix || (curr().type == TK_COLON && !allow_slice)) {
        return SyntaxError("expected an expression, got %s", pk_TokenSymbols[curr().type]);
    }
    advance();
    Error* err;
    check(prefix(self));
    while(rules[curr().type].precedence >= precedence && (allow_slice || curr().type != TK_COLON)) {
        TokenIndex op = curr().type;
        advance();
        PrattCallback infix = rules[op].infix;
        assert(infix != NULL);
        check(infix(self));
    }
    return NULL;
}

static Error* EXPR(pk_Compiler* self) {
    return parse_expression(self, PREC_LOWEST + 1, false);
}

static Error* EXPR_TUPLE(pk_Compiler* self, bool allow_slice){
    Error* err;
    check(parse_expression(self, PREC_LOWEST + 1, allow_slice));
    if(!match(TK_COMMA)) return NULL;
    // tuple expression
    int count = 1;
    do {
        if(curr().brackets_level) check_newlines_repl()
        if(!is_expression(self, allow_slice)) break;
        check(parse_expression(self, PREC_LOWEST + 1, allow_slice));
        count += 1;
        if(curr().brackets_level) check_newlines_repl();
    } while(match(TK_COMMA));
    // TupleExpr* e = make_expr<TupleExpr>(count);
    // for(int i=count-1; i>=0; i--)
    //     e->items[i] = ctx()->s_popx();
    // ctx()->s_push(e);
    return NULL;
}

// special case for `for loop` and `comp`
static Error* EXPR_VARS(pk_Compiler* self){
    // int count = 0;
    // do {
    //     consume(TK_ID);
    //     ctx()->s_push(make_expr<NameExpr>(prev().str(), name_scope()));
    //     count += 1;
    // } while(match(TK_COMMA));
    // if(count > 1){
    //     TupleExpr* e = make_expr<TupleExpr>(count);
    //     for(int i=count-1; i>=0; i--)
    //         e->items[i] = ctx()->s_popx();
    //     ctx()->s_push(e);
    // }
    return NULL;
}

static void setup_global_context(pk_Compiler* self, CodeObject* co){
    co->start_line = self->i == 0 ? 1 : prev().line;
    pk_CodeEmitContext* ctx = c11_vector__emplace(&self->contexts);
    pk_CodeEmitContext__ctor(ctx, co, NULL, self->contexts.count);
}

Error* pk_Compiler__compile(pk_Compiler* self, CodeObject* out){
    // make sure it is the first time to compile
    assert(self->i == 0);
    // make sure the first token is @sof
    assert(tk(0).type == TK_SOF);

    setup_global_context(self, out);

    advance();              // skip @sof, so prev() is always valid
    match_newlines();       // skip possible leading '\n'

    Error* err;
    // if(mode() == EVAL_MODE) {
    //     check(EXPR_TUPLE());
    //     ctx()->s_emit_top();
    //     consume(TK_EOF);
    //     ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
    //     check(pop_context());
    //     return NULL;
    // } else if(mode() == JSON_MODE) {
    //     check(EXPR());
    //     Expr* e = ctx()->s_popx();
    //     if(!e->is_json_object()){
    //         return SyntaxError("expect a JSON object, literal or array");
    //     }
    //     consume(TK_EOF);
    //     e->emit_(ctx());
    //     ctx()->emit_(OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
    //     check(pop_context());
    //     return NULL;
    // }

    // while(!match(TK_EOF)) {
    //     check(compile_stmt());
    //     match_newlines();
    // }
    // check(pop_context());
    return NULL;
}



Error* pk_compile(pk_SourceData_ src, CodeObject* out){
    pk_TokenArray tokens;
    Error* err = pk_Lexer__process(src, &tokens);
    if(err) return err;

    // Token* data = (Token*)tokens.data;
    // printf("%s\n", py_Str__data(&src->filename));
    // for(int i = 0; i < tokens.count; i++) {
    //     Token* t = data + i;
    //     py_Str tmp;
    //     py_Str__ctor2(&tmp, t->start, t->length);
    //     printf("[%d] %s: %s\n", t->line, pk_TokenSymbols[t->type], py_Str__data(&tmp));
    //     py_Str__dtor(&tmp);
    // }

    pk_Compiler compiler;
    pk_Compiler__ctor(&compiler, src, tokens);
    CodeObject__ctor(out, src, py_Str__sv(&src->filename));
    err = pk_Compiler__compile(&compiler, out);
    CodeObject__dtor(out);
    pk_Compiler__dtor(&compiler);
    return err;
}

const static PrattRule rules[TK__COUNT__] = {
    // clang-format off
// http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
    [TK_DOT] =         { NULL,          exprAttrib,         PREC_PRIMARY    },
    [TK_LPAREN] =      { exprGroup,     exprCall,           PREC_PRIMARY    },
    [TK_LBRACKET] =    { exprList,      exprSubscr,         PREC_PRIMARY    },
    [TK_MOD] =         { NULL,          exprBinaryOp,       PREC_FACTOR     },
    [TK_ADD] =         { NULL,          exprBinaryOp,       PREC_TERM       },
    [TK_SUB] =         { exprUnaryOp,   exprBinaryOp,       PREC_TERM       },
    [TK_MUL] =         { exprUnaryOp,   exprBinaryOp,       PREC_FACTOR     },
    [TK_INVERT] =      { exprUnaryOp,   NULL,               PREC_UNARY      },
    [TK_DIV] =         { NULL,          exprBinaryOp,       PREC_FACTOR     },
    [TK_FLOORDIV] =    { NULL,          exprBinaryOp,       PREC_FACTOR     },
    [TK_POW] =         { exprUnaryOp,   exprBinaryOp,       PREC_EXPONENT   },
    [TK_GT] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_LT] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_EQ] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_NE] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_GE] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_LE] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_IN] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_IS] =          { NULL,          exprBinaryOp,       PREC_COMPARISION },
    [TK_LSHIFT] =      { NULL,          exprBinaryOp,       PREC_BITWISE_SHIFT },
    [TK_RSHIFT] =      { NULL,          exprBinaryOp,       PREC_BITWISE_SHIFT },
    [TK_AND] =         { NULL,          exprBinaryOp,       PREC_BITWISE_AND   },
    [TK_OR] =          { NULL,          exprBinaryOp,       PREC_BITWISE_OR    },
    [TK_XOR] =         { NULL,          exprBinaryOp,       PREC_BITWISE_XOR   },
    [TK_DECORATOR] =   { NULL,          exprBinaryOp,       PREC_FACTOR        },
    [TK_IF] =          { NULL,          exprTernary,        PREC_TERNARY       },
    [TK_NOT_IN] =      { NULL,          exprBinaryOp,       PREC_COMPARISION   },
    [TK_IS_NOT] =      { NULL,          exprBinaryOp,       PREC_COMPARISION   },
    [TK_AND_KW ] =     { NULL,          exprAnd,            PREC_LOGICAL_AND   },
    [TK_OR_KW] =       { NULL,          exprOr,             PREC_LOGICAL_OR    },
    [TK_NOT_KW] =      { exprNot,       NULL,               PREC_LOGICAL_NOT   },
    [TK_TRUE] =        { exprLiteral0 },
    [TK_FALSE] =       { exprLiteral0 },
    [TK_NONE] =        { exprLiteral0 },
    [TK_DOTDOTDOT] =   { exprLiteral0 },
    [TK_LAMBDA] =      { exprLambda,  },
    [TK_ID] =          { exprName,    },
    [TK_NUM] =         { exprLiteral, },
    [TK_STR] =         { exprLiteral, },
    [TK_FSTR] =        { exprFString, },
    [TK_LONG] =        { exprLong,    },
    [TK_IMAG] =        { exprImag,    },
    [TK_BYTES] =       { exprBytes,   },
    [TK_LBRACE] =      { exprMap      },
    [TK_COLON] =       { exprSlice0,    exprSlice1,      PREC_PRIMARY }
};