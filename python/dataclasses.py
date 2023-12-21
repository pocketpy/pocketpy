def _wrapped__init__(self, *args, **kwargs):
    cls = type(self)
    cls_d = cls.__dict__
    fields: tuple[str] = cls.__annotations__
    i = 0   # index into args
    for field in fields:
        if field in kwargs:
            setattr(self, field, kwargs.pop(field))
        else:
            if i < len(args):
                setattr(self, field, args[i])
                ++i
            elif field in cls_d:    # has default value
                setattr(self, field, cls_d[field])
            else:
                raise TypeError(f"{cls.__name__} missing required argument {field!r}")
    if len(args) > i:
        raise TypeError(f"{cls.__name__} takes {len(field)} positional arguments but {len(args)} were given")
    if len(kwargs) > 0:
        raise TypeError(f"{cls.__name__} got an unexpected keyword argument {next(iter(kwargs))!r}")

def _wrapped__repr__(self):
    fields: tuple[str] = type(self).__annotations__
    obj_d = self.__dict__
    args: list = [f"{field}={obj_d[field]!r}" for field in fields]
    return f"{type(self).__name__}({', '.join(args)})"

def _wrapped__eq__(self, other):
    if type(self) is not type(other):
        return False
    fields: tuple[str] = type(self).__annotations__
    for field in fields:
        if getattr(self, field) != getattr(other, field):
            return False
    return True

def dataclass(cls: type):
    assert type(cls) is type
    cls_d = cls.__dict__
    if '__init__' not in cls_d:
        cls.__init__ = _wrapped__init__
    if '__repr__' not in cls_d:
        cls.__repr__ = _wrapped__repr__
    if '__eq__' not in cls_d:
        cls.__eq__ = _wrapped__eq__
    fields: tuple[str] = cls.__annotations__
    has_default = False
    for field in fields:
        if field in cls_d:
            has_default = True
        else:
            if has_default:
                raise TypeError(f"non-default argument {field!r} follows default argument")
    return cls

def asdict(obj) -> dict:
    fields: tuple[str] = type(obj).__annotations__
    obj_d = obj.__dict__
    return {field: obj_d[field] for field in fields}
