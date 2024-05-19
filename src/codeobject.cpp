#include "pocketpy/codeobject.h"

namespace pkpy{

    CodeObject::CodeObject(std::shared_ptr<SourceData> src, const Str& name):
        src(src), name(name), start_line(-1), end_line(-1) {
            blocks.push_back(CodeBlock(CodeBlockType::NO_BLOCK, -1, 0, 0));
        }

    PyVar const PY_NULL(Type(), new PyObject(false));
    PyVar const PY_OP_CALL(Type(), new PyObject(false));
    PyVar const PY_OP_YIELD(Type(), new PyObject(false));
}   // namespace pkpy