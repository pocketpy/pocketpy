#include "pocketpy/compiler/compiler.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/common/name.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/common/sstream.h"
#include <assert.h>
#include <stdbool.h>

/* expr.h */
typedef struct Expr Expr;
typedef struct Ctx Ctx;

typedef struct ExprVt {
    /* emit */
    void (*emit_)(Expr*, Ctx*);
    bool (*emit_del)(Expr*, Ctx*);
    bool (*emit_store)(Expr*, Ctx*);
    void (*emit_inplace)(Expr*, Ctx*);
    bool (*emit_istore)(Expr*, Ctx*);
    /* reflections */
    bool is_literal;
    bool is_name;     // NameExpr
    bool is_tuple;    // TupleExpr
    bool is_attrib;   // AttribExpr
    bool is_subscr;   // SubscrExpr
    bool is_starred;  // StarredExpr
    bool is_binary;   // BinaryExpr
    bool is_ternary;  // TernaryExpr
    void (*dtor)(Expr*);
} ExprVt;

#define vtcall(f, self, ctx) ((self)->vt->f((self), (ctx)))
#define vtemit_(self, ctx) vtcall(emit_, (self), (ctx))
#define vtemit_del(self, ctx) ((self)->vt->emit_del ? vtcall(emit_del, self, ctx) : false)
#define vtemit_store(self, ctx) ((self)->vt->emit_store ? vtcall(emit_store, self, ctx) : false)
#define vtemit_inplace(self, ctx)                                                                  \
    ((self)->vt->emit_inplace ? vtcall(emit_inplace, self, ctx) : vtemit_(self, ctx))
#define vtemit_istore(self, ctx)                                                                   \
    ((self)->vt->emit_istore ? vtcall(emit_istore, self, ctx) : vtemit_store(self, ctx))
#define vtdelete(self)                                                                             \
    do {                                                                                           \
        if(self) {                                                                                 \
            if((self)->vt->dtor) (self)->vt->dtor(self);                                           \
            PK_FREE(self);                                                                         \
        }                                                                                          \
    } while(0)

#define EXPR_COMMON_HEADER                                                                         \
    const ExprVt* vt;                                                                              \
    int line;

typedef struct Expr {
    EXPR_COMMON_HEADER
} Expr;

/* context.h */
typedef struct Ctx {
    CodeObject* co;  // 1 CodeEmitContext <=> 1 CodeObject*
    FuncDecl* func;  // optional, weakref
    int level;
    int curr_iblock;
    bool is_compiling_class;
    c11_vector /*T=Expr_p*/ s_expr;
    c11_smallmap_n2d global_names;
    c11_smallmap_v2d co_consts_string_dedup_map;  // this stores 0-based index instead of pointer
} Ctx;

typedef struct Expr Expr;

static void Ctx__ctor(Ctx* self, CodeObject* co, FuncDecl* func, int level);
static void Ctx__dtor(Ctx* self);
static int Ctx__prepare_loop_divert(Ctx* self, int line, bool is_break);
static int Ctx__enter_block(Ctx* self, CodeBlockType type);
static void Ctx__exit_block(Ctx* self);
static int Ctx__emit_(Ctx* self, Opcode opcode, uint16_t arg, int line);
// static void Ctx__revert_last_emit_(Ctx* self);
static int Ctx__emit_int(Ctx* self, int64_t value, int line);
static void Ctx__patch_jump(Ctx* self, int index);
static void Ctx__emit_jump(Ctx* self, int target, int line);
static int Ctx__add_varname(Ctx* self, py_Name name);
static int Ctx__add_name(Ctx* self, py_Name name);
static int Ctx__add_const(Ctx* self, py_Ref);
static int Ctx__add_const_string(Ctx* self, c11_sv);
static void Ctx__emit_store_name(Ctx* self, NameScope scope, py_Name name, int line);
static void Ctx__s_emit_top(Ctx*);     // emit top -> pop -> delete
static void Ctx__s_push(Ctx*, Expr*);  // push
static Expr* Ctx__s_top(Ctx*);         // top
static int Ctx__s_size(Ctx*);          // size
static void Ctx__s_pop(Ctx*);          // pop -> delete
static Expr* Ctx__s_popx(Ctx*);        // pop move
static void Ctx__s_emit_decorators(Ctx*, int count);

/* expr.c */
typedef struct NameExpr {
    EXPR_COMMON_HEADER
    py_Name name;
    NameScope scope;
} NameExpr;

void NameExpr__emit_(Expr* self_, Ctx* ctx) {
    NameExpr* self = (NameExpr*)self_;
    int index = c11_smallmap_n2d__get(&ctx->co->varnames_inv, self->name, -1);
    if(self->scope == NAME_LOCAL && index >= 0) {
        // we know this is a local variable
        Ctx__emit_(ctx, OP_LOAD_FAST, index, self->line);
    } else {
        Opcode op = ctx->level <= 1 ? OP_LOAD_GLOBAL : OP_LOAD_NONLOCAL;
        if(self->scope == NAME_GLOBAL) {
            if(ctx->co->src->is_dynamic) {
                op = OP_LOAD_NAME;
            } else {
                if(ctx->is_compiling_class) {
                    // if we are compiling a class, we should use `OP_LOAD_CLASS_GLOBAL`
                    // this is for @property.setter
                    op = OP_LOAD_CLASS_GLOBAL;
                }
            }
        }
        Ctx__emit_(ctx, op, Ctx__add_name(ctx, self->name), self->line);
    }
}

bool NameExpr__emit_del(Expr* self_, Ctx* ctx) {
    NameExpr* self = (NameExpr*)self_;
    switch(self->scope) {
        case NAME_LOCAL:
            Ctx__emit_(ctx, OP_DELETE_FAST, Ctx__add_varname(ctx, self->name), self->line);
            break;
        case NAME_GLOBAL: {
            Opcode op = ctx->co->src->is_dynamic ? OP_DELETE_NAME : OP_DELETE_GLOBAL;
            Ctx__emit_(ctx, op, Ctx__add_name(ctx, self->name), self->line);
            break;
        }
        default: c11__unreachable();
    }
    return true;
}

bool NameExpr__emit_store(Expr* self_, Ctx* ctx) {
    NameExpr* self = (NameExpr*)self_;
    if(ctx->is_compiling_class) {
        Ctx__emit_(ctx, OP_STORE_CLASS_ATTR, Ctx__add_name(ctx, self->name), self->line);
        return true;
    }
    Ctx__emit_store_name(ctx, self->scope, self->name, self->line);
    return true;
}

NameExpr* NameExpr__new(int line, py_Name name, NameScope scope) {
    const static ExprVt Vt = {.emit_ = NameExpr__emit_,
                              .emit_del = NameExpr__emit_del,
                              .emit_store = NameExpr__emit_store,
                              .is_name = true};
    NameExpr* self = PK_MALLOC(sizeof(NameExpr));
    self->vt = &Vt;
    self->line = line;
    self->name = name;
    self->scope = scope;
    return self;
}

typedef struct StarredExpr {
    EXPR_COMMON_HEADER
    Expr* child;
    int level;
} StarredExpr;

void StarredExpr__emit_(Expr* self_, Ctx* ctx) {
    StarredExpr* self = (StarredExpr*)self_;
    vtemit_(self->child, ctx);
    Ctx__emit_(ctx, OP_UNARY_STAR, self->level, self->line);
}

bool StarredExpr__emit_store(Expr* self_, Ctx* ctx) {
    StarredExpr* self = (StarredExpr*)self_;
    if(self->level != 1) return false;
    // simply proxy to child
    return vtemit_store(self->child, ctx);
}

void StarredExpr__dtor(Expr* self_) {
    StarredExpr* self = (StarredExpr*)self_;
    vtdelete(self->child);
}

StarredExpr* StarredExpr__new(int line, Expr* child, int level) {
    const static ExprVt Vt = {.emit_ = StarredExpr__emit_,
                              .emit_store = StarredExpr__emit_store,
                              .is_starred = true,
                              .dtor = StarredExpr__dtor};
    StarredExpr* self = PK_MALLOC(sizeof(StarredExpr));
    self->vt = &Vt;
    self->line = line;
    self->child = child;
    self->level = level;
    return self;
}

// InvertExpr, NotExpr, NegatedExpr
// NOTE: NegatedExpr always contains a non-const child. Should not generate -1 or -0.1
typedef struct UnaryExpr {
    EXPR_COMMON_HEADER
    Expr* child;
    Opcode opcode;
} UnaryExpr;

void UnaryExpr__dtor(Expr* self_) {
    UnaryExpr* self = (UnaryExpr*)self_;
    vtdelete(self->child);
}

static void UnaryExpr__emit_(Expr* self_, Ctx* ctx) {
    UnaryExpr* self = (UnaryExpr*)self_;
    vtemit_(self->child, ctx);
    Ctx__emit_(ctx, self->opcode, BC_NOARG, self->line);
}

UnaryExpr* UnaryExpr__new(int line, Expr* child, Opcode opcode) {
    const static ExprVt Vt = {.emit_ = UnaryExpr__emit_, .dtor = UnaryExpr__dtor};
    UnaryExpr* self = PK_MALLOC(sizeof(UnaryExpr));
    self->vt = &Vt;
    self->line = line;
    self->child = child;
    self->opcode = opcode;
    return self;
}

typedef struct FStringSpecExpr {
    EXPR_COMMON_HEADER
    Expr* child;
    c11_sv spec;
} FStringSpecExpr;

void FStringSpecExpr__emit_(Expr* self_, Ctx* ctx) {
    FStringSpecExpr* self = (FStringSpecExpr*)self_;
    vtemit_(self->child, ctx);
    int index = Ctx__add_const_string(ctx, self->spec);
    Ctx__emit_(ctx, OP_FORMAT_STRING, index, self->line);
}

FStringSpecExpr* FStringSpecExpr__new(int line, Expr* child, c11_sv spec) {
    const static ExprVt Vt = {.emit_ = FStringSpecExpr__emit_, .dtor = UnaryExpr__dtor};
    FStringSpecExpr* self = PK_MALLOC(sizeof(FStringSpecExpr));
    self->vt = &Vt;
    self->line = line;
    self->child = child;
    self->spec = spec;
    return self;
}

typedef struct RawStringExpr {
    EXPR_COMMON_HEADER
    c11_sv value;
    Opcode opcode;
} RawStringExpr;

void RawStringExpr__emit_(Expr* self_, Ctx* ctx) {
    RawStringExpr* self = (RawStringExpr*)self_;
    int index = Ctx__add_const_string(ctx, self->value);
    Ctx__emit_(ctx, self->opcode, index, self->line);
}

RawStringExpr* RawStringExpr__new(int line, c11_sv value, Opcode opcode) {
    const static ExprVt Vt = {.emit_ = RawStringExpr__emit_};
    RawStringExpr* self = PK_MALLOC(sizeof(RawStringExpr));
    self->vt = &Vt;
    self->line = line;
    self->value = value;
    self->opcode = opcode;
    return self;
}

typedef struct ImagExpr {
    EXPR_COMMON_HEADER
    double value;
} ImagExpr;

void ImagExpr__emit_(Expr* self_, Ctx* ctx) {
    ImagExpr* self = (ImagExpr*)self_;
    py_TValue value;
    py_newfloat(&value, self->value);
    int index = Ctx__add_const(ctx, &value);
    Ctx__emit_(ctx, OP_LOAD_CONST, index, self->line);
    Ctx__emit_(ctx, OP_BUILD_IMAG, BC_NOARG, self->line);
}

ImagExpr* ImagExpr__new(int line, double value) {
    const static ExprVt Vt = {.emit_ = ImagExpr__emit_};
    ImagExpr* self = PK_MALLOC(sizeof(ImagExpr));
    self->vt = &Vt;
    self->line = line;
    self->value = value;
    return self;
}

typedef struct LiteralExpr {
    EXPR_COMMON_HEADER
    const TokenValue* value;
    bool negated;
} LiteralExpr;

void LiteralExpr__emit_(Expr* self_, Ctx* ctx) {
    LiteralExpr* self = (LiteralExpr*)self_;
    switch(self->value->index) {
        case TokenValue_I64: {
            py_i64 val = self->value->_i64;
            if(self->negated) val = -val;
            Ctx__emit_int(ctx, val, self->line);
            break;
        }
        case TokenValue_F64: {
            py_TValue value;
            py_f64 val = self->value->_f64;
            if(self->negated) val = -val;
            py_newfloat(&value, val);
            int index = Ctx__add_const(ctx, &value);
            Ctx__emit_(ctx, OP_LOAD_CONST, index, self->line);
            break;
        }
        case TokenValue_STR: {
            assert(!self->negated);
            c11_sv sv = c11_string__sv(self->value->_str);
            int index = Ctx__add_const_string(ctx, sv);
            Ctx__emit_(ctx, OP_LOAD_CONST, index, self->line);
            break;
        }
        default: c11__unreachable();
    }
}

LiteralExpr* LiteralExpr__new(int line, const TokenValue* value) {
    const static ExprVt Vt = {.emit_ = LiteralExpr__emit_, .is_literal = true};
    LiteralExpr* self = PK_MALLOC(sizeof(LiteralExpr));
    self->vt = &Vt;
    self->line = line;
    self->value = value;
    self->negated = false;
    return self;
}

typedef struct Literal0Expr {
    EXPR_COMMON_HEADER
    TokenIndex token;
} Literal0Expr;

void Literal0Expr__emit_(Expr* self_, Ctx* ctx) {
    Literal0Expr* self = (Literal0Expr*)self_;
    Opcode opcode;
    switch(self->token) {
        case TK_NONE: opcode = OP_LOAD_NONE; break;
        case TK_TRUE: opcode = OP_LOAD_TRUE; break;
        case TK_FALSE: opcode = OP_LOAD_FALSE; break;
        case TK_DOTDOTDOT: opcode = OP_LOAD_ELLIPSIS; break;
        default: c11__unreachable();
    }
    Ctx__emit_(ctx, opcode, BC_NOARG, self->line);
}

