#pragma once

#include "obj.h"
#include "vm.h"

namespace pkpy {

struct BaseRef {
    virtual PyObject* get(VM*, Frame*) const = 0;
    virtual void set(VM*, Frame*, PyObject*) const = 0;
    virtual void del(VM*, Frame*) const = 0;
    virtual ~BaseRef() = default;
};

struct NameRef : BaseRef {
    const std::pair<StrName, NameScope> pair;
    inline StrName name() const { return pair.first; }
    inline NameScope scope() const { return pair.second; }
    NameRef(const std::pair<StrName, NameScope>& pair) : pair(pair) {}

    PyObject* get(VM* vm, Frame* frame) const{
        PyObject** val;
        val = frame->f_locals().try_get(name());
        if(val != nullptr) return *val;
        val = frame->f_closure_try_get(name());
        if(val != nullptr) return *val;
        val = frame->f_globals().try_get(name());
        if(val != nullptr) return *val;
        val = vm->builtins->attr().try_get(name());
        if(val != nullptr) return *val;
        vm->NameError(name());
        return nullptr;
    }

    void set(VM* vm, Frame* frame, PyObject* val) const{
        switch(scope()) {
            case NAME_LOCAL: frame->f_locals().set(name(), val); break;
            case NAME_GLOBAL:
                if(frame->f_locals().try_set(name(), val)) return;
                frame->f_globals().set(name(), val);
                break;
            default: UNREACHABLE();
        }
    }

    void del(VM* vm, Frame* frame) const{
        switch(scope()) {
            case NAME_LOCAL: {
                if(frame->f_locals().contains(name())){
                    frame->f_locals().erase(name());
                }else{
                    vm->NameError(name());
                }
            } break;
            case NAME_GLOBAL:
            {
                if(frame->f_locals().contains(name())){
                    frame->f_locals().erase(name());
                }else{
                    if(frame->f_globals().contains(name())){
                        frame->f_globals().erase(name());
                    }else{
                        vm->NameError(name());
                    }
                }
            } break;
            default: UNREACHABLE();
        }
    }
};

struct AttrRef : BaseRef {
    mutable PyObject* obj;
    NameRef attr;
    AttrRef(PyObject* obj, NameRef attr) : obj(obj), attr(attr) {}

    PyObject* get(VM* vm, Frame* frame) const{
        return vm->getattr(obj, attr.name());
    }

    void set(VM* vm, Frame* frame, PyObject* val) const{
        vm->setattr(obj, attr.name(), std::move(val));
    }

    void del(VM* vm, Frame* frame) const{
        if(!obj->is_attr_valid()) vm->TypeError("cannot delete attribute");
        if(!obj->attr().contains(attr.name())) vm->AttributeError(obj, attr.name());
        obj->attr().erase(attr.name());
    }
};

struct IndexRef : BaseRef {
    mutable PyObject* obj;
    PyObject* index;
    IndexRef(PyObject* obj, PyObject* index) : obj(obj), index(index) {}

    PyObject* get(VM* vm, Frame* frame) const{
        return vm->fast_call(__getitem__, Args{obj, index});
    }

    void set(VM* vm, Frame* frame, PyObject* val) const{
        Args args(3);
        args[0] = obj; args[1] = index; args[2] = std::move(val);
        vm->fast_call(__setitem__, std::move(args));
    }

    void del(VM* vm, Frame* frame) const{
        vm->fast_call(__delitem__, Args{obj, index});
    }
};

struct TupleRef : BaseRef {
    Tuple objs;
    TupleRef(Tuple&& objs) : objs(std::move(objs)) {}

    PyObject* get(VM* vm, Frame* frame) const{
        Tuple args(objs.size());
        for (int i = 0; i < objs.size(); i++) {
            args[i] = vm->PyRef_AS_C(objs[i])->get(vm, frame);
        }
        return VAR(std::move(args));
    }

    void set(VM* vm, Frame* frame, PyObject* val) const{
        val = vm->asIter(val);
        BaseIter* iter = vm->PyIter_AS_C(val);
        for(int i=0; i<objs.size(); i++){
            PyObject* x;
            if(is_type(objs[i], vm->tp_star_wrapper)){
                auto& star = _CAST(StarWrapper&, objs[i]);
                if(star.rvalue) vm->ValueError("can't use starred expression here");
                if(i != objs.size()-1) vm->ValueError("* can only be used at the end");
                auto ref = vm->PyRef_AS_C(star.obj);
                List list;
                while((x = iter->next()) != nullptr) list.push_back(x);
                ref->set(vm, frame, VAR(std::move(list)));
                return;
            }else{
                x = iter->next();
                if(x == nullptr) vm->ValueError("not enough values to unpack");
                vm->PyRef_AS_C(objs[i])->set(vm, frame, x);
            }
        }
        PyObject* x = iter->next();
        if(x != nullptr) vm->ValueError("too many values to unpack");
    }

    void del(VM* vm, Frame* frame) const{
        for(int i=0; i<objs.size(); i++) vm->PyRef_AS_C(objs[i])->del(vm, frame);
    }
};


template<typename P>
PyObject* VM::PyRef(P&& value) {
    static_assert(std::is_base_of_v<BaseRef, std::decay_t<P>>);
    return heap.gcnew<P>(tp_ref, std::forward<P>(value));
}

const BaseRef* VM::PyRef_AS_C(PyObject* obj)
{
    if(!is_type(obj, tp_ref)) TypeError("expected an l-value");
    return static_cast<const BaseRef*>(obj->value());
}

/***** Frame's Impl *****/
inline void Frame::try_deref(VM* vm, PyObject*& v){
    if(is_type(v, vm->tp_ref)) v = vm->PyRef_AS_C(v)->get(vm, this);
}

}   // namespace pkpy