#include "pocketpy/compiler/expr.h"
#include "pocketpy/compiler/context.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/strname.h"
#include <ctype.h>

static bool default_false(const pk_Expr* e) { return false; }
static int default_zero(const pk_Expr* e) { return 0; }
static void default_dtor(pk_Expr* e) {}

static bool default_emit_del(pk_Expr* e, pk_CodeEmitContext* ctx) { return false; }
static bool default_emit_store(pk_Expr* e, pk_CodeEmitContext* ctx) { return false; }
static void default_emit_inplace(pk_Expr* e, pk_CodeEmitContext* ctx) { e->vt->emit_(e, ctx); }
static bool default_emit_store_inplace(pk_Expr* e, pk_CodeEmitContext* ctx) { return e->vt->emit_store(e, ctx); }

void pk_ExprVt__ctor(pk_ExprVt* vt){
    vt->dtor = default_dtor;
    vt->star_level = default_zero;
    vt->is_compare = default_false;
    vt->emit_ = NULL;   // must be set
    vt->emit_del = default_emit_del;
    vt->emit_store = default_emit_store;
    vt->emit_inplace = default_emit_inplace;
    vt->emit_store_inplace = default_emit_store_inplace;
}

void pk_Expr__delete(pk_Expr* self){
    if(!self) return;
    self->vt->dtor(self);
    PoolExpr_dealloc(self);
}

/* Implementations */
#define static_assert_expr_size(T) static_assert(sizeof(T) <= kPoolExprBlockSize, "size is too large")

static pk_ExprVt NameExprVt;

void pk_NameExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_NameExpr* self = (pk_NameExpr*)self_;
    int index = c11_smallmap_n2i__get(&ctx->co->varnames_inv, self->name, -1);
    if(self->scope == NAME_LOCAL && index >= 0) {
        pk_CodeEmitContext__emit_(ctx, OP_LOAD_FAST, index, self->line);
    } else {
        Opcode op = ctx->level <= 1 ? OP_LOAD_GLOBAL : OP_LOAD_NONLOCAL;
        if(ctx->is_compiling_class && self->scope == NAME_GLOBAL) {
            // if we are compiling a class, we should use OP_LOAD_ATTR_GLOBAL instead of OP_LOAD_GLOBAL
            // this supports @property.setter
            op = OP_LOAD_CLASS_GLOBAL;
            // exec()/eval() won't work with OP_LOAD_ATTR_GLOBAL in class body
        } else {
            // we cannot determine the scope when calling exec()/eval()
            if(self->scope == NAME_GLOBAL_UNKNOWN) op = OP_LOAD_NAME;
        }
        pk_CodeEmitContext__emit_(ctx, op, self->name, self->line);
    }
}

bool pk_NameExpr__emit_del(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_NameExpr* self = (pk_NameExpr*)self_;
    switch(self->scope) {
        case NAME_LOCAL:
            pk_CodeEmitContext__emit_(
                ctx, OP_DELETE_FAST,
                pk_CodeEmitContext__add_varname(ctx, self->name),
                self->line
            );
            break;
        case NAME_GLOBAL:
            pk_CodeEmitContext__emit_(ctx, OP_DELETE_GLOBAL, self->name, self->line);
            break;
        case NAME_GLOBAL_UNKNOWN:
            pk_CodeEmitContext__emit_(ctx, OP_DELETE_NAME, self->name, self->line);
            break;
        default: PK_UNREACHABLE();
    }
    return true;
}

bool pk_NameExpr__emit_store(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_NameExpr* self = (pk_NameExpr*)self_;
    if(ctx->is_compiling_class) {
        pk_CodeEmitContext__emit_(ctx, OP_STORE_CLASS_ATTR, self->name, self->line);
        return true;
    }
    pk_CodeEmitContext__emit_store_name(ctx, self->scope, self->name, self->line);
    return true;
}

pk_NameExpr* pk_NameExpr__new(StrName name, NameScope scope){
    static_assert_expr_size(pk_NameExpr);
    pk_NameExpr* self = PoolExpr_alloc();
    self->vt = &NameExprVt;
    self->line = -1;
    self->name = name;
    self->scope = scope;
    return self;
}

static pk_ExprVt StarredExprVt;

int pk_ExprVt__star_level(const pk_Expr* self) {
    return ((pk_StarredExpr*)self)->level;
}