Literal0Expr* Literal0Expr__new(int line, TokenIndex token) {
    const static ExprVt Vt = {.emit_ = Literal0Expr__emit_};
    Literal0Expr* self = PK_MALLOC(sizeof(Literal0Expr));
    self->vt = &Vt;
    self->line = line;
    self->token = token;
    return self;
}

typedef struct LoadConstExpr {
    EXPR_COMMON_HEADER
    int index;
} LoadConstExpr;

void LoadConstExpr__emit_(Expr* self_, Ctx* ctx) {
    LoadConstExpr* self = (LoadConstExpr*)self_;
    Ctx__emit_(ctx, OP_LOAD_CONST, self->index, self->line);
}

LoadConstExpr* LoadConstExpr__new(int line, int index) {
    const static ExprVt Vt = {.emit_ = LoadConstExpr__emit_};
    LoadConstExpr* self = PK_MALLOC(sizeof(LoadConstExpr));
    self->vt = &Vt;
    self->line = line;
    self->index = index;
    return self;
}

typedef struct SliceExpr {
    EXPR_COMMON_HEADER
    Expr* start;
    Expr* stop;
    Expr* step;
} SliceExpr;

void SliceExpr__dtor(Expr* self_) {
    SliceExpr* self = (SliceExpr*)self_;
    vtdelete(self->start);
    vtdelete(self->stop);
    vtdelete(self->step);
}

void SliceExpr__emit_(Expr* self_, Ctx* ctx) {
    SliceExpr* self = (SliceExpr*)self_;
    if(self->start)
        vtemit_(self->start, ctx);
    else
        Ctx__emit_(ctx, OP_LOAD_NONE, BC_NOARG, self->line);
    if(self->stop)
        vtemit_(self->stop, ctx);
    else
        Ctx__emit_(ctx, OP_LOAD_NONE, BC_NOARG, self->line);
    if(self->step)
        vtemit_(self->step, ctx);
    else
        Ctx__emit_(ctx, OP_LOAD_NONE, BC_NOARG, self->line);
    Ctx__emit_(ctx, OP_BUILD_SLICE, BC_NOARG, self->line);
}

SliceExpr* SliceExpr__new(int line) {
    const static ExprVt Vt = {.dtor = SliceExpr__dtor, .emit_ = SliceExpr__emit_};
    SliceExpr* self = PK_MALLOC(sizeof(SliceExpr));
    self->vt = &Vt;
    self->line = line;
    self->start = NULL;
    self->stop = NULL;
    self->step = NULL;
    return self;
}

typedef struct DictItemExpr {
    EXPR_COMMON_HEADER
    Expr* key;
    Expr* value;
} DictItemExpr;

static void DictItemExpr__dtor(Expr* self_) {
    DictItemExpr* self = (DictItemExpr*)self_;
    vtdelete(self->key);
    vtdelete(self->value);
}

static void DictItemExpr__emit_(Expr* self_, Ctx* ctx) {
    DictItemExpr* self = (DictItemExpr*)self_;
    vtemit_(self->key, ctx);
    vtemit_(self->value, ctx);
}

static DictItemExpr* DictItemExpr__new(int line) {
    const static ExprVt Vt = {.dtor = DictItemExpr__dtor, .emit_ = DictItemExpr__emit_};
    DictItemExpr* self = PK_MALLOC(sizeof(DictItemExpr));
    self->vt = &Vt;
    self->line = line;
    self->key = NULL;
    self->value = NULL;
    return self;
}

// ListExpr, DictExpr, SetExpr, TupleExpr
typedef struct SequenceExpr {
    EXPR_COMMON_HEADER
    Expr** items;
    int itemCount;
    Opcode opcode;
} SequenceExpr;

static void SequenceExpr__emit_(Expr* self_, Ctx* ctx) {
    SequenceExpr* self = (SequenceExpr*)self_;
    for(int i = 0; i < self->itemCount; i++) {
        Expr* item = self->items[i];
        vtemit_(item, ctx);
    }
    Ctx__emit_(ctx, self->opcode, self->itemCount, self->line);
}

void SequenceExpr__dtor(Expr* self_) {
    SequenceExpr* self = (SequenceExpr*)self_;
    for(int i = 0; i < self->itemCount; i++) {
        vtdelete(self->items[i]);
    }
    PK_FREE(self->items);
}

bool TupleExpr__emit_store(Expr* self_, Ctx* ctx) {
    SequenceExpr* self = (SequenceExpr*)self_;
    // TOS is an iterable
    // items may contain StarredExpr, we should check it
    int starred_i = -1;
    for(int i = 0; i < self->itemCount; i++) {
        Expr* e = self->items[i];
        if(e->vt->is_starred) {
            if(((StarredExpr*)e)->level > 0) {
                if(starred_i == -1)
                    starred_i = i;
                else
                    return false;  // multiple StarredExpr not allowed
            }
        }
    }

    if(starred_i == -1) {
        Ctx__emit_(ctx, OP_UNPACK_SEQUENCE, self->itemCount, self->line);
    } else {
        // starred assignment target must be in a tuple
        if(self->itemCount == 1) return false;
        // starred assignment target must be the last one (differ from cpython)
        if(starred_i != self->itemCount - 1) return false;
        // a,*b = [1,2,3]
        // stack is [1,2,3] -> [1,[2,3]]
        Ctx__emit_(ctx, OP_UNPACK_EX, self->itemCount - 1, self->line);
    }
    // do reverse emit
    for(int i = self->itemCount - 1; i >= 0; i--) {
        Expr* e = self->items[i];
        bool ok = vtemit_store(e, ctx);
        if(!ok) return false;
    }
    return true;
}

bool TupleExpr__emit_del(Expr* self_, Ctx* ctx) {
    SequenceExpr* self = (SequenceExpr*)self_;
    for(int i = 0; i < self->itemCount; i++) {
        Expr* e = self->items[i];
        bool ok = vtemit_del(e, ctx);
        if(!ok) return false;
    }
    return true;
}

static SequenceExpr* SequenceExpr__new(int line, const ExprVt* vt, int count, Opcode opcode) {
    SequenceExpr* self = PK_MALLOC(sizeof(SequenceExpr));
    self->vt = vt;
    self->line = line;
    self->opcode = opcode;
    self->items = PK_MALLOC(sizeof(Expr*) * count);
    self->itemCount = count;
    return self;
}

SequenceExpr* FStringExpr__new(int line, int count) {
    const static ExprVt ListExprVt = {.dtor = SequenceExpr__dtor, .emit_ = SequenceExpr__emit_};
    return SequenceExpr__new(line, &ListExprVt, count, OP_BUILD_STRING);
}

SequenceExpr* ListExpr__new(int line, int count) {
    const static ExprVt ListExprVt = {.dtor = SequenceExpr__dtor, .emit_ = SequenceExpr__emit_};
    return SequenceExpr__new(line, &ListExprVt, count, OP_BUILD_LIST);
}

SequenceExpr* DictExpr__new(int line, int count) {
    const static ExprVt DictExprVt = {.dtor = SequenceExpr__dtor, .emit_ = SequenceExpr__emit_};
    return SequenceExpr__new(line, &DictExprVt, count, OP_BUILD_DICT);
}

SequenceExpr* SetExpr__new(int line, int count) {
    const static ExprVt SetExprVt = {
        .dtor = SequenceExpr__dtor,
        .emit_ = SequenceExpr__emit_,
    };
    return SequenceExpr__new(line, &SetExprVt, count, OP_BUILD_SET);
}

SequenceExpr* TupleExpr__new(int line, int count) {
    const static ExprVt TupleExprVt = {.dtor = SequenceExpr__dtor,
                                       .emit_ = SequenceExpr__emit_,
                                       .is_tuple = true,
                                       .emit_store = TupleExpr__emit_store,
                                       .emit_del = TupleExpr__emit_del};
    return SequenceExpr__new(line, &TupleExprVt, count, OP_BUILD_TUPLE);
}

typedef struct CompExpr {
    EXPR_COMMON_HEADER
    Expr* expr;  // loop expr
    Expr* vars;  // loop vars
    Expr* iter;  // loop iter
    Expr* cond;  // optional if condition

    Opcode op0;
    Opcode op1;
} CompExpr;

void CompExpr__dtor(Expr* self_) {
    CompExpr* self = (CompExpr*)self_;
    vtdelete(self->expr);
    vtdelete(self->vars);
    vtdelete(self->iter);
    vtdelete(self->cond);
}

