from typing import Dict, Tuple, List

bill: Dict[str, float] = {
    "apple": 3.14,
    "watermelon": 15.92,
    "pineapple": 6.53,
}
completed: Tuple[str] = ("DONE",)
succeeded: Tuple[int, str] = (1, "SUCCESS")
statuses: Tuple[str, ...] = (
    "DONE", "SUCCESS", "FAILED", "ERROR",
)
codes: List[int] = (0, 1, -1, -2)


from typing import Union

def resp200(meaningful) -> Union[int, str]:
    return "OK" if meaningful else 200


from typing import Self

class Employee:
    name: str = "John Doe"
    age: int = 0

    def set_name(self: Self, name) -> Self:
        self.name = name
        return self
    
from typing import TypeVar, Type

T = TypeVar("T")

# "mapper" is a type, like int, str, MyClass and so on.
# "default" is an instance of type T, such as 314, "string", MyClass() and so on.
# returned is an instance of type T too.
def converter(raw, mapper: Type[T] = None, default: T = None) -> T:
    try:
        return mapper(raw)
    except:
        return default

raw: str = '4'
result: int = converter(raw, mapper=int, default=0)

from typing import TypeVar, Callable, Any

T = TypeVar("T")

def converter(raw, mapper: Callable[[Any], T] = None, default: T = None) -> T:
    try:
        return mapper(raw)
    except:
        return default

# Callable[[Any], ReturnType] means a function declare like:
# def func(arg: Any) -> ReturnType:
#     pass

# Callable[[str, int], ReturnType] means a function declare like:
# def func(string: str, times: int) -> ReturnType:
#     pass

# Callable[..., ReturnType] means a function declare like:
# def func(*args, **kwargs) -> ReturnType:
#     pass

def is_success(value) -> bool:
    return value in (0, "OK", True, "success")

resp = {'code': 0, 'message': 'OK', 'data': []}
successed: bool = converter(resp['message'], mapper=is_success, default=False)


class A:
    x: List[Callable[[int], Any]]
    y: Dict[str, int]

a = A()
assert not hasattr(a, 'x')
assert not hasattr(a, 'y')

class B:
    x: List[Callable[[int], Any]] = []
    y: Dict[str, int] = {}

b = B()
assert hasattr(b, 'x')
assert hasattr(b, 'y')

abc123: int
assert 'abc123' not in globals()