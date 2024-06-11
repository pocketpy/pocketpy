#include "pocketpy/objects/codeobject.hpp"

namespace pkpy {

CodeObject::CodeObject(std::shared_ptr<SourceData> src, const Str& name) :
    src(src), name(name), nlocals(0), start_line(-1), end_line(-1) {
    blocks.push_back(CodeBlock(CodeBlockType::NO_BLOCK, -1, 0));
}
}  // namespace pkpy
