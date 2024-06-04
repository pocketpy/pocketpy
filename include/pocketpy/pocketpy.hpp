#pragma once

#include "pocketpy/common/traits.hpp"
#include "pocketpy/objects/builtins.hpp"
#include "pocketpy/interpreter/vm.hpp"
#include "pocketpy/interpreter/iter.hpp"
#include "pocketpy/interpreter/bindings.hpp"
#include "pocketpy/compiler/compiler.hpp"
#include "pocketpy/modules/linalg.hpp"
#include "pocketpy/tools/repl.hpp"

namespace pkpy {
static_assert(py_sizeof<Str> <= 64);
static_assert(py_sizeof<Mat3x3> <= 64);
static_assert(py_sizeof<Struct> <= 64);
static_assert(py_sizeof<Tuple> <= 80);
static_assert(py_sizeof<List> <= 64);
static_assert(py_sizeof<Dict> <= 64);
static_assert(py_sizeof<RangeIter> <= 64);
static_assert(py_sizeof<RangeIterR> <= 64);
static_assert(py_sizeof<ArrayIter> <= 64);
static_assert(py_sizeof<StringIter> <= 64);
static_assert(py_sizeof<Generator> <= 64);
static_assert(py_sizeof<DictItemsIter> <= 64);
}  // namespace pkpy
