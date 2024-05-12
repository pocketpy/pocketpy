#include "pocketpy/codeobject.h"

namespace pkpy{

    CodeObject::CodeObject(std::shared_ptr<SourceData> src, const Str& name):
        src(src), name(name), start_line(-1), end_line(-1) {
            blocks.push_back(CodeBlock(CodeBlockType::NO_BLOCK, -1, 0, 0));
        }

    void CodeObject::_gc_mark() const {
        for(PyVar v : consts) PK_OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }

    struct PySignalObject: PyObject {
        PySignalObject() : PyObject(Type(0)) { gc_enabled = false; }
        void _obj_gc_mark() override {}
    };

    PyVar const PY_NULL = new PySignalObject();
    PyVar const PY_OP_CALL = new PySignalObject();
    PyVar const PY_OP_YIELD = new PySignalObject();
}   // namespace pkpy