#pragma once

#include "compiler.h"
#include "vm.h"

namespace pkpy{

std::string platform_getline(bool* eof=nullptr);

class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;
public:
    REPL(VM* vm);
    bool input(std::string line);
};

} // namespace pkpy