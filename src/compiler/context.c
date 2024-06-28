#include "pocketpy/compiler/context.h"
#include "pocketpy/compiler/expr.h"

void pk_CodeEmitContext__ctor(pk_CodeEmitContext* self, CodeObject* co, FuncDecl* func, int level){
    self->co = co;
    self->func = func;
    self->level = level;
    self->curr_iblock = 0;
    self->is_compiling_class = false;
    c11_vector__ctor(&self->s_expr, sizeof(pk_Expr*));
    c11_vector__ctor(&self->global_names, sizeof(StrName));
    c11_smallmap_s2n__ctor(&self->co_consts_string_dedup_map);
}

void pk_CodeEmitContext__dtor(pk_CodeEmitContext* self){
    c11_vector__dtor(&self->s_expr);
    c11_vector__dtor(&self->global_names);
    c11_smallmap_s2n__dtor(&self->co_consts_string_dedup_map);
}

// emit top -> pop -> delete
void pk_CodeEmitContext__s_emit_top(pk_CodeEmitContext* self) {
    pk_Expr* top = c11_vector__back(pk_Expr*, &self->s_expr);
    top->vt->emit_(top, self);
    c11_vector__pop(&self->s_expr);
    pk_Expr__delete(top);
}
// push
void pk_CodeEmitContext__s_push(pk_CodeEmitContext* self, pk_Expr* expr) {
    c11_vector__push(pk_Expr*, &self->s_expr, expr);
}
// top
pk_Expr* pk_CodeEmitContext__s_top(pk_CodeEmitContext* self){
    return c11_vector__back(pk_Expr*, &self->s_expr);
}
// size
int pk_CodeEmitContext__s_size(pk_CodeEmitContext* self) {
    return self->s_expr.count;
}
// pop -> delete
void pk_CodeEmitContext__s_pop(pk_CodeEmitContext* self){
    pk_Expr__delete(c11_vector__back(pk_Expr*, &self->s_expr));
    c11_vector__pop(&self->s_expr);
}
// pop move
pk_Expr* pk_CodeEmitContext__s_popx(pk_CodeEmitContext* self){
    pk_Expr* e = c11_vector__back(pk_Expr*, &self->s_expr);
    c11_vector__pop(&self->s_expr);
    return e;
}
// clean
void pk_CodeEmitContext__s_clean(pk_CodeEmitContext* self){
    c11_smallmap_s2n__dtor(&self->co_consts_string_dedup_map);  // ??
    for(int i=0; i<self->s_expr.count; i++){
        pk_Expr__delete(c11__getitem(pk_Expr*, &self->s_expr, i));
    }
    c11_vector__clear(&self->s_expr);
}