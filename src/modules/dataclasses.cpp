#include "pocketpy/modules/dataclasses.hpp"
#include "pocketpy/interpreter/bindings.hpp"

namespace pkpy {

static void patch__init__(VM* vm, Type cls) {
    vm->bind(vm->_t(cls), "__init__(self, *args, **kwargs)", [](VM* vm, ArgsView _view) {
        PyVar self = _view[0];
        const Tuple& args = CAST(Tuple&, _view[1]);
        const Dict& kwargs_ = CAST(Dict&, _view[2]);
        NameDict kwargs;
        kwargs_.apply([&](PyVar k, PyVar v) {
            kwargs.set(CAST(Str&, k), v);
        });

        Type cls = vm->_tp(self);
        const PyTypeInfo* cls_info = &vm->_all_types[cls];
        NameDict& cls_d = cls_info->obj->attr();
        const auto& fields = cls_info->annotated_fields;

        int i = 0;  // index into args
        for(StrName field: fields) {
            if(kwargs.contains(field)) {
                self->attr().set(field, kwargs[field]);
                kwargs.del(field);
            } else {
                if(i < args.size()) {
                    self->attr().set(field, args[i]);
                    ++i;
                } else if(cls_d.contains(field)) {  // has default value
                    self->attr().set(field, cls_d[field]);
                } else {
                    vm->TypeError(_S(cls_info->name, " missing required argument ", field.escape()));
                }
            }
        }
        if(args.size() > i) {
            vm->TypeError(
                _S(cls_info->name, " takes ", fields.size(), " positional arguments but ", args.size(), " were given"));
        }
        if(kwargs.size() > 0) {
            StrName unexpected_key = kwargs.items()[0].first;
            vm->TypeError(_S(cls_info->name, " got an unexpected keyword argument ", unexpected_key.escape()));
        }
        return vm->None;
    });
}

static void patch__repr__(VM* vm, Type cls) {
    vm->bind__repr__(cls, [](VM* vm, PyVar _0) -> Str {
        const PyTypeInfo* cls_info = &vm->_all_types[vm->_tp(_0)];
        const auto& fields = cls_info->annotated_fields;
        const NameDict& obj_d = _0->attr();
        SStream ss;
        ss << cls_info->name << "(";
        bool first = true;
        for(StrName field: fields) {
            if(first)
                first = false;
            else
                ss << ", ";
            ss << field << "=" << vm->py_repr(obj_d[field]);
        }
        ss << ")";
        return ss.str();
    });
}

static void patch__eq__(VM* vm, Type cls) {
    vm->bind__eq__(cls, [](VM* vm, PyVar _0, PyVar _1) {
        if(vm->_tp(_0) != vm->_tp(_1)) return vm->NotImplemented;
        const PyTypeInfo* cls_info = &vm->_all_types[vm->_tp(_0)];
        const auto& fields = cls_info->annotated_fields;
        for(StrName field: fields) {
            PyVar lhs = _0->attr(field);
            PyVar rhs = _1->attr(field);
            if(vm->py_ne(lhs, rhs)) return vm->False;
        }
        return vm->True;
    });
}

void add_module_dataclasses(VM* vm) {
    PyObject* mod = vm->new_module("dataclasses");

    vm->bind_func(mod, "dataclass", 1, [](VM* vm, ArgsView args) {
        vm->check_type(args[0], VM::tp_type);
        Type cls = PK_OBJ_GET(Type, args[0]);
        NameDict& cls_d = args[0]->attr();

        if(!cls_d.contains(__init__)) patch__init__(vm, cls);
        if(!cls_d.contains(__repr__)) patch__repr__(vm, cls);
        if(!cls_d.contains(__eq__)) patch__eq__(vm, cls);

        const auto& fields = vm->_all_types[cls].annotated_fields;
        bool has_default = false;
        for(StrName field: fields) {
            if(cls_d.contains(field)) {
                has_default = true;
            } else {
                if(has_default) {
                    vm->TypeError(_S("non-default argument ", field.escape(), " follows default argument"));
                }
            }
        }
        return args[0];
    });

    vm->bind_func(mod, "asdict", 1, [](VM* vm, ArgsView args) {
        const auto& fields = vm->_tp_info(args[0])->annotated_fields;
        const NameDict& obj_d = args[0]->attr();
        Dict d;
        for(StrName field: fields) {
            d.set(vm, VAR(field.sv()), obj_d[field]);
        }
        return VAR(std::move(d));
    });
}

}  // namespace pkpy
