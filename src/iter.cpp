#include "pocketpy/iter.h"

namespace pkpy{

    void RangeIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<RangeIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){ return _0; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0) -> unsigned{
            RangeIter& self = PK_OBJ_GET(RangeIter, _0);
            if(self.r.step > 0){
                if(self.current >= self.r.stop) return 0;
            }else{
                if(self.current <= self.r.stop) return 0;
            }
            vm->s_data.push(VAR(self.current));
            self.current += self.r.step;
            return 1;
        });
    }

    void ArrayIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<ArrayIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){ return _0; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0) -> unsigned{
            ArrayIter& self = _CAST(ArrayIter&, _0);
            if(self.current == self.end) return 0;
            vm->s_data.push(*self.current++);
            return 1;
        });
    }

    void StringIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<StringIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){ return _0; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0) -> unsigned{
            StringIter& self = _CAST(StringIter&, _0);
            Str& s = PK_OBJ_GET(Str, self.ref);
            if(self.i == s.size) return 0;
            int start = self.i;
            int len = utf8len(s.data[self.i]);
            self.i += len;
            vm->s_data.push(VAR(s.substr(start, len)));
            return 1;
        });
    }

    PyObject* Generator::next(VM* vm){
        if(state == 2) return vm->StopIteration;
        // reset frame._sp_base
        frame._sp_base = vm->s_data._sp;
        frame._locals.a = vm->s_data._sp;
        // restore the context
        for(PyObject* obj: s_backup) vm->s_data.push(obj);
        s_backup.clear();
        vm->callstack.emplace(std::move(frame));

        PyObject* ret;
        try{
            ret = vm->_run_top_frame();
        }catch(...){
            state = 2;      // end this generator immediately when an exception is thrown
            throw;
        }
        
        if(ret == PY_OP_YIELD){
            // backup the context
            frame = std::move(vm->callstack.top());
            ret = vm->s_data.popx();
            for(PyObject* obj: frame.stack_view(&vm->s_data)) s_backup.push_back(obj);
            vm->_pop_frame();
            state = 1;
            if(ret == vm->StopIteration) state = 2;
            return ret;
        }else{
            state = 2;
            return vm->StopIteration;
        }
    }

    void Generator::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<Generator>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){ return _0; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0) -> unsigned{
            Generator& self = _CAST(Generator&, _0);
            PyObject* retval = self.next(vm);
            if(retval == vm->StopIteration) return 0;
            vm->s_data.push(retval);
            return 1;
        });
    }

    void DictItemsIter::_register(VM *vm, PyObject *mod, PyObject *type){
        PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, type)];
        info.subclass_enabled = false;
        vm->bind_notimplemented_constructor<DictItemsIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){ return _0; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0) -> unsigned{
            DictItemsIter& self = _CAST(DictItemsIter&, _0);
            Dict& d = PK_OBJ_GET(Dict, self.ref);
            if(self.i == -1) return 0;
            vm->s_data.push(d._items[self.i].first);
            vm->s_data.push(d._items[self.i].second);
            self.i = d._nodes[self.i].next;
            return 2;
        });
    }

PyObject* VM::_py_generator(Frame&& frame, ArgsView buffer){
    return vm->new_user_object<Generator>(std::move(frame), buffer);
}

}   // namespace pkpy