void pk_StarredExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_StarredExpr* self = (pk_StarredExpr*)self_;
    self->child->vt->emit_(self->child, ctx);
    pk_CodeEmitContext__emit_(ctx, OP_UNARY_STAR, self->level, self->line);
}

bool pk_StarredExpr__emit_store(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_StarredExpr* self = (pk_StarredExpr*)self_;
    if(self->level != 1) return false;
    // simply proxy to child
    return self->child->vt->emit_store(self->child, ctx);
}

pk_StarredExpr* pk_StarredExpr__new(pk_Expr* child, int level){
    static_assert_expr_size(pk_StarredExpr);
    pk_StarredExpr* self = PoolExpr_alloc();
    self->vt = &StarredExprVt;
    self->line = -1;
    self->child = child;
    self->level = level;
    return self;
}

static pk_ExprVt UnaryExprVt;

void pk_UnaryExpr__dtor(pk_Expr* self_){
    pk_UnaryExpr* self = (pk_UnaryExpr*)self_;
    pk_Expr__delete(self->child);
}

static void pk_UnaryExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_UnaryExpr* self = (pk_UnaryExpr*)self_;
    self->child->vt->emit_(self->child, ctx);
    pk_CodeEmitContext__emit_(ctx, self->opcode, BC_NOARG, self->line);
}

pk_UnaryExpr* pk_UnaryExpr__new(pk_Expr* child, Opcode opcode){
    static_assert_expr_size(pk_UnaryExpr);
    pk_UnaryExpr* self = PoolExpr_alloc();
    self->vt = &UnaryExprVt;
    self->line = -1;
    self->child = child;
    self->opcode = opcode;
    return self;
}

static pk_ExprVt RawStringExprVt;

void pk_RawStringExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_RawStringExpr* self = (pk_RawStringExpr*)self_;
    int index = pk_CodeEmitContext__add_const_string(ctx, self->value);
    pk_CodeEmitContext__emit_(ctx, OP_LOAD_CONST, index, self->line);
    pk_CodeEmitContext__emit_(ctx, self->opcode, BC_NOARG, self->line);
}

pk_RawStringExpr* pk_RawStringExpr__new(c11_string value, Opcode opcode){
    static_assert_expr_size(pk_RawStringExpr);
    pk_RawStringExpr* self = PoolExpr_alloc();
    self->vt = &RawStringExprVt;
    self->line = -1;
    self->value = value;
    self->opcode = opcode;
    return self;
}

static pk_ExprVt ImagExprVt;

void pk_ImagExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_ImagExpr* self = (pk_ImagExpr*)self_;
    PyVar value;
    py_newfloat(&value, self->value);
    int index = pk_CodeEmitContext__add_const(ctx, &value);
    pk_CodeEmitContext__emit_(ctx, OP_LOAD_CONST, index, self->line);
    pk_CodeEmitContext__emit_(ctx, OP_BUILD_IMAG, BC_NOARG, self->line);
}

pk_ImagExpr* pk_ImagExpr__new(double value){
    static_assert_expr_size(pk_ImagExpr);
    pk_ImagExpr* self = PoolExpr_alloc();
    self->vt = &ImagExprVt;
    self->line = -1;
    self->value = value;
    return self;
}

static pk_ExprVt LiteralExprVt;

void pk_LiteralExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_LiteralExpr* self = (pk_LiteralExpr*)self_;
    switch(self->value->index){
        case TokenValue_I64: {
            int64_t val = self->value->_i64;
            pk_CodeEmitContext__emit_int(ctx, val, self->line);
            break;
        }
        case TokenValue_F64: {
            PyVar value;
            py_newfloat(&value, self->value->_f64);
            int index = pk_CodeEmitContext__add_const(ctx, &value);
            pk_CodeEmitContext__emit_(ctx, OP_LOAD_CONST, index, self->line);
            break;
        }
        case TokenValue_STR: {
            c11_string sv = py_Str__sv(&self->value->_str);
            int index = pk_CodeEmitContext__add_const_string(ctx, sv);
            pk_CodeEmitContext__emit_(ctx, OP_LOAD_CONST, index, self->line);
            break;
        }
        default: PK_UNREACHABLE();
    }
}