void CompExpr__emit_(Expr* self_, Ctx* ctx) {
    CompExpr* self = (CompExpr*)self_;
    Ctx__emit_(ctx, self->op0, 0, self->line);
    vtemit_(self->iter, ctx);
    Ctx__emit_(ctx, OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
    int block = Ctx__enter_block(ctx, CodeBlockType_FOR_LOOP);
    int block_start = Ctx__emit_(ctx, OP_FOR_ITER, block, BC_KEEPLINE);
    bool ok = vtemit_store(self->vars, ctx);
    // this error occurs in `vars` instead of this line, but...nevermind
    assert(ok);  // this should raise a SyntaxError, but we just assert it
    if(self->cond) {
        vtemit_(self->cond, ctx);
        int patch = Ctx__emit_(ctx, OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
        vtemit_(self->expr, ctx);
        Ctx__emit_(ctx, self->op1, BC_NOARG, BC_KEEPLINE);
        Ctx__patch_jump(ctx, patch);
    } else {
        vtemit_(self->expr, ctx);
        Ctx__emit_(ctx, self->op1, BC_NOARG, BC_KEEPLINE);
    }
    Ctx__emit_jump(ctx, block_start, BC_KEEPLINE);
    Ctx__exit_block(ctx);
}

CompExpr* CompExpr__new(int line, Opcode op0, Opcode op1) {
    const static ExprVt Vt = {.dtor = CompExpr__dtor, .emit_ = CompExpr__emit_};
    CompExpr* self = PK_MALLOC(sizeof(CompExpr));
    self->vt = &Vt;
    self->line = line;
    self->op0 = op0;
    self->op1 = op1;
    self->expr = NULL;
    self->vars = NULL;
    self->iter = NULL;
    self->cond = NULL;
    return self;
}

typedef struct LambdaExpr {
    EXPR_COMMON_HEADER
    int index;
} LambdaExpr;

static void LambdaExpr__emit_(Expr* self_, Ctx* ctx) {
    LambdaExpr* self = (LambdaExpr*)self_;
    Ctx__emit_(ctx, OP_LOAD_FUNCTION, self->index, self->line);
}

LambdaExpr* LambdaExpr__new(int line, int index) {
    const static ExprVt Vt = {.emit_ = LambdaExpr__emit_};
    LambdaExpr* self = PK_MALLOC(sizeof(LambdaExpr));
    self->vt = &Vt;
    self->line = line;
    self->index = index;
    return self;
}

// AndExpr, OrExpr
typedef struct LogicBinaryExpr {
    EXPR_COMMON_HEADER
    Expr* lhs;
    Expr* rhs;
    Opcode opcode;
} LogicBinaryExpr;

void LogicBinaryExpr__dtor(Expr* self_) {
    LogicBinaryExpr* self = (LogicBinaryExpr*)self_;
    vtdelete(self->lhs);
    vtdelete(self->rhs);
}

void LogicBinaryExpr__emit_(Expr* self_, Ctx* ctx) {
    LogicBinaryExpr* self = (LogicBinaryExpr*)self_;
    vtemit_(self->lhs, ctx);
    int patch = Ctx__emit_(ctx, self->opcode, BC_NOARG, self->line);
    vtemit_(self->rhs, ctx);
    Ctx__patch_jump(ctx, patch);
}

LogicBinaryExpr* LogicBinaryExpr__new(int line, Opcode opcode) {
    const static ExprVt Vt = {.emit_ = LogicBinaryExpr__emit_, .dtor = LogicBinaryExpr__dtor};
    LogicBinaryExpr* self = PK_MALLOC(sizeof(LogicBinaryExpr));
    self->vt = &Vt;
    self->line = line;
    self->lhs = NULL;
    self->rhs = NULL;
    self->opcode = opcode;
    return self;
}

typedef struct GroupedExpr {
    EXPR_COMMON_HEADER
    Expr* child;
} GroupedExpr;

void GroupedExpr__dtor(Expr* self_) {
    GroupedExpr* self = (GroupedExpr*)self_;
    vtdelete(self->child);
}

void GroupedExpr__emit_(Expr* self_, Ctx* ctx) {
    GroupedExpr* self = (GroupedExpr*)self_;
    vtemit_(self->child, ctx);
}

bool GroupedExpr__emit_del(Expr* self_, Ctx* ctx) {
    GroupedExpr* self = (GroupedExpr*)self_;
    return vtemit_del(self->child, ctx);
}

bool GroupedExpr__emit_store(Expr* self_, Ctx* ctx) {
    GroupedExpr* self = (GroupedExpr*)self_;
    return vtemit_store(self->child, ctx);
}

GroupedExpr* GroupedExpr__new(int line, Expr* child) {
    const static ExprVt Vt = {.dtor = GroupedExpr__dtor,
                              .emit_ = GroupedExpr__emit_,
                              .emit_del = GroupedExpr__emit_del,
                              .emit_store = GroupedExpr__emit_store};
    GroupedExpr* self = PK_MALLOC(sizeof(GroupedExpr));
    self->vt = &Vt;
    self->line = line;
    self->child = child;
    return self;
}

typedef struct BinaryExpr {
    EXPR_COMMON_HEADER
    Expr* lhs;
    Expr* rhs;
    TokenIndex op;
    bool inplace;
} BinaryExpr;

static void BinaryExpr__dtor(Expr* self_) {
    BinaryExpr* self = (BinaryExpr*)self_;
    vtdelete(self->lhs);
    vtdelete(self->rhs);
}

static Opcode cmp_token2op(TokenIndex token) {
    switch(token) {
        case TK_LT: return OP_COMPARE_LT;
        case TK_LE: return OP_COMPARE_LE;
        case TK_EQ: return OP_COMPARE_EQ;
        case TK_NE: return OP_COMPARE_NE;
        case TK_GT: return OP_COMPARE_GT;
        case TK_GE: return OP_COMPARE_GE;
        default: return 0;
    }
}

#define is_compare_expr(e) ((e)->vt->is_binary && cmp_token2op(((BinaryExpr*)(e))->op))

static void _emit_compare(BinaryExpr* self, Ctx* ctx, c11_vector* jmps) {
    if(is_compare_expr(self->lhs)) {
        _emit_compare((BinaryExpr*)self->lhs, ctx, jmps);
    } else {
        vtemit_(self->lhs, ctx);  // [a]
    }
    vtemit_(self->rhs, ctx);                              // [a, b]
    Ctx__emit_(ctx, OP_DUP_TOP, BC_NOARG, self->line);    // [a, b, b]
    Ctx__emit_(ctx, OP_ROT_THREE, BC_NOARG, self->line);  // [b, a, b]
    Ctx__emit_(ctx, cmp_token2op(self->op), BC_NOARG, self->line);
    // [b, RES]
    int index = Ctx__emit_(ctx, OP_SHORTCUT_IF_FALSE_OR_POP, BC_NOARG, self->line);
    c11_vector__push(int, jmps, index);
}

static void BinaryExpr__emit_(Expr* self_, Ctx* ctx) {
    BinaryExpr* self = (BinaryExpr*)self_;
    c11_vector /*T=int*/ jmps;
    c11_vector__ctor(&jmps, sizeof(int));
    if(cmp_token2op(self->op) && is_compare_expr(self->lhs)) {
        // (a < b) < c
        BinaryExpr* e = (BinaryExpr*)self->lhs;
        _emit_compare(e, ctx, &jmps);
        // [b, RES]
    } else {
        // (1 + 2) < c
        if(self->inplace) {
            vtemit_inplace(self->lhs, ctx);
        } else {
            vtemit_(self->lhs, ctx);
        }
    }

    vtemit_(self->rhs, ctx);

    Opcode opcode;
    uint16_t arg = BC_NOARG;

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

        case TK_IN:
            opcode = OP_CONTAINS_OP;
            arg = 0;
            break;
        case TK_NOT_IN:
            opcode = OP_CONTAINS_OP;
            arg = 1;
            break;
        case TK_IS:
            opcode = OP_IS_OP;
            arg = 0;
            break;
        case TK_IS_NOT:
            opcode = OP_IS_OP;
            arg = 1;
            break;

        case TK_LSHIFT: opcode = OP_BINARY_LSHIFT; break;
        case TK_RSHIFT: opcode = OP_BINARY_RSHIFT; break;
        case TK_AND: opcode = OP_BINARY_AND; break;
        case TK_OR: opcode = OP_BINARY_OR; break;
        case TK_XOR: opcode = OP_BINARY_XOR; break;
        case TK_DECORATOR: opcode = OP_BINARY_MATMUL; break;
        default: c11__unreachable();
    }

    Ctx__emit_(ctx, opcode, arg, self->line);

    for(int i = 0; i < jmps.length; i++) {
        Ctx__patch_jump(ctx, c11__getitem(int, &jmps, i));
    }
    c11_vector__dtor(&jmps);
}

BinaryExpr* BinaryExpr__new(int line, TokenIndex op, bool inplace) {
    const static ExprVt Vt = {.emit_ = BinaryExpr__emit_,
                              .dtor = BinaryExpr__dtor,
                              .is_binary = true};
    BinaryExpr* self = PK_MALLOC(sizeof(BinaryExpr));
    self->vt = &Vt;
    self->line = line;
    self->lhs = NULL;
    self->rhs = NULL;
    self->op = op;
    self->inplace = inplace;
    return self;
}

typedef struct TernaryExpr {
    EXPR_COMMON_HEADER
    Expr* cond;
    Expr* true_expr;
    Expr* false_expr;
} TernaryExpr;

void TernaryExpr__dtor(Expr* self_) {
    TernaryExpr* self = (TernaryExpr*)self_;
    vtdelete(self->cond);
    vtdelete(self->true_expr);
    vtdelete(self->false_expr);
}

void TernaryExpr__emit_(Expr* self_, Ctx* ctx) {
    TernaryExpr* self = (TernaryExpr*)self_;
    vtemit_(self->cond, ctx);
    int patch = Ctx__emit_(ctx, OP_POP_JUMP_IF_FALSE, BC_NOARG, self->cond->line);
    vtemit_(self->true_expr, ctx);
    int patch_2 = Ctx__emit_(ctx, OP_JUMP_FORWARD, BC_NOARG, self->true_expr->line);
    Ctx__patch_jump(ctx, patch);
    vtemit_(self->false_expr, ctx);
    Ctx__patch_jump(ctx, patch_2);
}

TernaryExpr* TernaryExpr__new(int line) {
    const static ExprVt Vt = {
        .dtor = TernaryExpr__dtor,
        .emit_ = TernaryExpr__emit_,
        .is_ternary = true,
    };
    TernaryExpr* self = PK_MALLOC(sizeof(TernaryExpr));
    self->vt = &Vt;
    self->line = line;
    self->cond = NULL;
    self->true_expr = NULL;
    self->false_expr = NULL;
    return self;
}

typedef struct SubscrExpr {
    EXPR_COMMON_HEADER
    Expr* lhs;
    Expr* rhs;
} SubscrExpr;

void SubscrExpr__dtor(Expr* self_) {
    SubscrExpr* self = (SubscrExpr*)self_;
    vtdelete(self->lhs);
    vtdelete(self->rhs);
}

void SubscrExpr__emit_(Expr* self_, Ctx* ctx) {
    SubscrExpr* self = (SubscrExpr*)self_;
    vtemit_(self->lhs, ctx);
    vtemit_(self->rhs, ctx);
    Ctx__emit_(ctx, OP_LOAD_SUBSCR, BC_NOARG, self->line);
}

bool SubscrExpr__emit_store(Expr* self_, Ctx* ctx) {
    SubscrExpr* self = (SubscrExpr*)self_;
    vtemit_(self->lhs, ctx);
    vtemit_(self->rhs, ctx);
    Ctx__emit_(ctx, OP_STORE_SUBSCR, BC_NOARG, self->line);
    return true;
}

void SubscrExpr__emit_inplace(Expr* self_, Ctx* ctx) {
    SubscrExpr* self = (SubscrExpr*)self_;
    vtemit_(self->lhs, ctx);
    vtemit_(self->rhs, ctx);
    Ctx__emit_(ctx, OP_DUP_TOP_TWO, BC_NOARG, self->line);
    Ctx__emit_(ctx, OP_LOAD_SUBSCR, BC_NOARG, self->line);
}

bool SubscrExpr__emit_istore(Expr* self_, Ctx* ctx) {
    SubscrExpr* self = (SubscrExpr*)self_;
    // [a, b, val] -> [val, a, b]
    Ctx__emit_(ctx, OP_ROT_THREE, BC_NOARG, self->line);
    Ctx__emit_(ctx, OP_STORE_SUBSCR, BC_NOARG, self->line);
    return true;
}

bool SubscrExpr__emit_del(Expr* self_, Ctx* ctx) {
    SubscrExpr* self = (SubscrExpr*)self_;
    vtemit_(self->lhs, ctx);
    vtemit_(self->rhs, ctx);
    Ctx__emit_(ctx, OP_DELETE_SUBSCR, BC_NOARG, self->line);
    return true;
}

SubscrExpr* SubscrExpr__new(int line) {
    const static ExprVt Vt = {
        .dtor = SubscrExpr__dtor,
        .emit_ = SubscrExpr__emit_,
        .emit_store = SubscrExpr__emit_store,
        .emit_inplace = SubscrExpr__emit_inplace,
        .emit_istore = SubscrExpr__emit_istore,
        .emit_del = SubscrExpr__emit_del,
        .is_subscr = true,
    };
    SubscrExpr* self = PK_MALLOC(sizeof(SubscrExpr));
    self->vt = &Vt;
    self->line = line;
    self->lhs = NULL;
    self->rhs = NULL;
    return self;
}

typedef struct AttribExpr {
    EXPR_COMMON_HEADER
    Expr* child;
    py_Name name;
} AttribExpr;

void AttribExpr__dtor(Expr* self_) {
    AttribExpr* self = (AttribExpr*)self_;
    vtdelete(self->child);
}

void AttribExpr__emit_(Expr* self_, Ctx* ctx) {
    AttribExpr* self = (AttribExpr*)self_;
    vtemit_(self->child, ctx);
    Ctx__emit_(ctx, OP_LOAD_ATTR, Ctx__add_name(ctx, self->name), self->line);
}

bool AttribExpr__emit_del(Expr* self_, Ctx* ctx) {
    AttribExpr* self = (AttribExpr*)self_;
    vtemit_(self->child, ctx);
    Ctx__emit_(ctx, OP_DELETE_ATTR, Ctx__add_name(ctx, self->name), self->line);
    return true;
}

bool AttribExpr__emit_store(Expr* self_, Ctx* ctx) {
    AttribExpr* self = (AttribExpr*)self_;
    vtemit_(self->child, ctx);
    Ctx__emit_(ctx, OP_STORE_ATTR, Ctx__add_name(ctx, self->name), self->line);
    return true;
}

void AttribExpr__emit_inplace(Expr* self_, Ctx* ctx) {
    AttribExpr* self = (AttribExpr*)self_;
    vtemit_(self->child, ctx);
    Ctx__emit_(ctx, OP_DUP_TOP, BC_NOARG, self->line);
    Ctx__emit_(ctx, OP_LOAD_ATTR, Ctx__add_name(ctx, self->name), self->line);
}

bool AttribExpr__emit_istore(Expr* self_, Ctx* ctx) {
    // [a, val] -> [val, a]
    AttribExpr* self = (AttribExpr*)self_;
    Ctx__emit_(ctx, OP_ROT_TWO, BC_NOARG, self->line);
    Ctx__emit_(ctx, OP_STORE_ATTR, Ctx__add_name(ctx, self->name), self->line);
    return true;
}

AttribExpr* AttribExpr__new(int line, Expr* child, py_Name name) {
    const static ExprVt Vt = {.emit_ = AttribExpr__emit_,
                              .emit_del = AttribExpr__emit_del,
                              .emit_store = AttribExpr__emit_store,
                              .emit_inplace = AttribExpr__emit_inplace,
                              .emit_istore = AttribExpr__emit_istore,
                              .dtor = AttribExpr__dtor,
                              .is_attrib = true};
    AttribExpr* self = PK_MALLOC(sizeof(AttribExpr));
    self->vt = &Vt;
    self->line = line;
    self->child = child;
    self->name = name;
    return self;
}

typedef struct CallExprKwArg {
    py_Name key;
    Expr* val;
} CallExprKwArg;

typedef struct CallExpr {
    EXPR_COMMON_HEADER
    Expr* callable;
    c11_vector /*T=Expr* */ args;
    // **a will be interpreted as a special keyword argument: {{0}: a}
    c11_vector /*T=CallExprKwArg */ kwargs;
} CallExpr;

void CallExpr__dtor(Expr* self_) {
    CallExpr* self = (CallExpr*)self_;
    vtdelete(self->callable);
    c11__foreach(Expr*, &self->args, e) vtdelete(*e);
    c11__foreach(CallExprKwArg, &self->kwargs, e) vtdelete(e->val);
    c11_vector__dtor(&self->args);
    c11_vector__dtor(&self->kwargs);
}

void CallExpr__emit_(Expr* self_, Ctx* ctx) {
    CallExpr* self = (CallExpr*)self_;

    bool vargs = false;    // whether there is *args as input
    bool vkwargs = false;  // whether there is **kwargs as input
    c11__foreach(Expr*, &self->args, e) {
        if((*e)->vt->is_starred) vargs = true;
    }
    c11__foreach(CallExprKwArg, &self->kwargs, e) {
        if(e->val->vt->is_starred) vkwargs = true;
    }

    // if callable is a AttrExpr, we should try to use `fast_call` instead of use `boundmethod`
    if(self->callable->vt->is_attrib) {
        AttribExpr* p = (AttribExpr*)self->callable;
        vtemit_(p->child, ctx);
        Ctx__emit_(ctx, OP_LOAD_METHOD, Ctx__add_name(ctx, p->name), p->line);
    } else {
        vtemit_(self->callable, ctx);
        Ctx__emit_(ctx, OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);
    }

    Opcode opcode = OP_CALL;
    if(vargs || vkwargs) {
        // in this case, there is at least one *args or **kwargs as StarredExpr
        // OP_CALL_VARGS needs to unpack them via vectorcall_buffer
        opcode = OP_CALL_VARGS;
    }

    c11__foreach(Expr*, &self->args, e) { vtemit_(*e, ctx); }
    c11__foreach(CallExprKwArg, &self->kwargs, e) {
        Ctx__emit_int(ctx, (uintptr_t)e->key, self->line);
        vtemit_(e->val, ctx);
    }
    int KWARGC = self->kwargs.length;
    int ARGC = self->args.length;
    assert(KWARGC < 256 && ARGC < 256);
    Ctx__emit_(ctx, opcode, (KWARGC << 8) | ARGC, self->line);
}

CallExpr* CallExpr__new(int line, Expr* callable) {
    const static ExprVt Vt = {.dtor = CallExpr__dtor, .emit_ = CallExpr__emit_};
    CallExpr* self = PK_MALLOC(sizeof(CallExpr));
    self->vt = &Vt;
    self->line = line;
    self->callable = callable;
    c11_vector__ctor(&self->args, sizeof(Expr*));
    c11_vector__ctor(&self->kwargs, sizeof(CallExprKwArg));
    return self;
}

/* context.c */
static void Ctx__ctor(Ctx* self, CodeObject* co, FuncDecl* func, int level) {
    self->co = co;
    self->func = func;
    self->level = level;
    self->curr_iblock = 0;
    self->is_compiling_class = false;
    c11_vector__ctor(&self->s_expr, sizeof(Expr*));
    c11_smallmap_n2d__ctor(&self->global_names);
    c11_smallmap_v2d__ctor(&self->co_consts_string_dedup_map);
}

static void Ctx__dtor(Ctx* self) {
    // clean the expr stack
    for(int i = 0; i < self->s_expr.length; i++) {
        vtdelete(c11__getitem(Expr*, &self->s_expr, i));
    }
    c11_vector__dtor(&self->s_expr);
    c11_smallmap_n2d__dtor(&self->global_names);
    // free the dedup map
    c11__foreach(c11_smallmap_v2d_KV, &self->co_consts_string_dedup_map, p_kv) {
        const char* p = p_kv->key.data;
        PK_FREE((void*)p);
    }
    c11_smallmap_v2d__dtor(&self->co_consts_string_dedup_map);
}

static int Ctx__prepare_loop_divert(Ctx* self, int line, bool is_break) {
    int index = self->curr_iblock;
    while(index >= 0) {
        CodeBlock* block = c11__at(CodeBlock, &self->co->blocks, index);
        switch(block->type) {
            case CodeBlockType_WHILE_LOOP: return index;
            case CodeBlockType_FOR_LOOP: {
                if(is_break) Ctx__emit_(self, OP_POP_TOP, BC_NOARG, line);
                return index;
            }
            case CodeBlockType_WITH: {
                Ctx__emit_(self, OP_POP_TOP, BC_NOARG, line);
                break;
            }
            case CodeBlockType_TRY: {
                Ctx__emit_(self, OP_END_TRY, BC_NOARG, line);
                break;
            }
            case CodeBlockType_EXCEPT: {
                Ctx__emit_(self, OP_END_TRY, BC_NOARG, line);
                break;
            }
            default: break;
        }
        index = block->parent;
    }
    return index;
}

static int Ctx__enter_block(Ctx* self, CodeBlockType type) {
    CodeBlock block = {type, self->curr_iblock, self->co->codes.length, -1, -1};
    c11_vector__push(CodeBlock, &self->co->blocks, block);
    self->curr_iblock = self->co->blocks.length - 1;
    return self->curr_iblock;
}

static void Ctx__exit_block(Ctx* self) {
    CodeBlock* block = c11__at(CodeBlock, &self->co->blocks, self->curr_iblock);
    block->end = self->co->codes.length;
    self->curr_iblock = block->parent;
    assert(self->curr_iblock >= 0);
}

static void Ctx__s_emit_decorators(Ctx* self, int count) {
    if(count == 0) return;
    assert(Ctx__s_size(self) >= count);
    // [obj]
    for(int i = 0; i < count; i++) {
        Expr* deco = Ctx__s_popx(self);
        vtemit_(deco, self);                                    // [obj, f]
        Ctx__emit_(self, OP_ROT_TWO, BC_NOARG, deco->line);     // [f, obj]
        Ctx__emit_(self, OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);  // [f, obj, NULL]
        Ctx__emit_(self, OP_ROT_TWO, BC_NOARG, BC_KEEPLINE);    // [obj, NULL, f]
        Ctx__emit_(self, OP_CALL, 1, deco->line);               // [obj]
        vtdelete(deco);
    }
}

static int Ctx__emit_(Ctx* self, Opcode opcode, uint16_t arg, int line) {
    Bytecode bc = {(uint8_t)opcode, arg};
    BytecodeEx bcx = {line, self->curr_iblock};
    c11_vector__push(Bytecode, &self->co->codes, bc);
    c11_vector__push(BytecodeEx, &self->co->codes_ex, bcx);
    int i = self->co->codes.length - 1;
    BytecodeEx* codes_ex = (BytecodeEx*)self->co->codes_ex.data;
    if(line == BC_KEEPLINE) { codes_ex[i].lineno = i >= 1 ? codes_ex[i - 1].lineno : 1; }
    return i;
}

// static void Ctx__revert_last_emit_(Ctx* self) {
//     c11_vector__pop(&self->co->codes);
//     c11_vector__pop(&self->co->codes_ex);
// }

static int Ctx__emit_int(Ctx* self, int64_t value, int line) {
    if(INT16_MIN <= value && value <= INT16_MAX) {
        return Ctx__emit_(self, OP_LOAD_SMALL_INT, (uint16_t)value, line);
    } else {
        py_TValue tmp;
        py_newint(&tmp, value);
        return Ctx__emit_(self, OP_LOAD_CONST, Ctx__add_const(self, &tmp), line);
    }
}

static void Ctx__patch_jump(Ctx* self, int index) {
    Bytecode* co_codes = (Bytecode*)self->co->codes.data;
    int target = self->co->codes.length;
    Bytecode__set_signed_arg(&co_codes[index], target - index);
}

static void Ctx__emit_jump(Ctx* self, int target, int line) {
    int index = Ctx__emit_(self, OP_JUMP_FORWARD, BC_NOARG, line);
    // should place after Ctx__emit_ because of realloc
    Bytecode* co_codes = (Bytecode*)self->co->codes.data;
    Bytecode__set_signed_arg(&co_codes[index], target - index);
}

static int Ctx__add_varname(Ctx* self, py_Name name) {
    // PK_MAX_CO_VARNAMES will be checked when pop_context(), not here
    return CodeObject__add_varname(self->co, name);
}

static int Ctx__add_name(Ctx* self, py_Name name) { return CodeObject__add_name(self->co, name); }

static int Ctx__add_const_string(Ctx* self, c11_sv key) {
    if(key.size > 100) {
        py_Ref tmp = c11_vector__emplace(&self->co->consts);
        py_newstrv(tmp, key);
        int index = self->co->consts.length - 1;
        return index;
    }
    int* val = c11_smallmap_v2d__try_get(&self->co_consts_string_dedup_map, key);
    if(val) {
        return *val;
    } else {
        py_Ref tmp = c11_vector__emplace(&self->co->consts);
        py_newstrv(tmp, key);
        int index = self->co->consts.length - 1;
        // dedup
        char* new_buf = PK_MALLOC(key.size + 1);
        memcpy(new_buf, key.data, key.size);
        new_buf[key.size] = 0;
        c11_smallmap_v2d__set(&self->co_consts_string_dedup_map,
                              (c11_sv){new_buf, key.size},
                              index);
        return index;
    }
}

static int Ctx__add_const(Ctx* self, py_Ref v) {
    assert(v->type != tp_str);
    c11_vector__push(py_TValue, &self->co->consts, *v);
    return self->co->consts.length - 1;
}

static void Ctx__emit_store_name(Ctx* self, NameScope scope, py_Name name, int line) {
    switch(scope) {
        case NAME_LOCAL: Ctx__emit_(self, OP_STORE_FAST, Ctx__add_varname(self, name), line); break;
        case NAME_GLOBAL: {
            Opcode op = self->co->src->is_dynamic ? OP_STORE_NAME : OP_STORE_GLOBAL;
            Ctx__emit_(self, op, Ctx__add_name(self, name), line);
        } break;
        default: c11__unreachable();
    }
}

// emit top -> pop -> delete
static void Ctx__s_emit_top(Ctx* self) {
    assert(self->s_expr.length);
    Expr* top = c11_vector__back(Expr*, &self->s_expr);
    vtemit_(top, self);
    vtdelete(top);
    c11_vector__pop(&self->s_expr);
}

// push
static void Ctx__s_push(Ctx* self, Expr* expr) { c11_vector__push(Expr*, &self->s_expr, expr); }

// top
static Expr* Ctx__s_top(Ctx* self) {
    assert(self->s_expr.length);
    return c11_vector__back(Expr*, &self->s_expr);
}

// size
static int Ctx__s_size(Ctx* self) { return self->s_expr.length; }

// pop -> delete
static void Ctx__s_pop(Ctx* self) {
    assert(self->s_expr.length);
    Expr* top = c11_vector__back(Expr*, &self->s_expr);
    vtdelete(top);
    c11_vector__pop(&self->s_expr);
}

// pop move
static Expr* Ctx__s_popx(Ctx* self) {
    assert(self->s_expr.length);
    Expr* top = c11_vector__back(Expr*, &self->s_expr);
    c11_vector__pop(&self->s_expr);
    return top;
}

/* compiler.c */
typedef struct Compiler Compiler;
typedef Error* (*PrattCallback)(Compiler* self);

typedef struct PrattRule {
    PrattCallback prefix;
    PrattCallback infix;
    enum Precedence precedence;
} PrattRule;

const static PrattRule rules[TK__COUNT__];

typedef struct Compiler {
    SourceData_ src;  // weakref

    Token* tokens;
    int tokens_length;

    int i;  // current token index
    c11_vector /*T=CodeEmitContext*/ contexts;
} Compiler;

static void Compiler__ctor(Compiler* self, SourceData_ src, Token* tokens, int tokens_length) {
    self->src = src;
    self->tokens = tokens;
    self->tokens_length = tokens_length;
    self->i = 0;
    c11_vector__ctor(&self->contexts, sizeof(Ctx));
}

static void Compiler__dtor(Compiler* self) {
    // free tokens
    for(int i = 0; i < self->tokens_length; i++) {
        if(self->tokens[i].value.index == TokenValue_STR) {
            // PK_FREE internal string
            c11_string__delete(self->tokens[i].value._str);
        }
    }
    PK_FREE(self->tokens);
    // free contexts
    c11__foreach(Ctx, &self->contexts, ctx) Ctx__dtor(ctx);
    c11_vector__dtor(&self->contexts);
}

/**************************************/
#define tk(i) (&self->tokens[i])
#define prev() (&self->tokens[self->i - 1])
#define curr() (&self->tokens[self->i])
#define next() (&self->tokens[self->i + 1])

#define advance() self->i++
#define mode() self->src->mode
#define ctx() (&c11_vector__back(Ctx, &self->contexts))

#define match_newlines() match_newlines_impl(self)

#define consume(expected)                                                                          \
    if(!match(expected))                                                                           \
        return SyntaxError(self,                                                                   \
                           "expected '%s', got '%s'",                                              \
                           TokenSymbols[expected],                                                 \
                           TokenSymbols[curr()->type]);
#define consume_end_stmt()                                                                         \
    if(!match_end_stmt(self)) return SyntaxError(self, "expected statement end")

#define check(B)                                                                                   \
    if((err = B)) return err

static NameScope name_scope(Compiler* self) {
    return self->contexts.length > 1 ? NAME_LOCAL : NAME_GLOBAL;
}

Error* SyntaxError(Compiler* self, const char* fmt, ...) {
    Error* err = PK_MALLOC(sizeof(Error));
    err->src = self->src;
    PK_INCREF(self->src);
    Token* t = self->i == self->tokens_length ? prev() : curr();
    err->lineno = t->line;
    va_list args;
    va_start(args, fmt);
    vsnprintf(err->msg, sizeof(err->msg), fmt, args);
    va_end(args);
    return err;
}

/* Matchers */
static bool is_expression(Compiler* self, bool allow_slice) {
    PrattCallback prefix = rules[curr()->type].prefix;
    return prefix && (allow_slice || curr()->type != TK_COLON);
}

#define match(expected) (curr()->type == expected ? (++self->i) : 0)

static bool match_id_by_str(Compiler* self, const char* name) {
    if(curr()->type == TK_ID) {
        bool ok = c11__sveq2(Token__sv(curr()), name);
        if(ok) advance();
        return ok;
    }
    return false;
}

static bool match_newlines_impl(Compiler* self) {
    bool consumed = false;
    if(curr()->type == TK_EOL) {
        while(curr()->type == TK_EOL)
            advance();
        consumed = true;
    }
    return consumed;
}

static bool match_end_stmt(Compiler* self) {
    if(match(TK_SEMICOLON)) {
        match_newlines();
        return true;
    }
    if(match_newlines() || curr()->type == TK_EOF) return true;
    if(curr()->type == TK_DEDENT) return true;
    return false;
}

/* Expression */

/// Parse an expression and push it onto the stack.
static Error* parse_expression(Compiler* self, int precedence, bool allow_slice) {
    PrattCallback prefix = rules[curr()->type].prefix;
    if(!prefix || (curr()->type == TK_COLON && !allow_slice)) {
        return SyntaxError(self, "expected an expression, got %s", TokenSymbols[curr()->type]);
    }
    advance();
    Error* err;
    check(prefix(self));
    while(rules[curr()->type].precedence >= precedence &&
          (allow_slice || curr()->type != TK_COLON)) {
        TokenIndex op = curr()->type;
        advance();
        PrattCallback infix = rules[op].infix;
        if(infix == NULL) {
            return SyntaxError(self, "expected an infix operator, got %s", TokenSymbols[op]);
        }
        check(infix(self));
    }
    return NULL;
}

static Error* EXPR_TUPLE_ALLOW_SLICE(Compiler* self, bool allow_slice) {
    Error* err;
    check(parse_expression(self, PREC_LOWEST + 1, allow_slice));
    if(!match(TK_COMMA)) return NULL;
    // tuple expression     // (a, )
    int count = 1;
    do {
        if(curr()->brackets_level) match_newlines();
        if(!is_expression(self, allow_slice)) break;
        check(parse_expression(self, PREC_LOWEST + 1, allow_slice));
        count += 1;
        if(curr()->brackets_level) match_newlines();
    } while(match(TK_COMMA));
    // pop `count` expressions from the stack and merge them into a TupleExpr
    SequenceExpr* e = TupleExpr__new(prev()->line, count);
    for(int i = count - 1; i >= 0; i--) {
        e->items[i] = Ctx__s_popx(ctx());
    }
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

/// Parse a simple expression.
static Error* EXPR(Compiler* self) { return parse_expression(self, PREC_LOWEST + 1, false); }

/// Parse a simple expression or a tuple of expressions.
static Error* EXPR_TUPLE(Compiler* self) { return EXPR_TUPLE_ALLOW_SLICE(self, false); }

// special case for `for loop` and `comp`
static Error* EXPR_VARS(Compiler* self) {
    int count = 0;
    do {
        consume(TK_ID);
        py_Name name = py_namev(Token__sv(prev()));
        NameExpr* e = NameExpr__new(prev()->line, name, name_scope(self));
        Ctx__s_push(ctx(), (Expr*)e);
        count += 1;
    } while(match(TK_COMMA));
    if(count > 1) {
        SequenceExpr* e = TupleExpr__new(prev()->line, count);
        for(int i = count - 1; i >= 0; i--) {
            e->items[i] = Ctx__s_popx(ctx());
        }
        Ctx__s_push(ctx(), (Expr*)e);
    }
    return NULL;
}

/* Misc */
static void push_global_context(Compiler* self, CodeObject* co) {
    co->start_line = self->i == 0 ? 1 : prev()->line;
    Ctx* ctx = c11_vector__emplace(&self->contexts);
    Ctx__ctor(ctx, co, NULL, self->contexts.length);
}

static Error* pop_context(Compiler* self) {
    // add a `return None` in the end as a guard
    // previously, we only do this if the last opcode is not a return
    // however, this is buggy...since there may be a jump to the end (out of bound) even if the last
    // opcode is a return
    Ctx__emit_(ctx(), OP_RETURN_VALUE, BC_RETURN_VIRTUAL, BC_KEEPLINE);

    CodeObject* co = ctx()->co;
    // find the last valid token
    int j = self->i - 1;
    while(tk(j)->type == TK_EOL || tk(j)->type == TK_DEDENT || tk(j)->type == TK_EOF)
        j--;
    co->end_line = tk(j)->line;

    // some check here
    c11_vector* codes = &co->codes;
    if(co->nlocals > PK_MAX_CO_VARNAMES) {
        return SyntaxError(self, "maximum number of local variables exceeded");
    }
    if(co->consts.length > 65530) {
        return SyntaxError(self, "maximum number of constants exceeded");
    }
    // pre-compute block.end or block.end2
    for(int i = 0; i < codes->length; i++) {
        Bytecode* bc = c11__at(Bytecode, codes, i);
        if(bc->op == OP_LOOP_CONTINUE) {
            CodeBlock* block = c11__at(CodeBlock, &ctx()->co->blocks, bc->arg);
            Bytecode__set_signed_arg(bc, block->start - i);
        } else if(bc->op == OP_LOOP_BREAK) {
            CodeBlock* block = c11__at(CodeBlock, &ctx()->co->blocks, bc->arg);
            Bytecode__set_signed_arg(bc, (block->end2 != -1 ? block->end2 : block->end) - i);
        } else if(bc->op == OP_FOR_ITER || bc->op == OP_FOR_ITER_YIELD_VALUE) {
            CodeBlock* block = c11__at(CodeBlock, &ctx()->co->blocks, bc->arg);
            Bytecode__set_signed_arg(bc, block->end - i);
        }
    }
    // pre-compute func->is_simple
    FuncDecl* func = ctx()->func;
    if(func) {
        // check generator
        Bytecode* codes = func->code.codes.data;
        int codes_length = func->code.codes.length;

        for(int i = 0; i < codes_length; i++) {
            if(codes[i].op == OP_YIELD_VALUE || codes[i].op == OP_FOR_ITER_YIELD_VALUE) {
                func->type = FuncType_GENERATOR;
                break;
            }
        }

        if(func->type == FuncType_UNSET) {
            bool is_simple = true;
            if(func->kwargs.length > 0) is_simple = false;
            if(func->starred_arg >= 0) is_simple = false;
            if(func->starred_kwarg >= 0) is_simple = false;

            if(is_simple) {
                func->type = FuncType_SIMPLE;
            } else {
                func->type = FuncType_NORMAL;
            }
        }

        assert(func->type != FuncType_UNSET);
    }
    Ctx__dtor(ctx());
    c11_vector__pop(&self->contexts);
    return NULL;
}

/* Expression Callbacks */
static Error* exprLiteral(Compiler* self) {
    LiteralExpr* e = LiteralExpr__new(prev()->line, &prev()->value);
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprBytes(Compiler* self) {
    c11_sv sv = c11_string__sv(prev()->value._str);
    Ctx__s_push(ctx(), (Expr*)RawStringExpr__new(prev()->line, sv, OP_BUILD_BYTES));
    return NULL;
}

static Error* exprFString(Compiler* self) {
    // @fstr-begin, [@fstr-cpnt | <expr>]*, @fstr-end
    int count = 0;
    int line = prev()->line;
    while(true) {
        if(match(TK_FSTR_END)) {
            SequenceExpr* e = FStringExpr__new(line, count);
            for(int i = count - 1; i >= 0; i--) {
                e->items[i] = Ctx__s_popx(ctx());
            }
            Ctx__s_push(ctx(), (Expr*)e);
            return NULL;
        } else if(match(TK_FSTR_CPNT)) {
            // OP_LOAD_CONST
            LiteralExpr* e = LiteralExpr__new(prev()->line, &prev()->value);
            Ctx__s_push(ctx(), (Expr*)e);
            count++;
        } else {
            // {a!r:.2f}
            Error* err = EXPR(self);
            if(err) return err;
            count++;

            if(match(TK_FSTR_SPEC)) {
                c11_sv spec = Token__sv(prev());
                // ':.2f}' -> ':.2f'
                spec.size--;
                Expr* child = Ctx__s_popx(ctx());
                FStringSpecExpr* e = FStringSpecExpr__new(prev()->line, child, spec);
                Ctx__s_push(ctx(), (Expr*)e);
            }
        }
    }
}

static Error* exprImag(Compiler* self) {
    Ctx__s_push(ctx(), (Expr*)ImagExpr__new(prev()->line, prev()->value._f64));
    return NULL;
}

static FuncDecl_ push_f_context(Compiler* self, c11_sv name, int* out_index);
static Error* _compile_f_args(Compiler* self, FuncDecl* decl, bool is_lambda);

static Error* exprLambda(Compiler* self) {
    Error* err;
    int line = prev()->line;
    int decl_index;
    FuncDecl_ decl = push_f_context(self, (c11_sv){"<lambda>", 8}, &decl_index);
    if(!match(TK_COLON)) {
        check(_compile_f_args(self, decl, true));
        consume(TK_COLON);
    }
    // https://github.com/pocketpy/pocketpy/issues/37
    check(parse_expression(self, PREC_LAMBDA + 1, false));
    Ctx__s_emit_top(ctx());
    Ctx__emit_(ctx(), OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
    check(pop_context(self));
    LambdaExpr* e = LambdaExpr__new(line, decl_index);
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprOr(Compiler* self) {
    Error* err;
    int line = prev()->line;
    check(parse_expression(self, PREC_LOGICAL_OR + 1, false));
    LogicBinaryExpr* e = LogicBinaryExpr__new(line, OP_JUMP_IF_TRUE_OR_POP);
    e->rhs = Ctx__s_popx(ctx());
    e->lhs = Ctx__s_popx(ctx());
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprAnd(Compiler* self) {
    Error* err;
    int line = prev()->line;
    check(parse_expression(self, PREC_LOGICAL_AND + 1, false));
    LogicBinaryExpr* e = LogicBinaryExpr__new(line, OP_JUMP_IF_FALSE_OR_POP);
    e->rhs = Ctx__s_popx(ctx());
    e->lhs = Ctx__s_popx(ctx());
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprTernary(Compiler* self) {
    // [true_expr]
    Error* err;
    int line = prev()->line;
    check(parse_expression(self, PREC_TERNARY + 1, false));  // [true_expr, cond]
    consume(TK_ELSE);
    check(parse_expression(self, PREC_TERNARY + 1, false));  // [true_expr, cond, false_expr]
    TernaryExpr* e = TernaryExpr__new(line);
    e->false_expr = Ctx__s_popx(ctx());
    e->cond = Ctx__s_popx(ctx());
    e->true_expr = Ctx__s_popx(ctx());
    Ctx__s_push(ctx(), (Expr*)e);

    if(e->cond->vt->is_ternary || e->false_expr->vt->is_ternary || e->true_expr->vt->is_ternary) {
        return SyntaxError(self, "nested ternary expressions without `()` are ambiguous");
    }
    return NULL;
}

static Error* exprBinaryOp(Compiler* self) {
    Error* err;
    int line = prev()->line;
    TokenIndex op = prev()->type;
    int precedence = rules[op].precedence;
    if(op != TK_POW) {
        // if not right associative, increase precedence
        precedence += 1;
    }
    check(parse_expression(self, precedence, false));
    BinaryExpr* e = BinaryExpr__new(line, op, false);
    if(op == TK_IN || op == TK_NOT_IN) {
        e->lhs = Ctx__s_popx(ctx());
        e->rhs = Ctx__s_popx(ctx());
    } else {
        e->rhs = Ctx__s_popx(ctx());
        e->lhs = Ctx__s_popx(ctx());
    }
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprNot(Compiler* self) {
    Error* err;
    int line = prev()->line;
    check(parse_expression(self, PREC_LOGICAL_NOT + 1, false));
    UnaryExpr* e = UnaryExpr__new(line, Ctx__s_popx(ctx()), OP_UNARY_NOT);
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprUnaryOp(Compiler* self) {
    Error* err;
    int line = prev()->line;
    TokenIndex op = prev()->type;
    check(parse_expression(self, PREC_UNARY + 1, false));
    Expr* e = Ctx__s_popx(ctx());
    switch(op) {
        case TK_SUB: {
            // constant fold
            if(e->vt->is_literal) {
                LiteralExpr* le = (LiteralExpr*)e;
                if(le->value->index == TokenValue_I64 || le->value->index == TokenValue_F64) {
                    le->negated = true;
                }
                Ctx__s_push(ctx(), e);
            } else {
                Ctx__s_push(ctx(), (Expr*)UnaryExpr__new(line, e, OP_UNARY_NEGATIVE));
            }
            break;
        }
        case TK_INVERT: Ctx__s_push(ctx(), (Expr*)UnaryExpr__new(line, e, OP_UNARY_INVERT)); break;
        case TK_MUL: Ctx__s_push(ctx(), (Expr*)StarredExpr__new(line, e, 1)); break;
        case TK_POW: Ctx__s_push(ctx(), (Expr*)StarredExpr__new(line, e, 2)); break;
        default: assert(false);
    }
    return NULL;
}

static Error* exprGroup(Compiler* self) {
    Error* err;
    int line = prev()->line;
    if(match(TK_RPAREN)) {
        // empty tuple
        Ctx__s_push(ctx(), (Expr*)TupleExpr__new(line, 0));
        return NULL;
    }
    match_newlines();
    check(EXPR_TUPLE(self));  // () is just for change precedence
    match_newlines();
    consume(TK_RPAREN);
    if(Ctx__s_top(ctx())->vt->is_tuple) return NULL;
    GroupedExpr* g = GroupedExpr__new(line, Ctx__s_popx(ctx()));
    Ctx__s_push(ctx(), (Expr*)g);
    return NULL;
}

static Error* exprName(Compiler* self) {
    py_Name name = py_namev(Token__sv(prev()));
    NameScope scope = name_scope(self);
    // promote this name to global scope if needed
    if(c11_smallmap_n2d__contains(&ctx()->global_names, name)) {
        if(self->src->is_dynamic) return SyntaxError(self, "cannot use global keyword here");
        scope = NAME_GLOBAL;
    }
    NameExpr* e = NameExpr__new(prev()->line, name, scope);
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprAttrib(Compiler* self) {
    consume(TK_ID);
    py_Name name = py_namev(Token__sv(prev()));
    AttribExpr* e = AttribExpr__new(prev()->line, Ctx__s_popx(ctx()), name);
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprLiteral0(Compiler* self) {
    Literal0Expr* e = Literal0Expr__new(prev()->line, prev()->type);
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* consume_comp(Compiler* self, Opcode op0, Opcode op1) {
    // [expr]
    Error* err;
    int line = prev()->line;
    bool has_cond = false;
    check(EXPR_VARS(self));  // [expr, vars]
    consume(TK_IN);
    check(parse_expression(self, PREC_TERNARY + 1, false));  // [expr, vars, iter]
    match_newlines();
    if(match(TK_IF)) {
        check(parse_expression(self, PREC_TERNARY + 1, false));  // [expr, vars, iter, cond]
        has_cond = true;
    }
    CompExpr* ce = CompExpr__new(line, op0, op1);
    if(has_cond) ce->cond = Ctx__s_popx(ctx());
    ce->iter = Ctx__s_popx(ctx());
    ce->vars = Ctx__s_popx(ctx());
    ce->expr = Ctx__s_popx(ctx());
    Ctx__s_push(ctx(), (Expr*)ce);
    match_newlines();
    return NULL;
}

static Error* exprList(Compiler* self) {
    Error* err;
    int line = prev()->line;
    int count = 0;
    do {
        match_newlines();
        if(curr()->type == TK_RBRACKET) break;
        check(EXPR(self));
        count += 1;
        match_newlines();
        if(count == 1 && match(TK_FOR)) {
            check(consume_comp(self, OP_BUILD_LIST, OP_LIST_APPEND));
            consume(TK_RBRACKET);
            return NULL;
        }
        match_newlines();
    } while(match(TK_COMMA));
    consume(TK_RBRACKET);
    SequenceExpr* e = ListExpr__new(line, count);
    for(int i = count - 1; i >= 0; i--) {
        e->items[i] = Ctx__s_popx(ctx());
    }
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

static Error* exprMap(Compiler* self) {
    Error* err;
    int line = prev()->line;
    bool parsing_dict = false;  // {...} may be dict or set
    int count = 0;
    do {
        match_newlines();
        if(curr()->type == TK_RBRACE) break;
        check(EXPR(self));  // [key]
        if(curr()->type == TK_COLON) { parsing_dict = true; }
        if(parsing_dict) {
            consume(TK_COLON);
            check(EXPR(self));  // [key, value] -> [item]
            DictItemExpr* item = DictItemExpr__new(prev()->line);
            item->value = Ctx__s_popx(ctx());
            item->key = Ctx__s_popx(ctx());
            Ctx__s_push(ctx(), (Expr*)item);
        }
        count += 1;  // key-value pair count
        match_newlines();
        if(count == 1 && match(TK_FOR)) {
            if(parsing_dict) {
                check(consume_comp(self, OP_BUILD_DICT, OP_DICT_ADD));
            } else {
                check(consume_comp(self, OP_BUILD_SET, OP_SET_ADD));
            }
            consume(TK_RBRACE);
            return NULL;
        }
        match_newlines();
    } while(match(TK_COMMA));
    consume(TK_RBRACE);

    SequenceExpr* se;
    if(count == 0 || parsing_dict) {
        se = DictExpr__new(line, count);
    } else {
        se = SetExpr__new(line, count);
    }
    for(int i = count - 1; i >= 0; i--) {
        se->items[i] = Ctx__s_popx(ctx());
    }
    Ctx__s_push(ctx(), (Expr*)se);
    return NULL;
}

static Error* read_literal(Compiler* self, py_Ref out);

static Error* exprCompileTimeCall(Compiler* self, py_ItemRef func, int line) {
    Error* err;
    py_push(func);
    py_pushnil();

    uint16_t argc = 0;
    uint16_t kwargc = 0;
    // copied from `exprCall`
    do {
        match_newlines();
        if(curr()->type == TK_RPAREN) break;
        if(curr()->type == TK_ID && next()->type == TK_ASSIGN) {
            consume(TK_ID);
            py_Name key = py_namev(Token__sv(prev()));
            consume(TK_ASSIGN);
            // k=v
            py_pushname(key);
            check(read_literal(self, py_pushtmp()));
            kwargc += 1;
        } else {
            if(kwargc > 0) {
                return SyntaxError(self, "positional argument follows keyword argument");
            }
            check(read_literal(self, py_pushtmp()));
            argc += 1;
        }
        match_newlines();
    } while(match(TK_COMMA));
    consume(TK_RPAREN);

    py_StackRef p0 = py_peek(0);
    bool ok = py_vectorcall(argc, kwargc);
    if(!ok) {
        char* msg = py_formatexc();
        py_clearexc(p0);
        err = SyntaxError(self, "compile-time call error:\n%s", msg);
        PK_FREE(msg);
        return err;
    }

    // TODO: optimize string dedup
    int index = Ctx__add_const(ctx(), py_retval());
    Ctx__s_push(ctx(), (Expr*)LoadConstExpr__new(line, index));
    return NULL;
}

static Error* exprCall(Compiler* self) {
    Error* err;
    Expr* callable = Ctx__s_popx(ctx());
    int line = prev()->line;
    if(callable->vt->is_name) {
        NameExpr* ne = (NameExpr*)callable;
        py_ItemRef func = py_macroget(ne->name);
        if(func != NULL) {
            py_StackRef p0 = py_peek(0);
            err = exprCompileTimeCall(self, func, line);
            if(err != NULL) py_clearexc(p0);
            return err;
        }
    }

    CallExpr* e = CallExpr__new(line, callable);
    Ctx__s_push(ctx(), (Expr*)e);  // push onto the stack in advance
    do {
        match_newlines();
        if(curr()->type == TK_RPAREN) break;
        if(curr()->type == TK_ID && next()->type == TK_ASSIGN) {
            consume(TK_ID);
            py_Name key = py_namev(Token__sv(prev()));
            consume(TK_ASSIGN);
            check(EXPR(self));
            CallExprKwArg kw = {key, Ctx__s_popx(ctx())};
            c11_vector__push(CallExprKwArg, &e->kwargs, kw);
        } else {
            check(EXPR(self));
            int star_level = 0;
            Expr* top = Ctx__s_top(ctx());
            if(top->vt->is_starred) star_level = ((StarredExpr*)top)->level;
            if(star_level == 2) {
                // **kwargs
                CallExprKwArg kw = {0, Ctx__s_popx(ctx())};
                c11_vector__push(CallExprKwArg, &e->kwargs, kw);
            } else {
                // positional argument
                if(e->kwargs.length > 0) {
                    return SyntaxError(self, "positional argument follows keyword argument");
                }
                c11_vector__push(Expr*, &e->args, Ctx__s_popx(ctx()));
            }
        }
        match_newlines();
    } while(match(TK_COMMA));
    consume(TK_RPAREN);
    return NULL;
}

static Error* exprSlice0(Compiler* self) {
    Error* err;
    SliceExpr* slice = SliceExpr__new(prev()->line);
    Ctx__s_push(ctx(), (Expr*)slice);  // push onto the stack in advance
    if(is_expression(self, false)) {   // :<stop>
        check(EXPR(self));
        slice->stop = Ctx__s_popx(ctx());
        // try optional step
        if(match(TK_COLON)) {  // :<stop>:<step>
            check(EXPR(self));
            slice->step = Ctx__s_popx(ctx());
        }
    } else if(match(TK_COLON)) {
        if(is_expression(self, false)) {  // ::<step>
            check(EXPR(self));
            slice->step = Ctx__s_popx(ctx());
        }  // else ::
    }  // else :
    return NULL;
}

static Error* exprSlice1(Compiler* self) {
    Error* err;
    SliceExpr* slice = SliceExpr__new(prev()->line);
    slice->start = Ctx__s_popx(ctx());
    Ctx__s_push(ctx(), (Expr*)slice);  // push onto the stack in advance
    if(is_expression(self, false)) {   // <start>:<stop>
        check(EXPR(self));
        slice->stop = Ctx__s_popx(ctx());
        // try optional step
        if(match(TK_COLON)) {  // <start>:<stop>:<step>
            check(EXPR(self));
            slice->step = Ctx__s_popx(ctx());
        }
    } else if(match(TK_COLON)) {  // <start>::<step>
        check(EXPR(self));
        slice->step = Ctx__s_popx(ctx());
    }  // else <start>:
    return NULL;
}

static Error* exprSubscr(Compiler* self) {
    Error* err;
    int line = prev()->line;
    match_newlines();
    check(EXPR_TUPLE_ALLOW_SLICE(self, true));
    match_newlines();
    consume(TK_RBRACKET);  // [lhs, rhs]
    SubscrExpr* e = SubscrExpr__new(line);
    e->rhs = Ctx__s_popx(ctx());  // [lhs]
    e->lhs = Ctx__s_popx(ctx());  // []
    Ctx__s_push(ctx(), (Expr*)e);
    return NULL;
}

////////////////
static Error* consume_type_hints(Compiler* self) {
    Error* err;
    check(EXPR(self));
    Ctx__s_pop(ctx());
    return NULL;
}

static Error* consume_type_hints_sv(Compiler* self, c11_sv* out) {
    Error* err;
    const char* start = curr()->start;
    check(EXPR(self));
    const char* end = prev()->start + prev()->length;
    *out = (c11_sv){start, end - start};
    Ctx__s_pop(ctx());
    return NULL;
}

static Error* compile_stmt(Compiler* self);

static Error* compile_block_body(Compiler* self) {
    Error* err;
    consume(TK_COLON);

    if(curr()->type != TK_EOL && curr()->type != TK_EOF) {
        while(true) {
            check(compile_stmt(self));
            bool possible = curr()->type != TK_EOL && curr()->type != TK_EOF;
            if(prev()->type != TK_SEMICOLON || !possible) break;
        }
        return NULL;
    }

    bool consumed = match_newlines();
    if(!consumed) return SyntaxError(self, "expected a new line after ':'");

    consume(TK_INDENT);
    while(curr()->type != TK_DEDENT) {
        match_newlines();
        check(compile_stmt(self));
        match_newlines();
    }
    consume(TK_DEDENT);
    return NULL;
}

static Error* compile_if_stmt(Compiler* self) {
    Error* err;
    check(EXPR(self));  // condition
    Ctx__s_emit_top(ctx());
    int patch = Ctx__emit_(ctx(), OP_POP_JUMP_IF_FALSE, BC_NOARG, prev()->line);
    err = compile_block_body(self);
    if(err) return err;
    if(match(TK_ELIF)) {
        int exit_patch = Ctx__emit_(ctx(), OP_JUMP_FORWARD, BC_NOARG, prev()->line);
        Ctx__patch_jump(ctx(), patch);
        check(compile_if_stmt(self));
        Ctx__patch_jump(ctx(), exit_patch);
    } else if(match(TK_ELSE)) {
        int exit_patch = Ctx__emit_(ctx(), OP_JUMP_FORWARD, BC_NOARG, prev()->line);
        Ctx__patch_jump(ctx(), patch);
        check(compile_block_body(self));
        Ctx__patch_jump(ctx(), exit_patch);
    } else {
        Ctx__patch_jump(ctx(), patch);
    }
    return NULL;
}

static Error* compile_match_case(Compiler* self, c11_vector* patches) {
    Error* err;
    bool is_case_default = false;

    check(EXPR(self));  // condition
    Ctx__s_emit_top(ctx());

    consume(TK_COLON);

    bool consumed = match_newlines();
    if(!consumed) return SyntaxError(self, "expected a new line after ':'");

    consume(TK_INDENT);
    while(curr()->type != TK_DEDENT) {
        match_newlines();

        if(match_id_by_str(self, "case")) {
            if(is_case_default) return SyntaxError(self, "case _: must be the last one");
            is_case_default = match_id_by_str(self, "_");

            if(!is_case_default) {
                Ctx__emit_(ctx(), OP_DUP_TOP, BC_NOARG, prev()->line);
                check(EXPR(self));  // expr
                Ctx__s_emit_top(ctx());
                int patch = Ctx__emit_(ctx(), OP_POP_JUMP_IF_NOT_MATCH, BC_NOARG, prev()->line);
                check(compile_block_body(self));
                int break_patch = Ctx__emit_(ctx(), OP_JUMP_FORWARD, BC_NOARG, prev()->line);
                c11_vector__push(int, patches, break_patch);
                Ctx__patch_jump(ctx(), patch);
            } else {
                check(compile_block_body(self));
            }
        } else {
            return SyntaxError(self, "expected 'case', got '%s'", TokenSymbols[curr()->type]);
        }

        match_newlines();
    }
    consume(TK_DEDENT);

    for(int i = 0; i < patches->length; i++) {
        int patch = c11__getitem(int, patches, i);
        Ctx__patch_jump(ctx(), patch);
    }
    Ctx__emit_(ctx(), OP_POP_TOP, BC_NOARG, prev()->line);
    return NULL;
}

static Error* compile_while_loop(Compiler* self) {
    Error* err;
    int block = Ctx__enter_block(ctx(), CodeBlockType_WHILE_LOOP);
    int block_start = c11__at(CodeBlock, &ctx()->co->blocks, block)->start;
    check(EXPR(self));  // condition
    Ctx__s_emit_top(ctx());
    int patch = Ctx__emit_(ctx(), OP_POP_JUMP_IF_FALSE, BC_NOARG, prev()->line);
    check(compile_block_body(self));
    Ctx__emit_jump(ctx(), block_start, BC_KEEPLINE);
    Ctx__patch_jump(ctx(), patch);
    Ctx__exit_block(ctx());
    // optional else clause
    if(match(TK_ELSE)) {
        check(compile_block_body(self));
        CodeBlock* p_block = c11__at(CodeBlock, &ctx()->co->blocks, block);
        p_block->end2 = ctx()->co->codes.length;
    }
    return NULL;
}

static Error* compile_for_loop(Compiler* self) {
    Error* err;
    check(EXPR_VARS(self));  // [vars]
    consume(TK_IN);
    check(EXPR_TUPLE(self));  // [vars, iter]
    Ctx__s_emit_top(ctx());   // [vars]
    Ctx__emit_(ctx(), OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
    int block = Ctx__enter_block(ctx(), CodeBlockType_FOR_LOOP);
    int block_start = Ctx__emit_(ctx(), OP_FOR_ITER, block, BC_KEEPLINE);
    Expr* vars = Ctx__s_popx(ctx());
    bool ok = vtemit_store(vars, ctx());
    vtdelete(vars);
    if(!ok) {
        // this error occurs in `vars` instead of this line, but...nevermind
        return SyntaxError(self, "invalid syntax");
    }
    check(compile_block_body(self));
    Ctx__emit_jump(ctx(), block_start, BC_KEEPLINE);
    Ctx__exit_block(ctx());
    // optional else clause
    if(match(TK_ELSE)) {
        check(compile_block_body(self));
        CodeBlock* p_block = c11__at(CodeBlock, &ctx()->co->blocks, block);
        p_block->end2 = ctx()->co->codes.length;
    }
    return NULL;
}

static Error* compile_yield_from(Compiler* self, int kw_line) {
    Error* err;
    if(self->contexts.length <= 1) return SyntaxError(self, "'yield from' outside function");
    check(EXPR_TUPLE(self));
    Ctx__s_emit_top(ctx());
    Ctx__emit_(ctx(), OP_GET_ITER, BC_NOARG, kw_line);
    int block = Ctx__enter_block(ctx(), CodeBlockType_FOR_LOOP);
    int block_start = Ctx__emit_(ctx(), OP_FOR_ITER_YIELD_VALUE, block, kw_line);
    Ctx__emit_jump(ctx(), block_start, BC_KEEPLINE);
    Ctx__exit_block(ctx());
    // StopIteration.value will be pushed onto the stack
    return NULL;
}

Error* try_compile_assignment(Compiler* self, bool* is_assign) {
    Error* err;
    switch(curr()->type) {
        case TK_IADD:
        case TK_ISUB:
        case TK_IMUL:
        case TK_IDIV:
        case TK_IFLOORDIV:
        case TK_IMOD:
        case TK_ILSHIFT:
        case TK_IRSHIFT:
        case TK_IAND:
        case TK_IOR:
        case TK_IXOR: {
            if(Ctx__s_top(ctx())->vt->is_starred)
                return SyntaxError(self, "can't use inplace operator with starred expression");
            if(ctx()->is_compiling_class)
                return SyntaxError(self, "can't use inplace operator in class definition");
            advance();
            // a[x] += 1;   a and x should be evaluated only once
            // a.x += 1;    a should be evaluated only once
            // -1 to remove =; inplace=true
            int line = prev()->line;
            TokenIndex op = (TokenIndex)(prev()->type - 1);
            // [lhs]
            check(EXPR_TUPLE(self));  // [lhs, rhs]
            if(Ctx__s_top(ctx())->vt->is_starred)
                return SyntaxError(self, "can't use starred expression here");
            BinaryExpr* e = BinaryExpr__new(line, op, true);
            e->rhs = Ctx__s_popx(ctx());  // [lhs]
            e->lhs = Ctx__s_popx(ctx());  // []
            vtemit_((Expr*)e, ctx());
            bool ok = vtemit_istore(e->lhs, ctx());
            vtdelete((Expr*)e);
            if(!ok) return SyntaxError(self, "invalid syntax");
            *is_assign = true;
            return NULL;
        }
        case TK_ASSIGN: {
            consume(TK_ASSIGN);
            int n = 0;  // assignment count

            if(match(TK_YIELD_FROM)) {
                check(compile_yield_from(self, prev()->line));
                n = 1;
            } else {
                do {
                    check(EXPR_TUPLE(self));
                    n += 1;
                } while(match(TK_ASSIGN));

                // stack size is n+1
                Ctx__s_emit_top(ctx());
                for(int j = 1; j < n; j++)
                    Ctx__emit_(ctx(), OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
            }

            for(int j = 0; j < n; j++) {
                if(Ctx__s_top(ctx())->vt->is_starred)
                    return SyntaxError(self, "can't use starred expression here");
                Expr* e = Ctx__s_top(ctx());
                bool ok = vtemit_store(e, ctx());
                Ctx__s_pop(ctx());
                if(!ok) return SyntaxError(self, "invalid syntax");
            }
            *is_assign = true;
            return NULL;
        }
        default: *is_assign = false;
    }
    return NULL;
}

static FuncDecl_ push_f_context(Compiler* self, c11_sv name, int* out_index) {
    FuncDecl_ decl = FuncDecl__rcnew(self->src, name);
    decl->code.start_line = self->i == 0 ? 1 : prev()->line;
    decl->nested = name_scope(self) == NAME_LOCAL;
    // add_func_decl
    Ctx* top_ctx = ctx();
    c11_vector__push(FuncDecl_, &top_ctx->co->func_decls, decl);
    *out_index = top_ctx->co->func_decls.length - 1;
    // push new context
    top_ctx = c11_vector__emplace(&self->contexts);
    Ctx__ctor(top_ctx, &decl->code, decl, self->contexts.length);
    return decl;
}

static Error* read_literal(Compiler* self, py_Ref out) {
    Error* err;
    advance();
    const TokenValue* value = &prev()->value;
    bool negated = false;
    switch(prev()->type) {
        case TK_SUB:
            consume(TK_NUM);
            value = &prev()->value;
            negated = true;
        case TK_NUM: {
            if(value->index == TokenValue_I64) {
                py_newint(out, negated ? -value->_i64 : value->_i64);
            } else if(value->index == TokenValue_F64) {
                py_newfloat(out, negated ? -value->_f64 : value->_f64);
            } else {
                c11__unreachable();
            }
            return NULL;
        }
        case TK_STR: py_newstr(out, value->_str->data); return NULL;
        case TK_TRUE: py_newbool(out, true); return NULL;
        case TK_FALSE: py_newbool(out, false); return NULL;
        case TK_NONE: py_newnone(out); return NULL;
        case TK_DOTDOTDOT: py_newellipsis(out); return NULL;
        case TK_LPAREN: {
            py_TValue cpnts[4];
            int count = 0;
            while(true) {
                if(count == 4)
                    return SyntaxError(self, "default argument tuple exceeds 4 elements");
                check(read_literal(self, &cpnts[count]));
                count += 1;
                if(curr()->type == TK_RPAREN) break;
                consume(TK_COMMA);
                if(curr()->type == TK_RPAREN) break;
            }
            consume(TK_RPAREN);
            py_Ref p = py_newtuple(out, count);
            for(int i = 0; i < count; i++) {
                p[i] = cpnts[i];
            }
            return NULL;
        }
        default: {
            return SyntaxError(self, "expected a literal, got '%s'", TokenSymbols[prev()->type]);
        }
    }
}

static Error* _compile_f_args(Compiler* self, FuncDecl* decl, bool is_lambda) {
    int state = 0;  // 0 for args, 1 for *args, 2 for k=v, 3 for **kwargs
    Error* err;
    do {
        if(!is_lambda) match_newlines();
        if(state >= 3) return SyntaxError(self, "**kwargs should be the last argument");
        if(match(TK_MUL)) {
            if(state < 1)
                state = 1;
            else
                return SyntaxError(self, "*args should be placed before **kwargs");
        } else if(match(TK_POW)) {
            state = 3;
        }
        consume(TK_ID);
        py_Name name = py_namev(Token__sv(prev()));

        // check duplicate argument name
        if(FuncDecl__is_duplicated_arg(decl, name)) {
            return SyntaxError(self, "duplicate argument name");
        }

        // eat type hints
        if(!is_lambda && match(TK_COLON)) check(consume_type_hints(self));
        if(state == 0 && curr()->type == TK_ASSIGN) state = 2;
        switch(state) {
            case 0: FuncDecl__add_arg(decl, name); break;
            case 1:
                FuncDecl__add_starred_arg(decl, name);
                state += 1;
                break;
            case 2: {
                consume(TK_ASSIGN);
                py_TValue value;
                check(read_literal(self, &value));
                FuncDecl__add_kwarg(decl, name, &value);
            } break;
            case 3:
                FuncDecl__add_starred_kwarg(decl, name);
                state += 1;
                break;
        }
    } while(match(TK_COMMA));
    if(!is_lambda) match_newlines();
    return NULL;
}

static Error* consume_pep695_py312(Compiler* self) {
    // https://peps.python.org/pep-0695/
    Error* err;
    if(match(TK_LBRACKET)) {
        do {
            consume(TK_ID);
            if(match(TK_COLON)) check(consume_type_hints(self));
        } while(match(TK_COMMA));
        consume(TK_RBRACKET);
    }
    return NULL;
}

static Error* compile_function(Compiler* self, int decorators) {
    Error* err;
    int def_line = prev()->line;
    consume(TK_ID);
    c11_sv decl_name_sv = Token__sv(prev());
    int decl_index;
    FuncDecl_ decl = push_f_context(self, decl_name_sv, &decl_index);
    consume_pep695_py312(self);
    consume(TK_LPAREN);
    if(!match(TK_RPAREN)) {
        check(_compile_f_args(self, decl, false));
        consume(TK_RPAREN);
    }
    if(match(TK_ARROW)) check(consume_type_hints(self));
    check(compile_block_body(self));
    check(pop_context(self));

    if(decl->code.codes.length >= 2) {
        Bytecode* codes = (Bytecode*)decl->code.codes.data;

        if(codes[0].op == OP_LOAD_CONST && codes[1].op == OP_POP_TOP) {
            // handle optional docstring
            py_TValue* consts = decl->code.consts.data;
            py_TValue* c = &consts[codes[0].arg];
            if(py_isstr(c)) {
                decl->docstring = py_tostr(c);
                codes[0].op = OP_NO_OP;
                codes[1].op = OP_NO_OP;
            }
        }
    }

    Ctx__emit_(ctx(), OP_LOAD_FUNCTION, decl_index, def_line);
    Ctx__s_emit_decorators(ctx(), decorators);

    py_Name decl_name = py_namev(decl_name_sv);
    if(ctx()->is_compiling_class) {
        if(decl_name == __new__ || decl_name == __init__) {
            if(decl->args.length == 0) {
                return SyntaxError(self,
                                   "%s() should have at least one positional argument",
                                   py_name2str(decl_name));
            }
        }

        Ctx__emit_(ctx(), OP_STORE_CLASS_ATTR, Ctx__add_name(ctx(), decl_name), def_line);
    } else {
        NameExpr* e = NameExpr__new(def_line, decl_name, name_scope(self));
        vtemit_store((Expr*)e, ctx());
        vtdelete((Expr*)e);
    }
    return NULL;
}

static Error* compile_class(Compiler* self, int decorators) {
    Error* err;
    if(ctx()->level > 1) return SyntaxError(self, "class definition not allowed here");
    consume(TK_ID);
    py_Name name = py_namev(Token__sv(prev()));
    bool has_base = false;
    consume_pep695_py312(self);
    if(match(TK_LPAREN)) {
        if(is_expression(self, false)) {
            check(EXPR(self));
            has_base = true;  // [base]
        }
        consume(TK_RPAREN);
    }
    if(!has_base) {
        Ctx__emit_(ctx(), OP_LOAD_NONE, BC_NOARG, prev()->line);
    } else {
        Ctx__s_emit_top(ctx());  // []
    }
    Ctx__emit_(ctx(), OP_BEGIN_CLASS, Ctx__add_name(ctx(), name), BC_KEEPLINE);

    c11__foreach(Ctx, &self->contexts, it) {
        if(it->is_compiling_class) return SyntaxError(self, "nested class is not allowed");
    }
    ctx()->is_compiling_class = true;
    check(compile_block_body(self));
    ctx()->is_compiling_class = false;

    Ctx__s_emit_decorators(ctx(), decorators);
    Ctx__emit_(ctx(), OP_END_CLASS, Ctx__add_name(ctx(), name), BC_KEEPLINE);
    return NULL;
}

static Error* compile_decorated(Compiler* self) {
    Error* err;
    int count = 0;
    do {
        check(EXPR(self));
        count += 1;
        if(!match_newlines()) return SyntaxError(self, "expected a newline after '@'");
    } while(match(TK_DECORATOR));

    if(match(TK_CLASS)) {
        check(compile_class(self, count));
    } else {
        consume(TK_DEF);
        check(compile_function(self, count));
    }
    return NULL;
}

// import a [as b]
// import a [as b], c [as d]
static Error* compile_normal_import(Compiler* self) {
    do {
        consume(TK_ID);
        c11_sv name = Token__sv(prev());
        int index = Ctx__add_const_string(ctx(), name);
        Ctx__emit_(ctx(), OP_IMPORT_PATH, index, prev()->line);
        if(match(TK_AS)) {
            consume(TK_ID);
            name = Token__sv(prev());
        }
        Ctx__emit_store_name(ctx(), name_scope(self), py_namev(name), prev()->line);
    } while(match(TK_COMMA));
    consume_end_stmt();
    return NULL;
}

// from a import b [as c], d [as e]
// from a.b import c [as d]
// from . import a [as b]
// from .a import b [as c]
// from ..a import b [as c]
// from .a.b import c [as d]
// from xxx import *
static Error* compile_from_import(c11_sbuf* buf, Compiler* self) {
    int dots = 0;

    while(true) {
        switch(curr()->type) {
            case TK_DOT: dots += 1; break;
            case TK_DOTDOT: dots += 2; break;
            case TK_DOTDOTDOT: dots += 3; break;
            default: goto __EAT_DOTS_END;
        }
        advance();
    }
__EAT_DOTS_END:
    for(int i = 0; i < dots; i++) {
        c11_sbuf__write_char(buf, '.');
    }

    if(dots > 0) {
        // @id is optional if dots > 0
        if(match(TK_ID)) {
            c11_sbuf__write_sv(buf, Token__sv(prev()));
            while(match(TK_DOT)) {
                consume(TK_ID);
                c11_sbuf__write_char(buf, '.');
                c11_sbuf__write_sv(buf, Token__sv(prev()));
            }
        }
    } else {
        // @id is required if dots == 0
        consume(TK_ID);
        c11_sbuf__write_sv(buf, Token__sv(prev()));
        while(match(TK_DOT)) {
            consume(TK_ID);
            c11_sbuf__write_char(buf, '.');
            c11_sbuf__write_sv(buf, Token__sv(prev()));
        }
    }

    c11_string* path = c11_sbuf__submit(buf);
    Ctx__emit_(ctx(),
               OP_IMPORT_PATH,
               Ctx__add_const_string(ctx(), c11_string__sv(path)),
               prev()->line);
    c11_string__delete(path);
    consume(TK_IMPORT);

    if(match(TK_MUL)) {
        if(name_scope(self) != NAME_GLOBAL)
            return SyntaxError(self, "from <module> import * can only be used in global scope");
        // pop the module and import __all__
        Ctx__emit_(ctx(), OP_POP_IMPORT_STAR, BC_NOARG, prev()->line);
        consume_end_stmt();
        return NULL;
    }

    bool has_bracket = match(TK_LPAREN);
    do {
        if(has_bracket) match_newlines();
        Ctx__emit_(ctx(), OP_DUP_TOP, BC_NOARG, BC_KEEPLINE);
        consume(TK_ID);
        c11_sv name = Token__sv(prev());
        Ctx__emit_(ctx(), OP_LOAD_ATTR, Ctx__add_name(ctx(), py_namev(name)), prev()->line);
        if(match(TK_AS)) {
            consume(TK_ID);
            name = Token__sv(prev());
        }
        Ctx__emit_store_name(ctx(), name_scope(self), py_namev(name), prev()->line);
    } while(match(TK_COMMA));
    if(has_bracket) {
        match_newlines();
        consume(TK_RPAREN);
    }
    Ctx__emit_(ctx(), OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
    consume_end_stmt();
    return NULL;
}

static Error* compile_try_except(Compiler* self) {
    Error* err;
    int patches[8];
    int patches_length = 0;

    Ctx__enter_block(ctx(), CodeBlockType_TRY);
    Ctx__emit_(ctx(), OP_BEGIN_TRY, BC_NOARG, prev()->line);
    check(compile_block_body(self));
    Ctx__emit_(ctx(), OP_END_TRY, BC_NOARG, BC_KEEPLINE);

    // https://docs.python.org/3/reference/compound_stmts.html#finally-clause
    /* If finally is present, it specifies a ‘cleanup’ handler. The try clause is executed,
     * including any except and else clauses. If an exception occurs in any of the clauses and is
     * not handled, the exception is temporarily saved. The finally clause is executed. If there is
     * a saved exception it is re-raised at the end of the finally clause. If the finally clause
     * raises another exception, the saved exception is set as the context of the new exception. If
     * the finally clause executes a return, break or continue statement, the saved exception is
     * discarded.
     */

    // known issue:
    // A return, break, continue in try/except block will make the finally block not executed

    bool has_finally = curr()->type == TK_FINALLY;
    if(has_finally) return SyntaxError(self, "finally clause is not supported yet");

    patches[patches_length++] = Ctx__emit_(ctx(), OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE);
    Ctx__exit_block(ctx());

    do {
        if(patches_length == 8) {
            return SyntaxError(self, "maximum number of except clauses reached");
        }
        py_Name as_name = 0;
        consume(TK_EXCEPT);
        if(is_expression(self, false)) {
            // except <expr>:
            check(EXPR(self));
            Ctx__s_emit_top(ctx());
            Ctx__emit_(ctx(), OP_EXCEPTION_MATCH, BC_NOARG, prev()->line);
            if(match(TK_AS)) {
                // except <expr> as <name>:
                consume(TK_ID);
                as_name = py_namev(Token__sv(prev()));
            }
        } else {
            // except:
            Ctx__emit_(ctx(), OP_LOAD_TRUE, BC_NOARG, BC_KEEPLINE);
        }
        int patch = Ctx__emit_(ctx(), OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
        // on match
        Ctx__emit_(ctx(), OP_HANDLE_EXCEPTION, BC_NOARG, BC_KEEPLINE);
        if(as_name) {
            Ctx__emit_(ctx(), OP_PUSH_EXCEPTION, BC_NOARG, BC_KEEPLINE);
            Ctx__emit_store_name(ctx(), name_scope(self), as_name, BC_KEEPLINE);
        }
        Ctx__enter_block(ctx(), CodeBlockType_EXCEPT);
        check(compile_block_body(self));
        Ctx__exit_block(ctx());
        Ctx__emit_(ctx(), OP_END_TRY, BC_NOARG, BC_KEEPLINE);
        patches[patches_length++] = Ctx__emit_(ctx(), OP_JUMP_FORWARD, BC_NOARG, BC_KEEPLINE);
        Ctx__patch_jump(ctx(), patch);
    } while(curr()->type == TK_EXCEPT);

    // no match, re-raise
    Ctx__emit_(ctx(), OP_RE_RAISE, BC_NOARG, BC_KEEPLINE);

    // match one & handled, jump to the end
    for(int i = 0; i < patches_length; i++) {
        Ctx__patch_jump(ctx(), patches[i]);
    }

    if(match(TK_FINALLY)) return SyntaxError(self, "finally clause is not supported yet");
    return NULL;
}

static Error* compile_stmt(Compiler* self) {
    Error* err;
    if(match(TK_CLASS)) {
        check(compile_class(self, 0));
        return NULL;
    }
    advance();
    int kw_line = prev()->line;  // backup line number
    switch(prev()->type) {
        case TK_BREAK: {
            int curr_loop_block = Ctx__prepare_loop_divert(ctx(), kw_line, true);
            if(curr_loop_block < 0) return SyntaxError(self, "'break' outside loop");
            Ctx__emit_(ctx(), OP_LOOP_BREAK, curr_loop_block, kw_line);
            consume_end_stmt();
            break;
        }
        case TK_CONTINUE: {
            int curr_loop_block = Ctx__prepare_loop_divert(ctx(), kw_line, false);
            if(curr_loop_block < 0) return SyntaxError(self, "'continue' not properly in loop");
            Ctx__emit_(ctx(), OP_LOOP_CONTINUE, curr_loop_block, kw_line);
            consume_end_stmt();
            break;
        }
        case TK_YIELD:
            if(self->contexts.length <= 1) return SyntaxError(self, "'yield' outside function");
            if(match_end_stmt(self)) {
                Ctx__emit_(ctx(), OP_YIELD_VALUE, 1, kw_line);
            } else {
                check(EXPR_TUPLE(self));
                Ctx__s_emit_top(ctx());
                Ctx__emit_(ctx(), OP_YIELD_VALUE, BC_NOARG, kw_line);
                consume_end_stmt();
            }
            break;
        case TK_YIELD_FROM:
            check(compile_yield_from(self, kw_line));
            Ctx__emit_(ctx(), OP_POP_TOP, BC_NOARG, kw_line);
            consume_end_stmt();
            break;
        case TK_RETURN:
            if(self->contexts.length <= 1) return SyntaxError(self, "'return' outside function");
            if(match_end_stmt(self)) {
                Ctx__emit_(ctx(), OP_RETURN_VALUE, 1, kw_line);
            } else {
                check(EXPR_TUPLE(self));
                Ctx__s_emit_top(ctx());
                consume_end_stmt();
                Ctx__emit_(ctx(), OP_RETURN_VALUE, BC_NOARG, kw_line);
            }
            break;
        /*************************************************/
        case TK_IF: check(compile_if_stmt(self)); break;
        case TK_MATCH: {
            c11_vector patches;
            c11_vector__ctor(&patches, sizeof(int));
            check(compile_match_case(self, &patches));
            c11_vector__dtor(&patches);
            break;
        }
        case TK_WHILE: check(compile_while_loop(self)); break;
        case TK_FOR: check(compile_for_loop(self)); break;
        case TK_IMPORT: check(compile_normal_import(self)); break;
        case TK_FROM: {
            c11_sbuf buf;
            c11_sbuf__ctor(&buf);
            err = compile_from_import(&buf, self);
            c11_sbuf__dtor(&buf);
            if(err) return err;
            break;
        }
        case TK_DEF: check(compile_function(self, 0)); break;
        case TK_DECORATOR: check(compile_decorated(self)); break;
        case TK_TRY: check(compile_try_except(self)); break;
        case TK_PASS: consume_end_stmt(); break;
        /*************************************************/
        case TK_ASSERT: {
            check(EXPR(self));  // condition
            Ctx__s_emit_top(ctx());
            int index = Ctx__emit_(ctx(), OP_POP_JUMP_IF_TRUE, BC_NOARG, kw_line);
            int has_msg = 0;
            if(match(TK_COMMA)) {
                check(EXPR(self));  // message
                Ctx__s_emit_top(ctx());
                has_msg = 1;
            }
            Ctx__emit_(ctx(), OP_RAISE_ASSERT, has_msg, kw_line);
            Ctx__patch_jump(ctx(), index);
            consume_end_stmt();
            break;
        }
        case TK_GLOBAL:
            do {
                consume(TK_ID);
                py_Name name = py_namev(Token__sv(prev()));
                c11_smallmap_n2d__set(&ctx()->global_names, name, 0);
            } while(match(TK_COMMA));
            consume_end_stmt();
            break;
        case TK_RAISE: {
            if(is_expression(self, false)) {
                check(EXPR(self));
                Ctx__s_emit_top(ctx());
                Ctx__emit_(ctx(), OP_RAISE, BC_NOARG, kw_line);
            } else {
                int iblock = ctx()->curr_iblock;
                CodeBlock* blocks = (CodeBlock*)ctx()->co->blocks.data;
                if(blocks[iblock].type != CodeBlockType_EXCEPT) {
                    return SyntaxError(self,
                                       "raise without exception is only allowed in except block");
                }
                Ctx__emit_(ctx(), OP_RE_RAISE, BC_NOARG, kw_line);
            }
            consume_end_stmt();
        } break;
        case TK_DEL: {
            check(EXPR_TUPLE(self));
            Expr* e = Ctx__s_top(ctx());
            if(!vtemit_del(e, ctx())) return SyntaxError(self, "invalid syntax");
            Ctx__s_pop(ctx());
            consume_end_stmt();
        } break;
        case TK_WITH: {
            check(EXPR(self));  // [ <expr> ]
            Ctx__s_emit_top(ctx());
            Ctx__enter_block(ctx(), CodeBlockType_WITH);
            NameExpr* as_name = NULL;
            if(match(TK_AS)) {
                consume(TK_ID);
                py_Name name = py_namev(Token__sv(prev()));
                as_name = NameExpr__new(prev()->line, name, name_scope(self));
            }
            Ctx__emit_(ctx(), OP_WITH_ENTER, BC_NOARG, prev()->line);
            // [ <expr> <expr>.__enter__() ]
            if(as_name) {
                bool ok = vtemit_store((Expr*)as_name, ctx());
                vtdelete((Expr*)as_name);
                if(!ok) return SyntaxError(self, "invalid syntax");
            } else {
                // discard `__enter__()`'s return value
                Ctx__emit_(ctx(), OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
            }
            check(compile_block_body(self));
            Ctx__emit_(ctx(), OP_WITH_EXIT, BC_NOARG, prev()->line);
            Ctx__exit_block(ctx());
        } break;
        /*************************************************/
        // handle dangling expression or assignment
        default: {
            // do revert since we have pre-called advance() at the beginning
            --self->i;

            check(EXPR_TUPLE(self));

            bool is_typed_name = false;  // e.g. x: int
            // eat variable's type hint if it is a single name
            const ExprVt* top_vt = Ctx__s_top(ctx())->vt;
            if(top_vt->is_name || top_vt->is_attrib) {
                if(match(TK_COLON)) {
                    c11_sv type_hint;
                    check(consume_type_hints_sv(self, &type_hint));
                    is_typed_name = true;

                    if(ctx()->is_compiling_class && top_vt->is_name) {
                        NameExpr* ne = (NameExpr*)Ctx__s_top(ctx());
                        int index = Ctx__add_const_string(ctx(), type_hint);
                        Ctx__emit_(ctx(), OP_LOAD_CONST, index, BC_KEEPLINE);
                        Ctx__emit_(ctx(),
                                   OP_ADD_CLASS_ANNOTATION,
                                   Ctx__add_name(ctx(), ne->name),
                                   BC_KEEPLINE);
                    }
                }
            }
            bool is_assign = false;
            check(try_compile_assignment(self, &is_assign));
            if(!is_assign) {
                if(Ctx__s_size(ctx()) > 0 && Ctx__s_top(ctx())->vt->is_starred) {
                    return SyntaxError(self, "can't use starred expression here");
                }
                if(!is_typed_name) {
                    Ctx__s_emit_top(ctx());
                    if((mode() == SINGLE_MODE) && name_scope(self) == NAME_GLOBAL) {
                        Ctx__emit_(ctx(), OP_PRINT_EXPR, BC_NOARG, BC_KEEPLINE);
                    } else {
                        Ctx__emit_(ctx(), OP_POP_TOP, BC_NOARG, BC_KEEPLINE);
                    }
                } else {
                    Ctx__s_pop(ctx());
                }
            }
            consume_end_stmt();
            break;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////

Error* Compiler__compile(Compiler* self, CodeObject* out) {
    // make sure it is the first time to compile
    assert(self->i == 0);
    // make sure the first token is @sof
    assert(tk(0)->type == TK_SOF);

    push_global_context(self, out);

    advance();         // skip @sof, so prev() is always valid
    match_newlines();  // skip possible leading '\n'

    Error* err;
    if(mode() == EVAL_MODE) {
        check(EXPR_TUPLE(self));
        Ctx__s_emit_top(ctx());
        consume(TK_EOF);
        Ctx__emit_(ctx(), OP_RETURN_VALUE, BC_NOARG, BC_KEEPLINE);
        check(pop_context(self));
        return NULL;
    }

    while(!match(TK_EOF)) {
        check(compile_stmt(self));
        match_newlines();
    }
    check(pop_context(self));
    return NULL;
}

Error* pk_compile(SourceData_ src, CodeObject* out) {
    Token* tokens;
    int tokens_length;
    Error* err = Lexer__process(src, &tokens, &tokens_length);
    if(err) return err;

#if 0
    Token* data = (Token*)tokens.data;
    printf("%s\n", src->filename->data);
    for(int i = 0; i < tokens.length; i++) {
        Token* t = data + i;
        c11_string* tmp = c11_string__new2(t->start, t->length);
        if(t->value.index == TokenValue_STR) {
            const char* value_str = t->value._str->data;
            printf("[%d] %s: %s (value._str=%s)\n",
                   t->line,
                   TokenSymbols[t->type],
                   tmp->data,
                   value_str);
        } else {
            printf("[%d] %s: %s\n", t->line, TokenSymbols[t->type], tmp->data);
        }
        c11_string__delete(tmp);
    }
#endif

    Compiler compiler;
    Compiler__ctor(&compiler, src, tokens, tokens_length);
    CodeObject__ctor(out, src, c11_string__sv(src->filename));
    err = Compiler__compile(&compiler, out);
    if(err) {
        // dispose the code object if error occurs
        CodeObject__dtor(out);
    }
    Compiler__dtor(&compiler);
    return err;
}

// clang-format off
const static PrattRule rules[TK__COUNT__] = {
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
    [TK_FSTR_BEGIN] =  { exprFString, },
    [TK_IMAG] =        { exprImag,    },
    [TK_BYTES] =       { exprBytes,   },
    [TK_LBRACE] =      { exprMap      },
    [TK_COLON] =       { exprSlice0,    exprSlice1,      PREC_PRIMARY }
};
// clang-format on

#undef vtcall
#undef vtemit_
#undef vtemit_del
#undef vtemit_store
#undef vtemit_inplace
#undef vtemit_istore
#undef vtdelete
#undef EXPR_COMMON_HEADER
#undef is_compare_expr
#undef tk
#undef prev
#undef curr
#undef next
#undef advance
#undef mode
#undef ctx
#undef match_newlines
#undef consume
#undef consume_end_stmt
#undef check
#undef match