#pragma once

#include "pocketpy/common/traits.hpp"

namespace pkpy{

struct StackMemory{
    int count;
    StackMemory(int count) : count(count) {}
};

template<>
inline bool constexpr is_sso_v<StackMemory> = true;

inline const int kTpStackMemoryIndex = 27;

}   // namespace pkpy