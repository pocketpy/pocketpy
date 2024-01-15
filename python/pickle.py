import json
from c import struct
import builtins

_BASIC_TYPES = [int, float, str, bool, type(None)]
_MOD_T_SEP = "@"

def _find_class(path: str):
    if _MOD_T_SEP not in path:
        return builtins.__dict__[path]
    modpath, name = path.split(_MOD_T_SEP)
    return __import__(modpath).__dict__[name]

class _Pickler:
    def __init__(self, obj) -> None:
        self.obj = obj
        self.raw_memo = {}  # id -> int
        self.memo = []      # int -> object

    @staticmethod
    def _type_id(t: type):
        assert type(t) is type
        name = t.__name__
        mod = t.__module__
        if mod is not None:
            name = mod.__path__ + _MOD_T_SEP + name
        return name

    def wrap(self, o):
        o_t = type(o)
        if o_t in _BASIC_TYPES:
            return o
        if o_t is type:
            return ["type", self._type_id(o)]

        index = self.raw_memo.get(id(o), None)
        if index is not None:
            return [index]
        
        ret = []
        index = len(self.memo)
        self.memo.append(ret)
        self.raw_memo[id(o)] = index

        if o_t is tuple:
            ret.append("tuple")
            ret.append([self.wrap(i) for i in o])
            return [index]
        if o_t is bytes:
            ret.append("bytes")
            ret.append([o[j] for j in range(len(o))])
            return [index]
        if o_t is list:
            ret.append("list")
            ret.append([self.wrap(i) for i in o])
            return [index]
        if o_t is dict:
            ret.append("dict")
            ret.append([[self.wrap(k), self.wrap(v)] for k,v in o.items()])
            return [index]
        
        _0 = self._type_id(o_t)

        if getattr(o_t, '__struct__', False):
            ret.append(_0)
            ret.append(o.to_struct().hex())
            return [index]

        if hasattr(o, "__getnewargs__"):
            _1 = o.__getnewargs__()     # an iterable
            _1 = [self.wrap(i) for i in _1]
        else:
            _1 = None

        if o.__dict__ is None:
            _2 = None
        else:
            _2 = {k: self.wrap(v) for k,v in o.__dict__.items()}

        ret.append(_0)  # type id
        ret.append(_1)  # newargs
        ret.append(_2)  # state
        return [index]
    
    def run_pipe(self):
        o = self.wrap(self.obj)
        return [o, self.memo]



class _Unpickler:
    def __init__(self, obj, memo: list) -> None:
        self.obj = obj
        self.memo = memo
        self._unwrapped = [None] * len(memo)

    def tag(self, index, o):
        assert self._unwrapped[index] is None
        self._unwrapped[index] = o

    def unwrap(self, o, index=None):
        if type(o) in _BASIC_TYPES:
            return o
        assert type(o) is list

        if o[0] == "type":
            return _find_class(o[1])

        # reference
        if type(o[0]) is int:
            assert index is None    # index should be None
            index = o[0]
            if self._unwrapped[index] is None:
                o = self.memo[index]
                assert type(o) is list
                assert type(o[0]) is str
                self.unwrap(o, index)
                assert self._unwrapped[index] is not None
            return self._unwrapped[index]
        
        # concrete reference type
        if o[0] == "tuple":
            ret = tuple([self.unwrap(i) for i in o[1]])
            self.tag(index, ret)
            return ret
        if o[0] == "bytes":
            ret = bytes(o[1])
            self.tag(index, ret)
            return ret
        if o[0] == "list":
            ret = []
            self.tag(index, ret)
            for i in o[1]:
                ret.append(self.unwrap(i))
            return ret
        if o[0] == "dict":
            ret = {}
            self.tag(index, ret)
            for k,v in o[1]:
                ret[self.unwrap(k)] = self.unwrap(v)
            return ret
        
        # generic object
        cls = _find_class(o[0])
        if getattr(cls, '__struct__', False):
            inst = cls.from_struct(struct.fromhex(o[1]))
            self.tag(index, inst)
            return inst
        else:
            _, newargs, state = o
            # create uninitialized instance
            new_f = getattr(cls, "__new__")
            if newargs is not None:
                newargs = [self.unwrap(i) for i in newargs]
                inst = new_f(cls, *newargs)
            else:
                inst = new_f(cls)
            self.tag(index, inst)
            # restore state
            if state is not None:
                for k,v in state.items():
                    setattr(inst, k, self.unwrap(v))
            return inst

    def run_pipe(self):
        return self.unwrap(self.obj)


def _wrap(o):
    return _Pickler(o).run_pipe()

def _unwrap(packed: list):
    return _Unpickler(*packed).run_pipe()

def dumps(o) -> bytes:
    o = _wrap(o)
    return json.dumps(o).encode()

def loads(b) -> object:
    assert type(b) is bytes
    o = json.loads(b.decode())
    return _unwrap(o)