pk_LiteralExpr* pk_LiteralExpr__new(const TokenValue* value){
    static_assert_expr_size(pk_LiteralExpr);
    pk_LiteralExpr* self = PoolExpr_alloc();
    self->vt = &LiteralExprVt;
    self->line = -1;
    self->value = value;
    return self;
}

static pk_ExprVt SliceExprVt;

void pk_SliceExpr__dtor(pk_Expr* self_){
    pk_SliceExpr* self = (pk_SliceExpr*)self_;
    pk_Expr__delete(self->start);
    pk_Expr__delete(self->stop);
    pk_Expr__delete(self->step);
}

void pk_SliceExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_SliceExpr* self = (pk_SliceExpr*)self_;
    if(self->start) self->start->vt->emit_(self->start, ctx);
    else pk_CodeEmitContext__emit_(ctx, OP_LOAD_NONE, BC_NOARG, self->line);
    if(self->stop) self->stop->vt->emit_(self->stop, ctx);
    else pk_CodeEmitContext__emit_(ctx, OP_LOAD_NONE, BC_NOARG, self->line);
    if(self->step) self->step->vt->emit_(self->step, ctx);
    else pk_CodeEmitContext__emit_(ctx, OP_LOAD_NONE, BC_NOARG, self->line);
    pk_CodeEmitContext__emit_(ctx, OP_BUILD_SLICE, BC_NOARG, self->line);
}

pk_SliceExpr* pk_SliceExpr__new(){
    static_assert_expr_size(pk_SliceExpr);
    pk_SliceExpr* self = PoolExpr_alloc();
    self->vt = &SliceExprVt;
    self->line = -1;
    self->start = NULL;
    self->stop = NULL;
    self->step = NULL;
    return self;
}

static pk_ExprVt ListExprVt;
static pk_ExprVt DictExprVt;
static pk_ExprVt SetExprVt;
static pk_ExprVt TupleExprVt;

pk_SequenceExpr* pk_SequenceExpr__new(pk_ExprVt* vt, int count, Opcode opcode){
    static_assert_expr_size(pk_SequenceExpr);
    pk_SequenceExpr* self = PoolExpr_alloc();
    self->vt = vt;
    self->line = -1;
    self->opcode = opcode;
    c11_array__ctor(&self->items, count, sizeof(pk_Expr*));
    return self;
}

static void pk_SequenceExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_SequenceExpr* self = (pk_SequenceExpr*)self_;
    for(int i=0; i<self->items.count; i++){
        pk_Expr* item = c11__getitem(pk_Expr*, &self->items, i);
        item->vt->emit_(item, ctx);
    }
    pk_CodeEmitContext__emit_(ctx, self->opcode, self->items.count, self->line);
}

void pk_SequenceExpr__dtor(pk_Expr* self_){
    pk_SequenceExpr* self = (pk_SequenceExpr*)self_;
    c11__foreach(pk_Expr*, &self->items, e){
        pk_Expr__delete(*e);
    }
    c11_array__dtor(&self->items);
}

bool pk_TupleExpr__emit_store(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_SequenceExpr* self = (pk_SequenceExpr*)self_;
    // TOS is an iterable
    // items may contain StarredExpr, we should check it
    int starred_i = -1;
    for(int i = 0; i < self->items.count; i++) {
        pk_Expr* e = c11__getitem(pk_Expr*, &self->items, i);
        if(e->vt->star_level(e) == 0) continue;
        if(starred_i == -1) starred_i = i;
        else return false;  // multiple StarredExpr not allowed
    }

    if(starred_i == -1) {
        Bytecode* prev = c11__at(Bytecode, &ctx->co->codes, ctx->co->codes.count - 1);
        if(prev->op == OP_BUILD_TUPLE && prev->arg == self->items.count) {
            // build tuple and unpack it is meaningless
            pk_CodeEmitContext__revert_last_emit_(ctx);
        } else {
            if(prev->op == OP_FOR_ITER) {
                prev->op = OP_FOR_ITER_UNPACK;
                prev->arg = self->items.count;
            } else {
                pk_CodeEmitContext__emit_(ctx, OP_UNPACK_SEQUENCE, self->items.count, self->line);
            }
        }
    } else {
        // starred assignment target must be in a tuple
        if(self->items.count == 1) return false;
        // starred assignment target must be the last one (differ from cpython)
        if(starred_i != self->items.count - 1) return false;
        // a,*b = [1,2,3]
        // stack is [1,2,3] -> [1,[2,3]]
        pk_CodeEmitContext__emit_(ctx, OP_UNPACK_EX, self->items.count - 1, self->line);
    }
    // do reverse emit
    for(int i = self->items.count - 1; i >= 0; i--) {
        pk_Expr* e = c11__getitem(pk_Expr*, &self->items, i);
        bool ok = e->vt->emit_store(e, ctx);
        if(!ok) return false;
    }
    return true;
}

