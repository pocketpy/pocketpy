class _Placeholder:
    def __init__(self, *args, **kwargs):
        pass
    def __getitem__(self, *args):
        return self
    def __call__(self, *args, **kwargs):
        return self
    def __and__(self, other):
        return self
    def __or__(self, other):
        return self
    def __xor__(self, other):
        return self


_PLACEHOLDER = _Placeholder()

Sequence = _PLACEHOLDER
List = _PLACEHOLDER
Dict = _PLACEHOLDER
Tuple = _PLACEHOLDER
Set = _PLACEHOLDER
Any = _PLACEHOLDER
Union = _PLACEHOLDER
Optional = _PLACEHOLDER
Callable = _PLACEHOLDER
Type = _PLACEHOLDER
TypeAlias = _PLACEHOLDER
NewType = _PLACEHOLDER

Literal = _PLACEHOLDER
LiteralString = _PLACEHOLDER

Iterable = _PLACEHOLDER
Generator = _PLACEHOLDER
Iterator = _PLACEHOLDER

Hashable = _PLACEHOLDER

TypeVar = _PLACEHOLDER
Self = _PLACEHOLDER

Protocol = object
Generic = object
Never = object

TYPE_CHECKING = False

# decorators
overload = lambda x: x
final = lambda x: x

# exhaustiveness checking
assert_never = lambda x: x

TypedDict = dict
NotRequired = _PLACEHOLDER
