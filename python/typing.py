class _Placeholder:
    def __init__(self, *args, **kwargs):
        pass
    def __getitem__(self, *args):
        return self
    def __call__(self, *args, **kwargs):
        return self
    

_PLACEHOLDER = _Placeholder()

List = _PLACEHOLDER
Dict = _PLACEHOLDER
Tuple = _PLACEHOLDER
Set = _PLACEHOLDER
Any = _PLACEHOLDER
Union = _PLACEHOLDER
Optional = _PLACEHOLDER
Callable = _PLACEHOLDER
Type = _PLACEHOLDER
Protocol = _PLACEHOLDER

Iterable = _PLACEHOLDER
Generator = _PLACEHOLDER

TypeVar = _PLACEHOLDER
Self = _PLACEHOLDER

class Generic:
    pass

TYPE_CHECKING = False

# decorators
overload = lambda x: x
final = lambda x: x