bool pk_TupleExpr__emit_del(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_SequenceExpr* self = (pk_SequenceExpr*)self_;
    c11__foreach(pk_Expr*, &self->items, e){
        bool ok = (*e)->vt->emit_del(*e, ctx);
        if(!ok) return false;
    }
    return true;
}

static pk_ExprVt CompExprVt;

void pk_CompExpr__dtor(pk_Expr* self_){
    pk_CompExpr* self = (pk_CompExpr*)self_;
    pk_Expr__delete(self->expr);
    pk_Expr__delete(self->vars);
    pk_Expr__delete(self->iter);
    pk_Expr__delete(self->cond);
}

void pk_CompExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_CompExpr* self = (pk_CompExpr*)self_;
    pk_CodeEmitContext__emit_(ctx, self->op0, 0, self->line);
    self->iter->vt->emit_(self->iter, ctx);
    pk_CodeEmitContext__emit_(ctx, OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
    pk_CodeEmitContext__enter_block(ctx, CodeBlockType_FOR_LOOP);
    int curr_iblock = ctx->curr_iblock;
    int for_codei = pk_CodeEmitContext__emit_(ctx, OP_FOR_ITER, curr_iblock, BC_KEEPLINE);
    bool ok = self->vars->vt->emit_store(self->vars, ctx);
    // this error occurs in `vars` instead of this line, but...nevermind
    assert(ok);  // this should raise a SyntaxError, but we just assert it
    pk_CodeEmitContext__try_merge_for_iter_store(ctx, for_codei);
    if(self->cond) {
        self->cond->vt->emit_(self->cond, ctx);
        int patch = pk_CodeEmitContext__emit_(ctx, OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
        self->expr->vt->emit_(self->expr, ctx);
        pk_CodeEmitContext__emit_(ctx, self->op1, BC_NOARG, BC_KEEPLINE);
        pk_CodeEmitContext__patch_jump(ctx, patch);
    } else {
        self->expr->vt->emit_(self->expr, ctx);
        pk_CodeEmitContext__emit_(ctx, self->op1, BC_NOARG, BC_KEEPLINE);
    }
    pk_CodeEmitContext__emit_(ctx, OP_LOOP_CONTINUE, curr_iblock, BC_KEEPLINE);
    pk_CodeEmitContext__exit_block(ctx);
}

pk_CompExpr* pk_CompExpr__new(Opcode op0, Opcode op1){
    static_assert_expr_size(pk_CompExpr);
    pk_CompExpr* self = PoolExpr_alloc();
    self->vt = &CompExprVt;
    self->line = -1;
    self->op0 = op0;
    self->op1 = op1;
    self->expr = NULL;
    self->vars = NULL;
    self->iter = NULL;
    self->cond = NULL;
    return self;
}

static pk_ExprVt LambdaExprVt;

pk_LambdaExpr* pk_LambdaExpr__new(int index){
    static_assert_expr_size(pk_LambdaExpr);
    pk_LambdaExpr* self = PoolExpr_alloc();
    self->vt = &LambdaExprVt;
    self->line = -1;
    self->index = index;
    return self;
}

static void pk_LambdaExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_LambdaExpr* self = (pk_LambdaExpr*)self_;
    pk_CodeEmitContext__emit_(ctx, OP_LOAD_FUNCTION, self->index, self->line);
}

static pk_ExprVt FStringExprVt;

static bool is_fmt_valid_char(char c) {
    switch(c) {
        // clang-format off
        case '-': case '=': case '*': case '#': case '@': case '!': case '~':
        case '<': case '>': case '^':
        case '.': case 'f': case 'd': case 's':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        return true;
        default: return false;
        // clang-format on
    }
}

