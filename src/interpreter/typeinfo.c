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

PK_INLINE py_ItemRef py_tpfindname(py_Type type, py_Name name) {
    py_TypeInfo* ti = pk_typeinfo(type);
    return pk_tpfindname(ti, name);
}

PK_INLINE py_Ref py_tpfindmagic(py_Type t, py_Name name) {
    // assert(py_ismagicname(name));
    return py_tpfindname(t, name);
}

PK_INLINE py_Type py_tpbase(py_Type t) {
    assert(t);
    py_TypeInfo* ti = pk_typeinfo(t);
    return ti->base;
}

PK_DEPRECATED py_Ref py_tpgetmagic(py_Type type, py_Name name) {
    // assert(py_ismagicname(name));
    py_TypeInfo* ti = pk_typeinfo(type);
    py_Ref retval = py_getdict(&ti->self, name);
    return retval != NULL ? retval : py_NIL();
}

py_Ref py_tpobject(py_Type type) {
    assert(type);
    return &pk_typeinfo(type)->self;
}

const char* py_tpname(py_Type type) {
    if(!type) return "nil";
    py_Name name = pk_typeinfo(type)->name;
    return py_name2str(name);
}

PK_INLINE py_TypeInfo* pk_typeinfo(py_Type type) {
#ifndef NDEBUG
    int length = pk_current_vm->types.length;
    if(type < 0 || type >= length) {
        c11__abort("type index %d is out of bounds [0, %d)", type, length);
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
                                     bool is_sealed,
                                     py_TypeInfo* self,
                                     py_TValue* typeobject) {
    py_TypeInfo* base_ti = base ? pk_typeinfo(base) : NULL;
    if(base_ti && base_ti->is_sealed) {
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
    self->is_sealed = is_sealed;

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
                   bool is_sealed) {
    py_Type index = pk_current_vm->types.length;
    py_TypeInfo* self = py_newobject(py_retval(), tp_type, -1, sizeof(py_TypeInfo));
    py_TypeInfo__common_init(py_name(name),
                             base,
                             index,
                             module,
                             dtor,
                             is_python,
                             is_sealed,
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
                           bool is_sealed,
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
                                     is_sealed,
                                     self,
                                     &self->self);
            TypePointer* pointer = c11__at(TypePointer, &pk_current_vm->types, index);
            pointer->ti = self;
            pointer->dtor = self->dtor;
            return index;
        }
    }

    return pk_newtype(py_name2str(name), base, module, dtor, is_python, is_sealed);
}

py_Type py_newtype(const char* name, py_Type base, const py_GlobalRef module, void (*dtor)(void*)) {
    if(strlen(name) == 0) c11__abort("type name cannot be empty");
    py_Type type = pk_newtype(name, base, module, dtor, false, false);
    if(module) py_setdict(module, py_name(name), py_tpobject(type));
    return type;
}

void py_tphookattributes(py_Type type,
                         bool (*getattribute)(py_Ref self, py_Name name),
                         bool (*setattribute)(py_Ref self, py_Name name, py_Ref val),
                         bool (*delattribute)(py_Ref self, py_Name name),
                         bool (*getunboundmethod)(py_Ref self, py_Name name)) {
    assert(type);
    py_TypeInfo* ti = pk_typeinfo(type);
    ti->getattribute = getattribute;
    ti->setattribute = setattribute;
    ti->delattribute = delattribute;
    ti->getunboundmethod = getunboundmethod;
}
