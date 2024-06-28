#pragma once

#include <stdbool.h>
#include "pocketpy/common/memorypool.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/codeobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_Expr pk_Expr;
typedef struct pk_CodeEmitContext pk_CodeEmitContext;

typedef struct pk_ExprVt{
    void (*dtor)(pk_Expr*);
    /* reflections */
    bool (*is_literal)(const pk_Expr*);
    bool (*is_json_object)(const pk_Expr*);
    bool (*is_attrib)(const pk_Expr*);
    bool (*is_subscr)(const pk_Expr*);
    bool (*is_compare)(const pk_Expr*);
    int (*star_level)(const pk_Expr*);
    bool (*is_tuple)(const pk_Expr*);
    bool (*is_name)(const pk_Expr*);
    /* emit */
    void (*emit_)(pk_Expr*, pk_CodeEmitContext*);
    bool (*emit_del)(pk_Expr*, pk_CodeEmitContext*);
    bool (*emit_store)(pk_Expr*, pk_CodeEmitContext*);
    void (*emit_inplace)(pk_Expr*, pk_CodeEmitContext*);
    bool (*emit_store_inplace)(pk_Expr*, pk_CodeEmitContext*);
} pk_ExprVt;

typedef struct pk_Expr{
    pk_ExprVt* vt;
    int line;
} pk_Expr;

void pk_ExprVt__ctor(pk_ExprVt* vt);
void pk_Expr__emit_(pk_Expr* self, pk_CodeEmitContext* ctx);
bool pk_Expr__emit_del(pk_Expr* self, pk_CodeEmitContext* ctx);
bool pk_Expr__emit_store(pk_Expr* self, pk_CodeEmitContext* ctx);
void pk_Expr__emit_inplace(pk_Expr* self, pk_CodeEmitContext* ctx);
bool pk_Expr__emit_store_inplace(pk_Expr* self, pk_CodeEmitContext* ctx);
void pk_Expr__delete(pk_Expr* self);

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

void pk_CodeEmitContext__ctor(pk_CodeEmitContext* self, CodeObject* co, FuncDecl* func, int level);
void pk_CodeEmitContext__dtor(pk_CodeEmitContext* self);

#ifdef __cplusplus
}
#endif
