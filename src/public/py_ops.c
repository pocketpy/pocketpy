#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"

int py_eq(const py_Ref lhs, const py_Ref rhs) { return 0; }

int py_le(const py_Ref lhs, const py_Ref rhs) { return 0; }

bool py_hash(const py_Ref val, int64_t* out) { return 0; }

bool py_getattr(const py_Ref self, py_Name name, py_Ref out) { return true; }

bool py_setattr(py_Ref self, py_Name name, const py_Ref val) { return -1; }

bool py_delattr(py_Ref self, py_Name name) { return -1; }

bool py_getitem(const py_Ref self, const py_Ref key, py_Ref out) { return -1; }

bool py_setitem(py_Ref self, const py_Ref key, const py_Ref val) { return -1; }

bool py_delitem(py_Ref self, const py_Ref key) { return -1; }