// #pragma once

// #include <stdbool.h>
// #include "pocketpy/common/memorypool.h"
// #include "pocketpy/compiler/lexer.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// struct pk_Expr;
// struct pk_CodeEmitContext;

// struct pk_ExprVt{
//     void (*dtor)(pk_Expr*);
//     /* reflections */
//     bool (*is_literal)(const pk_Expr*);
//     bool (*is_json_object)(const pk_Expr*);
//     bool (*is_attrib)(const pk_Expr*);
//     bool (*is_subscr)(const pk_Expr*);
//     bool (*is_compare)(const pk_Expr*);
//     int (*star_level)(const pk_Expr*);
//     bool (*is_tuple)(const pk_Expr*);
//     bool (*is_name)(const pk_Expr*);
//     /* emit */
//     void (*emit_)(pk_Expr*, pk_CodeEmitContext*);
//     bool (*emit_del)(pk_Expr*, pk_CodeEmitContext*);
//     bool (*emit_store)(pk_Expr*, pk_CodeEmitContext*);
//     void (*emit_inplace)(pk_Expr*, pk_CodeEmitContext*);
//     bool (*emit_store_inplace)(pk_Expr*, pk_CodeEmitContext*);
// };

// typedef struct pk_Expr{
//     pk_ExprVt* vt;
//     int line;
// } pk_Expr;

// void pk_ExprVt__ctor(pk_ExprVt* vt);
// void pk_Expr__emit_(pk_Expr* self, pk_CodeEmitContext* ctx);
// bool pk_Expr__emit_del(pk_Expr* self, pk_CodeEmitContext* ctx);
// bool pk_Expr__emit_store(pk_Expr* self, pk_CodeEmitContext* ctx);
// void pk_Expr__emit_inplace(pk_Expr* self, pk_CodeEmitContext* ctx);
// bool pk_Expr__emit_store_inplace(pk_Expr* self, pk_CodeEmitContext* ctx);
// void pk_Expr__delete(pk_Expr* self);

// typedef struct pk_CodeEmitContext{

// } pk_CodeEmitContext;

// #ifdef __cplusplus
// }
// #endif