static bool is_identifier(c11_string s) {
    if(s.size == 0) return false;
    if(!isalpha(s.data[0]) && s.data[0] != '_') return false;
    for(int i=0; i<s.size; i++){
        char c = s.data[i];
        if(!isalnum(c) && c != '_') return false;
    }
    return true;
}

static void _load_simple_expr(pk_CodeEmitContext* ctx, c11_string expr, int line) {
    bool repr = false;
    const char* expr_end = expr.data + expr.size;
    if(expr.size >= 2 && expr_end[-2] == '!') {
        switch(expr_end[-1]) {
            case 'r':
                repr = true;
                expr.size -= 2; // expr[:-2]
                break;
            case 's':
                repr = false;
                expr.size -= 2; // expr[:-2]
                break;
            default: break;  // nothing happens
        }
    }
    // name or name.name
    bool is_fastpath = false;
    if(is_identifier(expr)) {
        // ctx->emit_(OP_LOAD_NAME, StrName(expr.sv()).index, line);
        pk_CodeEmitContext__emit_(
            ctx,
            OP_LOAD_NAME,
            pk_StrName__map2(expr),
            line
        );
        is_fastpath = true;
    } else {
        int dot = c11_string__index(expr, '.');
        if(dot > 0) {
            // std::string_view a = expr.sv().substr(0, dot);
            // std::string_view b = expr.sv().substr(dot + 1);
            c11_string a = {expr.data, dot};    // expr[:dot]
            c11_string b = {expr.data+(dot+1), expr.size-(dot+1)};  // expr[dot+1:]
            if(is_identifier(a) && is_identifier(b)) {
                pk_CodeEmitContext__emit_(ctx, OP_LOAD_NAME, pk_StrName__map2(a), line);
                pk_CodeEmitContext__emit_(ctx, OP_LOAD_ATTR, pk_StrName__map2(b), line);
                is_fastpath = true;
            }
        }
    }

    if(!is_fastpath) {
        int index = pk_CodeEmitContext__add_const_string(ctx, expr);
        pk_CodeEmitContext__emit_(ctx, OP_FSTRING_EVAL, index, line);
    }

    if(repr) {
        pk_CodeEmitContext__emit_(ctx, OP_REPR, BC_NOARG, line);
    }
}

static void pk_FStringExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_FStringExpr* self = (pk_FStringExpr*)self_;
    int i = 0;          // left index
    int j = 0;          // right index
    int count = 0;      // how many string parts
    bool flag = false;  // true if we are in a expression

    const char* src = self->src.data;
    while(j < self->src.size) {
        if(flag) {
            if(src[j] == '}') {
                // add expression
                c11_string expr = {src+i, j-i}; // src[i:j]
                // BUG: ':' is not a format specifier in f"{stack[2:]}"
                int conon = c11_string__index(expr, ':');
                if(conon >= 0) {
                    c11_string spec = {expr.data+(conon+1), expr.size-(conon+1)};  // expr[conon+1:]
                    // filter some invalid spec
                    bool ok = true;
                    for(int k = 0; k < spec.size; k++) {
                        char c = spec.data[k];
                        if(!is_fmt_valid_char(c)) {
                            ok = false;
                            break;
                        }
                    }
                    if(ok) {
                        expr.size = conon;  // expr[:conon]
                        _load_simple_expr(ctx, expr, self->line);
                        // ctx->emit_(OP_FORMAT_STRING, ctx->add_const_string(spec.sv()), line);
                        pk_CodeEmitContext__emit_(ctx, OP_FORMAT_STRING, pk_CodeEmitContext__add_const_string(ctx, spec), self->line);
                    } else {
                        // ':' is not a spec indicator
                        _load_simple_expr(ctx, expr, self->line);
                    }
                } else {
                    _load_simple_expr(ctx, expr, self->line);
                }
                flag = false;
                count++;
            }
        } else {
            if(src[j] == '{') {
                // look at next char
                if(j + 1 < self->src.size && src[j + 1] == '{') {
                    // {{ -> {
                    j++;
                    pk_CodeEmitContext__emit_(
                        ctx,
                        OP_LOAD_CONST,
                        pk_CodeEmitContext__add_const_string(ctx, (c11_string){"{", 1}),
                        self->line
                    );
                    count++;
                } else {
                    // { -> }
                    flag = true;
                    i = j + 1;
                }
            } else if(src[j] == '}') {
                // look at next char
                if(j + 1 < self->src.size && src[j + 1] == '}') {
                    // }} -> }
                    j++;
                    pk_CodeEmitContext__emit_(
                        ctx,
                        OP_LOAD_CONST,
                        pk_CodeEmitContext__add_const_string(ctx, (c11_string){"}", 1}),
                        self->line
                    );
                    count++;
                } else {
                    // } -> error
                    // throw std::runtime_error("f-string: unexpected }");
                    // just ignore
                }
            } else {
                // literal
                i = j;
                while(j < self->src.size && src[j] != '{' && src[j] != '}')
                    j++;
                c11_string literal = {src+i, j-i};  // src[i:j]
                pk_CodeEmitContext__emit_(
                    ctx,
                    OP_LOAD_CONST,
                    pk_CodeEmitContext__add_const_string(ctx, literal),
                    self->line
                );
                count++;
                continue;  // skip j++
            }
        }
        j++;
    }

    if(flag) {
        // literal
        c11_string literal = {src+i, self->src.size-i}; // src[i:]
        pk_CodeEmitContext__emit_(ctx, OP_LOAD_CONST, pk_CodeEmitContext__add_const_string(ctx, literal), self->line);
        count++;
    }
    pk_CodeEmitContext__emit_(ctx, OP_BUILD_STRING, count, self->line);
}

