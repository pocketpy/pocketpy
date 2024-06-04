#pragma once

#include "pocketpy/common/traits.hpp"

namespace pkpy {

struct StackMemory {
    int count;

    StackMemory(int count) : count(count) {}
};

template <>
constexpr inline bool is_sso_v<StackMemory> = true;

const inline int kTpStackMemoryIndex = 27;

}  // namespace pkpy
