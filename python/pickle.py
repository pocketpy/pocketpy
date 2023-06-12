import json
import builtins

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
    raise PickleError(f"cannot find __new__ for {cls.__name__}")

def _wrap(o):
    if type(o) in (int, float, str, bool, type(None)):
        return o
    if type(o) is list:
        return ["list", [_wrap(i) for i in o]]
    if type(o) is tuple:
        return ["tuple", [_wrap(i) for i in o]]
    if type(o) is dict:
        return ["dict", [[_wrap(k), _wrap(v)] for k,v in o.items()]]
    if type(o) is bytes:
        return ["bytes", [o[j] for j in range(len(o))]]
    
    _0 = o.__class__.__name__
    if hasattr(o, "__getnewargs__"):
        _1 = o.__getnewargs__()     # an iterable
        _1 = [_wrap(i) for i in _1]
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
                _2[k] = _wrap(v)
    return [_0, _1, _2]

def _unwrap(o):
    if type(o) in (int, float, str, bool, type(None)):
        return o
    if isinstance(o, list):
        if o[0] == "list":
            return [_unwrap(i) for i in o[1]]
        if o[0] == "tuple":
            return tuple([_unwrap(i) for i in o[1]])
        if o[0] == "dict":
            return {_unwrap(k): _unwrap(v) for k,v in o[1]}
        if o[0] == "bytes":
            return bytes(o[1])
        # generic object
        cls, newargs, state = o
        cls = _find_class(o[0])
        # create uninitialized instance
        new_f = _find__new__(cls)
        if newargs is not None:
            newargs = [_unwrap(i) for i in newargs]
            inst = new_f(cls, *newargs)
        else:
            inst = new_f(cls)
        # restore state
        if hasattr(inst, "__setstate__"):
            inst.__setstate__(state)
        else:
            if state is not None:
                for k,v in state.items():
                    setattr(inst, k, _unwrap(v))
        return inst
    raise PickleError(f"cannot unpickle {type(o).__name__} object")


def dumps(o) -> bytes:
    return json.dumps(_wrap(o)).encode()


def loads(b) -> object:
    assert type(b) is bytes
    o = json.loads(b.decode())
    return _unwrap(o)