pk_FStringExpr* pk_FStringExpr__new(c11_string src){
    static_assert_expr_size(pk_FStringExpr);
    pk_FStringExpr* self = PoolExpr_alloc();
    self->vt = &FStringExprVt;
    self->line = -1;
    self->src = src;
    return self;
}

static pk_ExprVt LogicBinaryExpr;

pk_LogicBinaryExpr* pk_LogicBinaryExpr__new(pk_Expr* lhs, pk_Expr* rhs, Opcode opcode){
    static_assert_expr_size(pk_LogicBinaryExpr);
    pk_LogicBinaryExpr* self = PoolExpr_alloc();
    self->vt = &LogicBinaryExpr;
    self->line = -1;
    self->lhs = lhs;
    self->rhs = rhs;
    self->opcode = opcode;
    return self;
}

void pk_LogicBinaryExpr__dtor(pk_Expr* self_){
    pk_LogicBinaryExpr* self = (pk_LogicBinaryExpr*)self_;
    pk_Expr__delete(self->lhs);
    pk_Expr__delete(self->rhs);
}

void pk_LogicBinaryExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_LogicBinaryExpr* self = (pk_LogicBinaryExpr*)self_;
    self->lhs->vt->emit_(self->lhs, ctx);
    int patch = pk_CodeEmitContext__emit_(ctx, self->opcode, BC_NOARG, self->line);
    self->rhs->vt->emit_(self->rhs, ctx);
    pk_CodeEmitContext__patch_jump(ctx, patch);
}

static pk_ExprVt GroupedExprVt;

void pk_GroupedExpr__dtor(pk_Expr* self_){
    pk_GroupedExpr* self = (pk_GroupedExpr*)self_;
    pk_Expr__delete(self->child);
}

void pk_GroupedExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_GroupedExpr* self = (pk_GroupedExpr*)self_;
    self->child->vt->emit_(self->child, ctx);
}

bool pk_GroupedExpr__emit_del(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_GroupedExpr* self = (pk_GroupedExpr*)self_;
    return self->child->vt->emit_del(self->child, ctx);
}

bool pk_GroupedExpr__emit_store(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_GroupedExpr* self = (pk_GroupedExpr*)self_;
    return self->child->vt->emit_store(self->child, ctx);
}

pk_GroupedExpr* pk_GroupedExpr__new(pk_Expr* child){
    static_assert_expr_size(pk_GroupedExpr);
    pk_GroupedExpr* self = PoolExpr_alloc();
    self->vt = &GroupedExprVt;
    self->line = -1;
    self->child = child;
    return self;
}

static pk_ExprVt BinaryExprVt;

static void pk_BinaryExpr__dtor(pk_Expr* self_){
    pk_BinaryExpr* self = (pk_BinaryExpr*)self_;
    pk_Expr__delete(self->lhs);
    pk_Expr__delete(self->rhs);
}

