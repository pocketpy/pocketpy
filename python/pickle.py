import json
import builtins

_BASIC_TYPES = [int, float, str, bool, type(None)]

def _find_class(path: str):
    if "." not in path:
        g = globals()
        if path in g:
            return g[path]
        return builtins.__dict__[path]
    modname, name = path.split(".")
    return __import__(modname).__dict__[name]

def _find__new__(cls):
    while cls is not None:
        d = cls.__dict__
        if "__new__" in d:
            return d["__new__"]
        cls = cls.__base__
    assert False

class _Pickler:
    def __init__(self) -> None:
        self.raw_memo = {}  # id -> int
        self.memo = []      # int -> object

    def wrap(self, o):
        if type(o) in _BASIC_TYPES:
            return o
        
        index = self.raw_memo.get(id(o), None)
        if index is not None:
            return ["$", index]
        
        ret = []
        index = len(self.memo)
        self.memo.append(ret)
        self.raw_memo[id(o)] = index

        if type(o) is list:
            ret.append("list")
            ret.append([self.wrap(i) for i in o])
            return ["$", index]

        if type(o) is tuple:
            ret.append("tuple")
            ret.append([self.wrap(i) for i in o])
            return ["$", index]
        
        if type(o) is dict:
            ret.append("dict")
            ret.append([[self.wrap(k), self.wrap(v)] for k,v in o.items()])
            return ["$", index]
        
        if type(o) is bytes:
            ret.append("bytes")
            ret.append([o[j] for j in range(len(o))])
            return ["$", index]
        
        _0 = o.__class__.__name__
        if hasattr(o, "__getnewargs__"):
            _1 = o.__getnewargs__()     # an iterable
            _1 = [self.wrap(i) for i in _1]
        else:
            _1 = None
        if hasattr(o, "__getstate__"):
            _2 = o.__getstate__()
        else:
            if o.__dict__ is None:
                _2 = None
            else:
                _2 = {}
                for k,v in o.__dict__.items():
                    _2[k] = self.wrap(v)
        ret.append(_0)
        ret.append(_1)
        ret.append(_2)
        return ["$", index]

class _Unpickler:
    def __init__(self, memo: list) -> None:
        self.memo = memo
        self.unwrapped = [None] * len(memo)

    def unwrap_ref(self, i: int):
        if self.unwrapped[i] is None:
            o = self.memo[i]
            assert type(o) is list
            assert o[0] != '$'
            self.unwrapped[i] = self.unwrap(o)
        return self.unwrapped[i]

    def unwrap(self, o):
        if type(o) in _BASIC_TYPES:
            return o
        assert type(o) is list
        if o[0] == '$':
            index = o[1]
            return self.unwrap_ref(index)
        if o[0] == "list":
            return [self.unwrap(i) for i in o[1]]
        if o[0] == "tuple":
            return tuple([self.unwrap(i) for i in o[1]])
        if o[0] == "dict":
            return {self.unwrap(k): self.unwrap(v) for k,v in o[1]}
        if o[0] == "bytes":
            return bytes(o[1])
        # generic object
        cls, newargs, state = o
        cls = _find_class(o[0])
        # create uninitialized instance
        new_f = _find__new__(cls)
        if newargs is not None:
            newargs = [self.unwrap(i) for i in newargs]
            inst = new_f(cls, *newargs)
        else:
            inst = new_f(cls)
        # restore state
        if hasattr(inst, "__setstate__"):
            inst.__setstate__(state)
        else:
            if state is not None:
                for k,v in state.items():
                    setattr(inst, k, self.unwrap(v))
        return inst

def _wrap(o):
    p = _Pickler()
    o = p.wrap(o)
    return [o, p.memo]

def _unwrap(packed: list):
    o, memo = packed
    return _Unpickler(memo).unwrap(o)

def dumps(o) -> bytes:
    o = _wrap(o)
    return json.dumps(o).encode()

def loads(b) -> object:
    assert type(b) is bytes
    o = json.loads(b.decode())
    return _unwrap(o)