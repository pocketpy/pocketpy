#include "pocketpy/objects/error.hpp"

namespace pkpy {
Str Exception::summary() const {
    SStream ss;
    if(is_re) ss << "Traceback (most recent call last):\n";
    for(int i = stacktrace.size() - 1; i >= 0; i--) {
        ss << stacktrace[i].snapshot() << '\n';
    }
    if(!msg.empty())
        ss << type.sv() << ": " << msg;
    else
        ss << type.sv();
    return ss.str();
}

}  // namespace pkpy
