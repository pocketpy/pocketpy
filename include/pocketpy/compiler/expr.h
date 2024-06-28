#pragma once

#include <stdbool.h>
#include "pocketpy/common/memorypool.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/common/strname.h"
#include "pocketpy/objects/codeobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pk_Expr pk_Expr;
typedef struct pk_CodeEmitContext pk_CodeEmitContext;

typedef struct pk_ExprVt{
    void (*dtor)(pk_Expr*);
    /* reflections */
    bool is_literal;
    bool is_json_object;
    bool is_name;
    bool is_tuple;
    bool is_attrib;
    bool is_subscr;
    bool (*is_compare)(const pk_Expr*);
    int (*star_level)(const pk_Expr*);
    /* emit */
    void (*emit_)(pk_Expr*, pk_CodeEmitContext*);
    bool (*emit_del)(pk_Expr*, pk_CodeEmitContext*);
    bool (*emit_store)(pk_Expr*, pk_CodeEmitContext*);
    void (*emit_inplace)(pk_Expr*, pk_CodeEmitContext*);
    bool (*emit_store_inplace)(pk_Expr*, pk_CodeEmitContext*);
} pk_ExprVt;

#define COMMON_HEADER \
    pk_ExprVt* vt; \
    int line;

typedef struct pk_Expr{
    COMMON_HEADER
} pk_Expr;

void pk_ExprVt__ctor(pk_ExprVt* vt);
void pk_Expr__delete(pk_Expr* self);

void pk_Expr__initialize();
#define pk_Expr__finalize()  // do nothing

typedef struct pk_NameExpr{
    COMMON_HEADER
    StrName name;
    NameScope scope;
} pk_NameExpr;

typedef struct pk_StarredExpr{
    COMMON_HEADER
    pk_Expr* child;
    int level;
} pk_StarredExpr;

// InvertExpr, NotExpr, AndExpr, OrExpr, NegatedExpr
// NOTE: NegatedExpr always contains a non-const child. Should not generate -1 or -0.1
typedef struct pk_UnaryExpr{
    COMMON_HEADER
    pk_Expr* child;
    Opcode opcode;
} pk_UnaryExpr;

// LongExpr, BytesExpr
typedef struct pk_RawStringExpr{
    COMMON_HEADER
    c11_string value;
    Opcode opcode;
} pk_RawStringExpr;

typedef struct pk_ImagExpr{
    COMMON_HEADER
    double value;
} pk_ImagExpr;

typedef struct pk_LiteralExpr{
    COMMON_HEADER
    const TokenValue* value;
} pk_LiteralExpr;

typedef struct pk_SliceExpr{
    COMMON_HEADER
    pk_Expr* start;
    pk_Expr* stop;
    pk_Expr* step;
} pk_SliceExpr;

// ListExpr, DictExpr, SetExpr, TupleExpr
typedef struct pk_SequenceExpr{
    COMMON_HEADER
    c11_array/*T=Expr* */ items;
    Opcode opcode;
} pk_SequenceExpr;

#ifdef __cplusplus
}
#endif
