#include "pocketpy/interpreter/vm.h"
#include <assert.h>

py_ItemRef pk_tpfindname(py_TypeInfo* ti, py_Name name) {
    assert(ti != NULL);
    do {
        py_Ref res = py_getdict(&ti->self, name);
        if(res) return res;
        ti = ti->base_ti;
    } while(ti);
    return NULL;
}

PK_INLINE py_TypeInfo* pk_typeinfo(py_Type type) {
#ifndef NDEBUG
    int length = pk_current_vm->types.length;
    if(type <= 0 || type >= length) {
        c11__abort("type index %d is out of bounds (0, %d)", type, length);
    }
#endif
    return c11__getitem(TypePointer, &pk_current_vm->types, type).ti;
}

static void py_TypeInfo__common_init(py_Name name,
                                     py_Type base,
                                     py_Type index,
                                     const py_GlobalRef module,
                                     void (*dtor)(void*),
                                     bool is_python,
                                     bool is_final,
                                     py_TypeInfo* self,
                                     py_TValue* typeobject) {
    py_TypeInfo* base_ti = base ? pk_typeinfo(base) : NULL;
    if(base_ti && base_ti->is_final) {
        c11__abort("type '%s' is not an acceptable base type", py_name2str(base_ti->name));
    }

    self->name = name;
    self->index = index;
    self->base = base;
    self->base_ti = base_ti;

    py_assign(&self->self, typeobject);
    self->module = module ? module : py_NIL();

    if(!dtor && base) dtor = base_ti->dtor;
    self->is_python = is_python;
    self->is_final = is_final;

    self->getattribute = NULL;
    self->setattribute = NULL;
    self->delattribute = NULL;
    self->getunboundmethod = NULL;

    self->annotations = *py_NIL();
    self->dtor = dtor;
    self->on_end_subclass = NULL;
}

py_Type pk_newtype(const char* name,
                   py_Type base,
                   const py_GlobalRef module,
                   void (*dtor)(void*),
                   bool is_python,
                   bool is_final) {
    py_Type index = pk_current_vm->types.length;
    py_TypeInfo* self = py_newobject(py_retval(), tp_type, -1, sizeof(py_TypeInfo));
    py_TypeInfo__common_init(py_name(name),
                             base,
                             index,
                             module,
                             dtor,
                             is_python,
                             is_final,
                             self,
                             py_retval());
    TypePointer* pointer = c11_vector__emplace(&pk_current_vm->types);
    pointer->ti = self;
    pointer->dtor = self->dtor;
    return index;
}

py_Type pk_newtypewithmode(py_Name name,
                           py_Type base,
                           const py_GlobalRef module,
                           void (*dtor)(void*),
                           bool is_python,
                           bool is_final,
                           enum py_CompileMode mode) {
    if(mode == RELOAD_MODE && module != NULL) {
        py_ItemRef old_class = py_getdict(module, name);
        if(old_class != NULL && py_istype(old_class, tp_type)) {
#ifndef NDEBUG
            const char* name_cstr = py_name2str(name);
            (void)name_cstr;  // avoid unused warning
#endif
            py_cleardict(old_class);
            py_TypeInfo* self = py_touserdata(old_class);
            py_Type index = self->index;
            py_TypeInfo__common_init(name,
                                     base,
                                     index,
                                     module,
                                     dtor,
                                     is_python,
                                     is_final,
                                     self,
                                     &self->self);
            TypePointer* pointer = c11__at(TypePointer, &pk_current_vm->types, index);
            pointer->ti = self;
            pointer->dtor = self->dtor;
            return index;
        }
    }

    return pk_newtype(py_name2str(name), base, module, dtor, is_python, is_final);
}