static pk_BinaryExpr__is_compare(pk_Expr* self_){
    pk_BinaryExpr* self = (pk_BinaryExpr*)self_;
    switch(self->op) {
        case TK_LT:
        case TK_LE:
        case TK_EQ:
        case TK_NE:
        case TK_GT:
        case TK_GE: return true;
        default: return false;
    }
}

static void _emit_compare(pk_BinaryExpr* self, pk_CodeEmitContext* ctx, c11_vector* jmps) {
    if(self->lhs->vt->is_compare(self->lhs)) {
        pk_BinaryExpr* lhs = (pk_BinaryExpr*)lhs;
        _emit_compare(lhs, ctx, jmps);
    } else {
        self->lhs->vt->emit_(self->lhs, ctx);  // [a]
    }
    self->rhs->vt->emit_(self->rhs, ctx);                                   // [a, b]
    pk_CodeEmitContext__emit_(ctx, OP_DUP_TOP, BC_NOARG, self->line);       // [a, b, b]
    pk_CodeEmitContext__emit_(ctx, OP_ROT_THREE, BC_NOARG, self->line);     // [b, a, b]
    Opcode opcode;
    switch(self->op) {
        case TK_LT: opcode = OP_COMPARE_LT; break;
        case TK_LE: opcode = OP_COMPARE_LE; break;
        case TK_EQ: opcode = OP_COMPARE_EQ; break;
        case TK_NE: opcode = OP_COMPARE_NE; break;
        case TK_GT: opcode = OP_COMPARE_GT; break;
        case TK_GE: opcode = OP_COMPARE_GE; break;
        default: PK_UNREACHABLE()
    }
    pk_CodeEmitContext__emit_(ctx, opcode, BC_NOARG, self->line);
    // [b, RES]
    int index = pk_CodeEmitContext__emit_(ctx, OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, self->line);
    c11_vector__push(int, jmps, index);
}

static void pk_BinaryExpr__emit_(pk_Expr* self_, pk_CodeEmitContext* ctx) {
    pk_BinaryExpr* self = (pk_BinaryExpr*)self_;
    c11_vector/*T=int*/ jmps;
    c11_vector__ctor(&jmps, sizeof(int));
    if(self->vt->is_compare(self_) && self->lhs->vt->is_compare(self->lhs)) {
        // (a < b) < c
        pk_BinaryExpr* e = (pk_BinaryExpr*)self->lhs;
        _emit_compare(e, ctx, &jmps);
        // [b, RES]
    } else {
        // (1 + 2) < c
        if(self->inplace) {
            self->lhs->vt->emit_inplace(self->lhs, ctx);
        } else {
            self->lhs->vt->emit_(self->lhs, ctx);
        }
    }

    self->rhs->vt->emit_(self->rhs, ctx);
    Opcode opcode;
    switch(self->op) {
        case TK_ADD: opcode = OP_BINARY_ADD; break;
        case TK_SUB: opcode = OP_BINARY_SUB; break;
        case TK_MUL: opcode = OP_BINARY_MUL; break;
        case TK_DIV: opcode = OP_BINARY_TRUEDIV; break;
        case TK_FLOORDIV: opcode = OP_BINARY_FLOORDIV; break;
        case TK_MOD: opcode = OP_BINARY_MOD; break;
        case TK_POW: opcode = OP_BINARY_POW; break;

        case TK_LT: opcode = OP_COMPARE_LT; break;
        case TK_LE: opcode = OP_COMPARE_LE; break;
        case TK_EQ: opcode = OP_COMPARE_EQ; break;
        case TK_NE: opcode = OP_COMPARE_NE; break;
        case TK_GT: opcode = OP_COMPARE_GT; break;
        case TK_GE: opcode = OP_COMPARE_GE; break;

        // case TK_IN: ctx->emit_(OP_CONTAINS_OP, 0, line); break;
        // case TK_NOT_IN: ctx->emit_(OP_CONTAINS_OP, 1, line); break;
        // case TK_IS: ctx->emit_(OP_IS_OP, BC_NOARG, line); break;
        // case TK_IS_NOT: ctx->emit_(OP_IS_NOT_OP, BC_NOARG, line); break;

        case TK_LSHIFT: ctx->emit_(OP_BITWISE_LSHIFT, BC_NOARG, line); break;
        case TK_RSHIFT: ctx->emit_(OP_BITWISE_RSHIFT, BC_NOARG, line); break;
        case TK_AND: ctx->emit_(OP_BITWISE_AND, BC_NOARG, line); break;
        case TK_OR: ctx->emit_(OP_BITWISE_OR, BC_NOARG, line); break;
        case TK_XOR: ctx->emit_(OP_BITWISE_XOR, BC_NOARG, line); break;

        case TK_DECORATOR: ctx->emit_(OP_BINARY_MATMUL, BC_NOARG, line); break;
        default: PK_FATAL_ERROR("unknown binary operator: %s\n", pk_TokenSymbols[op]);
    }

    for(int i: jmps)
        ctx->patch_jump(i);
}

