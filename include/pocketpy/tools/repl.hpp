#pragma once

#include "pocketpy/interpreter/vm.hpp"

namespace pkpy {

class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;

public:
    REPL(VM* vm);
    bool input(std::string line);
};

}  // namespace pkpy
