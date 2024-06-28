#include "pocketpy/compiler/expr.h"
#include "pocketpy/compiler/context.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/strname.h"

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
}
