#pragma once

#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/strname.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_CodeEmitContext{
    CodeObject* co;  // 1 CodeEmitContext <=> 1 CodeObject*
    FuncDecl* func;  // optional, weakref
    int level;
    int curr_iblock;
    bool is_compiling_class;
    c11_vector/*T=Expr* */ s_expr;
    c11_vector/*T=StrName*/ global_names;
    c11_smallmap_s2n co_consts_string_dedup_map;
} pk_CodeEmitContext;

typedef struct pk_Expr pk_Expr;

void pk_CodeEmitContext__ctor(pk_CodeEmitContext* self, CodeObject* co, FuncDecl* func, int level);
void pk_CodeEmitContext__dtor(pk_CodeEmitContext* self);

int pk_CodeEmitContext__get_loop(pk_CodeEmitContext* self);
CodeBlock* pk_CodeEmitContext__enter_block(pk_CodeEmitContext* self, CodeBlockType type);
void pk_CodeEmitContext__exit_block(pk_CodeEmitContext* self);
int pk_CodeEmitContext__emit_(pk_CodeEmitContext* self, Opcode opcode, uint16_t arg, int line);
int pk_CodeEmitContext__emit_virtual(pk_CodeEmitContext* self, Opcode opcode, uint16_t arg, int line);
void pk_CodeEmitContext__revert_last_emit_(pk_CodeEmitContext* self);
int pk_CodeEmitContext__emit_int(pk_CodeEmitContext* self, int64_t value, int line);
void pk_CodeEmitContext__patch_jump(pk_CodeEmitContext* self, int index);
bool pk_CodeEmitContext__add_label(pk_CodeEmitContext* self, StrName name);
int pk_CodeEmitContext__add_varname(pk_CodeEmitContext* self, StrName name);
int pk_CodeEmitContext__add_const(pk_CodeEmitContext* self, py_Ref);
int pk_CodeEmitContext__add_const_string(pk_CodeEmitContext* self, c11_string);
void pk_CodeEmitContext__emit_store_name(pk_CodeEmitContext* self, NameScope scope, StrName name, int line);
void pk_CodeEmitContext__try_merge_for_iter_store(pk_CodeEmitContext* self, int);

// emit top -> pop -> delete
void pk_CodeEmitContext__s_emit_top(pk_CodeEmitContext*);
// push
void pk_CodeEmitContext__s_push(pk_CodeEmitContext*, pk_Expr*);
// top
pk_Expr* pk_CodeEmitContext__s_top(pk_CodeEmitContext*);
// size
int pk_CodeEmitContext__s_size(pk_CodeEmitContext*);
// pop -> delete
void pk_CodeEmitContext__s_pop(pk_CodeEmitContext*);
// pop move
pk_Expr* pk_CodeEmitContext__s_popx(pk_CodeEmitContext*);
// clean
void pk_CodeEmitContext__s_clean(pk_CodeEmitContext*);
// emit decorators
void pk_CodeEmitContext__s_emit_decorators(pk_CodeEmitContext*, int count);

#ifdef __cplusplus
}
#endif