static pk_ExprVt TernaryExprVt;

/////////////////////////////////////////////
void pk_Expr__initialize(){
    pk_ExprVt__ctor(&NameExprVt);
    pk_ExprVt* vt = &NameExprVt;
    vt->emit_ = pk_NameExpr__emit_;
    vt->emit_del = pk_NameExpr__emit_del;
    vt->emit_store = pk_NameExpr__emit_store;

    pk_ExprVt__ctor(&StarredExprVt);
    vt = &StarredExprVt;
    vt->dtor = pk_UnaryExpr__dtor;
    vt->star_level = pk_ExprVt__star_level;
    vt->emit_ = pk_StarredExpr__emit_;
    vt->emit_store = pk_StarredExpr__emit_store;

    pk_ExprVt__ctor(&UnaryExprVt);
    vt = &UnaryExprVt;
    vt->dtor = pk_UnaryExpr__dtor;
    vt->emit_ = pk_UnaryExpr__emit_;

    pk_ExprVt__ctor(&RawStringExprVt);
    vt = &RawStringExprVt;
    vt->emit_ = pk_RawStringExpr__emit_;

    pk_ExprVt__ctor(&ImagExprVt);
    vt = &ImagExprVt;
    vt->emit_ = pk_ImagExpr__emit_;

    pk_ExprVt__ctor(&LiteralExprVt);
    vt = &LiteralExprVt;
    vt->emit_ = pk_LiteralExpr__emit_;
    vt->is_literal = true;
    vt->is_json_object = true;

    pk_ExprVt__ctor(&SliceExprVt);
    vt = &SliceExprVt;
    vt->dtor = pk_SliceExpr__dtor;
    vt->emit_ = pk_SliceExpr__emit_;

    pk_ExprVt* seqVt[] = {&ListExprVt, &DictExprVt, &SetExprVt, &TupleExprVt};
    for(int i=0; i<4; i++){
        pk_ExprVt__ctor(seqVt[i]);
        vt = seqVt[i];
        vt->dtor = pk_SequenceExpr__dtor;
        vt->emit_ = pk_SequenceExpr__emit_;
    }

    ListExprVt.is_json_object = true;
    DictExprVt.is_json_object = true;

    TupleExprVt.is_tuple = true;
    TupleExprVt.emit_store = pk_TupleExpr__emit_store;
    TupleExprVt.emit_del = pk_TupleExpr__emit_del;

    pk_ExprVt__ctor(&CompExprVt);
    vt = &CompExprVt;
    vt->dtor = pk_CompExpr__dtor;
    vt->emit_ = pk_CompExpr__emit_;

    pk_ExprVt__ctor(&LambdaExprVt);
    vt = &LambdaExprVt;
    vt->emit_ = pk_LambdaExpr__emit_;

    pk_ExprVt__ctor(&FStringExprVt);
    vt = &FStringExprVt;
    vt->emit_ = pk_FStringExpr__emit_;

    pk_ExprVt__ctor(&LogicBinaryExpr);
    vt = &LogicBinaryExpr;
    vt->dtor = pk_LogicBinaryExpr__dtor;
    vt->emit_ = pk_LogicBinaryExpr__emit_;

    pk_ExprVt__ctor(&GroupedExprVt);
    vt = &GroupedExprVt;
    vt->dtor = pk_GroupedExpr__dtor;
    vt->emit_ = pk_GroupedExpr__emit_;
    vt->emit_del = pk_GroupedExpr__emit_del;
    vt->emit_store = pk_GroupedExpr__emit_store;
}
