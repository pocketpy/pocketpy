#include "pocketpy/iter.h"

namespace pkpy{

    void RangeIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<RangeIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            RangeIter& self = _CAST(RangeIter&, obj);
            bool has_next = self.r.step > 0 ? self.current < self.r.stop : self.current > self.r.stop;
            if(!has_next) return vm->StopIteration;
            self.current += self.r.step;
            return VAR(self.current - self.r.step);
        });
    }

    void ArrayIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<ArrayIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            ArrayIter& self = _CAST(ArrayIter&, obj);
            if(self.current == self.end) return vm->StopIteration;
            return *self.current++;
        });
    }

    void StringIter::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<StringIter>(type);
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            StringIter& self = _CAST(StringIter&, obj);
            if(self.index == self.str->size) return vm->StopIteration;
            int start = self.index;
            int len = utf8len(self.str->data[self.index]);
            self.index += len;
            return VAR(self.str->substr(start, len));
        });
    }

    PyObject* Generator::next(VM* vm){
        if(state == 2) return vm->StopIteration;
        // reset frame._sp_base
        frame._sp_base = frame._s->_sp;
        frame._locals.a = frame._s->_sp;
        // restore the context
        for(PyObject* obj: s_backup) frame._s->push(obj);
        s_backup.clear();
        vm->callstack.push(std::move(frame));

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
            ret = frame._s->popx();
            for(PyObject* obj: frame.stack_view()) s_backup.push_back(obj);
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
        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){ return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Generator& self = _CAST(Generator&, obj);
            return self.next(vm);
        });
    }

PyObject* VM::_py_generator(Frame&& frame, ArgsView buffer){
    return VAR_T(Generator, std::move(frame), buffer);
}

}   // namespace pkpy