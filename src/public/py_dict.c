#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

#include "pocketpy/common/vector.h"
#include "pocketpy/objects/dict.h"

py_Type pk_dict__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "dict", tp_object, NULL, false);
    pk_TypeInfo* ti = c11__at(pk_TypeInfo, &vm->types, type);
    ti->dtor = (void (*)(void*))pkpy_Dict__dtor;
    return type;
}

void py_newdict(py_Ref out) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_dict, 0, sizeof(pkpy_Dict));
    pkpy_Dict* userdata = PyObject__userdata(obj);
    pkpy_Dict__ctor(userdata);
    out->type = tp_dict;
    out->is_ptr = true;
    out->_obj = obj;
}

// dict medthods
// https://docs.python.org/3/library/stdtypes.html#dict

// TODO: list(d)

bool _py_dict__len__(int argc, py_Ref argv) {
    py_checkargc(1);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_newint(py_retval(), dict->count);
    return true;
}

void _py_dict__getitem__(int argc, py_Ref argv) {
    py_checkargc(2);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1);
    py_TValue* val = pkpy_Dict__try_get(dict, *key);
    if (val) {
        *py_retval() = *val;
        return true;
    } else {
        return KeyError();
    }
}

bool _py_dict__setitem__(int argc, py_Ref argv) {
    py_checkargc(3);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1), val = py_arg(2);
    pkpy_Dict__set(dict, *key, *val);
    py_newnone(py_retval());
    return true;
}

bool _py_dict__delitem__(int argc, py_Ref argv) {
    py_checkargc(2);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1);
    pkpy_Dict__del(dict, *key);
    py_newnone(py_retval());
    return true;
}

bool _py_dict__contains__(int argc, py_Ref argv) {
    py_checkargc(2);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1);
    py_newbool(py_retval(), pkpy_Dict__contains(dict, *key));
    return true;
}

// TODO: iter(d)

bool _py_dict__clear(int argc, py_Ref argv) {
    py_checkargc(1);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    pkpy_Dict__clear(dict);
    py_newnone(py_retval());
    return true;
}

bool _py_dict__copy(int argc, py_Ref argv) {
    py_checkargc(1);
    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref ret = py_retval();
    py_newdict(ret);
    pkpy_Dict__update(py_touserdata(ret), dict);
    return true;
}

// TODO: classmethod fromkeys

bool _py_dict__get(int argc, py_Ref argv) {
    if (argc < 2 || argc > 3)
        return TypeError();

    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1);
    py_TValue* val = pkpy_Dict__try_get(dict, *key);
    if (val) {
        *py_retval() = *val;
        return true;
    }

    if (argc == 3)
        *py_retval() = *py_arg(2);
    else
        py_newnone(py_retval());

    return true;
}

// TODO: method items

// TODO: method keys

bool _py_dict__pop(int argc, py_Ref argv) {
    if (argc < 2 || argc > 3)
        return TypeError();

    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1);
    py_TValue* val = pkpy_Dict__try_get(dict, *key);
    if (val) {
        pkpy_Dict__del(dict, *key);
        *py_retval() = *val;
        return true;
    }

    if (argc == 3) {
        *py_retval() = *py_arg(2);
        return true;
    }

    return KeyError();
}

bool _py_dict__popitem(int argc, py_Ref argv) {
    py_checkargc(1);

    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_StackRef key = py_pushtmp(), val = py_pushtmp();

    py_Ref ret = py_retval();
    if (pkpy_Dict__try_pop(dict, key, val)) {
        py_newtuple(ret, 2);
        py_setslot(ret, 0, key);
        py_setslot(ret, 1, val);
        return true;
    }

    return KeyError();
}

// https://docs.python.org/3/library/stdtypes.html#dict-views
// TODO: dict_keys, dict_values, dict_items
// TODO: dict_iterator, dict_keyiterator, dict_valueiterator

// TODO: reversed(d)

bool _py_dict__setdefault(int argc, py_Ref argv) {
    if (argc < 2 || argc > 3)
        return TypeError();

    pkpy_Dict* dict = py_touserdata(py_arg(0));
    py_Ref key = py_arg(1);
    py_TValue* val = pkpy_Dict__try_get(dict, *key);
    if (val) {
        *py_retval() = *val;
        return true;
    }

    py_Ref dft;
    if (argc == 3) {
        dft = py_arg(2);
    } else {
        dft = py_pushtmp();
        py_newnone(dft);
    }

    pkpy_Dict__set(dict, *key, *dft);
    *py_retval() = *dft;
    return true;
}

bool _py_dict__update(int argc, py_Ref argv) {
    // TODO: accept kwargs
    py_checkargc(2);

    pkpy_Dict* me = py_touserdata(py_arg(0));
    pkpy_Dict* other = py_touserdata(py_arg(1));
    pkpy_Dict__update(me, other);
    py_newnone(py_retval());
    return true;
}

// TODO: method values

bool _py_dict__or__(int argc, py_Ref argv) {
    py_checkargc(2);

    pkpy_Dict* me = py_touserdata(py_arg(0));
    pkpy_Dict* other = py_touserdata(py_arg(1));
    
    py_Ref ret = py_retval();
    py_newdict(ret);
    pkpy_Dict__update(py_touserdata(ret), me);
    pkpy_Dict__update(py_touserdata(ret), other);
    return true;
}

// TODO: method operator |=

py_Type pk_set__register() {
    // TODO: implement
}

void py_newset(py_Ref out){
    // TODO: implement
}
