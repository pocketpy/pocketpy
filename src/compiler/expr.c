// #include "pocketpy/compiler/expr.h"
// #include "pocketpy/common/memorypool.h"

// static bool default_false(const pk_Expr*) { return false; }
// static int default_zero(const pk_Expr*) { return 0; }
// static void default_dtor(pk_Expr*) {}

// void pk_ExprVt__ctor(pk_ExprVt* vt){
//     vt->dtor = default_dtor;
//     vt->is_literal = default_false;
//     vt->is_json_object = default_false;
//     vt->is_attrib = default_false;
//     vt->is_subscr = default_false;
//     vt->is_compare = default_false;
//     vt->star_level = default_zero;
//     vt->is_tuple = default_false;
//     vt->is_name = default_false;
//     vt->emit_ = NULL;   // must be set
//     vt->emit_del = NULL;
//     vt->emit_store = NULL;
//     vt->emit_inplace = NULL;
//     vt->emit_store_inplace = NULL;
// }

// void pk_Expr__emit_(pk_Expr* self, pk_CodeEmitContext* ctx){
//     assert(self->vt->emit_);
//     self->vt->emit_(self, ctx);
// }

// bool pk_Expr__emit_del(pk_Expr* self, pk_CodeEmitContext* ctx){
//     if(!self->vt->emit_del) return false;
//     return self->vt->emit_del(self, ctx);
// }

// bool pk_Expr__emit_store(pk_Expr* self, pk_CodeEmitContext* ctx){
//     if(!self->vt->emit_store) return false;
//     return self->vt->emit_store(self, ctx);
// }

// void pk_Expr__emit_inplace(pk_Expr* self, pk_CodeEmitContext* ctx){
//     if(!self->vt->emit_inplace){
//         pk_Expr__emit_(self, ctx);
//         return;
//     }
//     self->vt->emit_inplace(self, ctx);
// }

// bool pk_Expr__emit_store_inplace(pk_Expr* self, pk_CodeEmitContext* ctx){
//     if(!self->vt->emit_store_inplace){
//         return pk_Expr__emit_store(self, ctx);
//     }
//     return self->vt->emit_store_inplace(self, ctx);
// }

// void pk_Expr__delete(pk_Expr* self){
//     if(!self) return;
//     self->vt->dtor(self);
//     PoolExpr_dealloc(self);
// }