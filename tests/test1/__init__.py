from . import _a
add = _a.add

from ._a import add as add2

assert add is add2