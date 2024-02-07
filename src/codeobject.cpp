#include "pocketpy/codeobject.h"

namespace pkpy{

    CodeObject::CodeObject(std::shared_ptr<SourceData> src, const Str& name):
        src(src), name(name), start_line(-1), end_line(-1) {}

    void CodeObject::_gc_mark() const {
        for(PyObject* v : consts) PK_OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }

    NativeFunc::NativeFunc(NativeFuncC f, int argc, bool method){
        this->f = f;
        this->argc = argc;
        if(argc != -1) this->argc += (int)method;
    }

    NativeFunc::NativeFunc(NativeFuncC f, FuncDecl_ decl){
        this->f = f;
        this->argc = -1;
        this->decl = decl;
    }
}   // namespace pkpy