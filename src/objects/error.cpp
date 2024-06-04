#include "pocketpy/objects/error.hpp"

namespace pkpy {
Str Exception::summary() const {
    SStream ss;
    if(is_re) ss << "Traceback (most recent call last):\n";
    // while(!st.empty()) {
    //     ss << st.top().snapshot() << '\n';
    //     st.pop();
    // }
    const auto& container = stacktrace.container();
    for(int i = container.size() - 1; i >= 0; i--) {
        ss << container[i].snapshot() << '\n';
    }
    if(!msg.empty())
        ss << type.sv() << ": " << msg;
    else
        ss << type.sv();
    return ss.str();
}

}  // namespace pkpy
