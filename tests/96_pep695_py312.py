
class Test[T]:
    def __init__(self, value: T):
        self.value = value

    def get_value(self) -> T:
        return self.value


def add[T: int|str|float](a: T, b: T) -> T:
    return a + b # type: ignore

res = add(1, 2)
assert res == 3

test = Test(1)
assert test.get_value() == 1
