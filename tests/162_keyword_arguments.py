# https://github.com/pocketpy/pocketpy/issues/536
def move_entity(entity_id: int, x: float, duration: float = 0):
    return entity_id, x, duration


assert move_entity(entity_id=1, x=2.5, duration=1) == (1, 2.5, 1)
assert move_entity(duration=1, x=2.5, entity_id=1) == (1, 2.5, 1)
assert move_entity(1, x=2.5) == (1, 2.5, 0)
assert move_entity(entity_id=1, x=2.5) == (1, 2.5, 0)
assert move_entity(entity_id=None, x=None, duration=None) == (None, None, None)
assert move_entity(*[1], **{'x': 2.5, 'duration': 1}) == (1, 2.5, 1)


# Functions without defaults retain their positional call path after a keyword call.
def required(a, b):
    return a, b


assert required(b=2, a=1) == (1, 2)
assert required(1, b=2) == (1, 2)
assert required(a=None, b=2) == (None, 2)
assert required(**{'b': 2, 'a': 1}) == (1, 2)
assert required(1, 2) == (1, 2)
assert (lambda a, b: (a, b))(b=2, a=1) == (1, 2)


def optional(a=None, b=2):
    return a, b


assert optional() == (None, 2)
assert optional(a=1) == (1, 2)
assert optional(b=None) == (None, None)
assert optional(1, b=3) == (1, 3)


# Named parameters bind before leftover keywords are collected in **kwargs.
def packed(a, *args, flag=3, **kwargs):
    return a, args, flag, kwargs


assert packed(a=1) == (1, (), 3, {})
assert packed(flag=None, a=None, extra=4) == (None, (), None, {'extra': 4})
assert packed(1, 2, 3, flag=4, extra=5) == (1, (2, 3), 4, {'extra': 5})
assert packed(**{'a': 1, 'extra': 2}) == (1, (), 3, {'extra': 2})


def generated(a, b=2):
    yield a, b


assert list(generated(a=1)) == [(1, 2)]
assert list(generated(b=None, a=None)) == [(None, None)]


class Entity:
    def __init__(self, entity_id):
        self.entity_id = entity_id

    def move(self, x, duration=0):
        return self.entity_id, x, duration

    def get(self, value):
        return self.entity_id, value


entity = Entity(entity_id=1)
assert entity.move(x=2.5, duration=1) == (1, 2.5, 1)
assert Entity.move(self=entity, x=2.5) == (1, 2.5, 0)
assert entity.get(value=2) == (1, 2)
bound = entity.get
assert bound(value=3) == (1, 3)
assert bound(4) == (1, 4)


# Declaration-based C bindings use the same argument binding path.
import json

assert json.loads(json.dumps(obj={'a': 1})) == {'a': 1}


def assert_type_error(call):
    try:
        call()
    except TypeError:
        return
    raise AssertionError('expected TypeError')


for call in (
    lambda: move_entity(entity_id=1),
    lambda: move_entity(x=2.5),
    lambda: required(b=2),
    lambda: packed(flag=4),
    lambda: generated(b=2),
    lambda: required(1, a=2, b=3),
    lambda: required(None, a=1, b=2),
    lambda: move_entity(1, 2.5, 0, duration=1),
    lambda: optional(1, a=2),
    lambda: optional(None, a=2),
    lambda: packed(1, a=2),
    lambda: entity.get(value=2, self=entity),
    lambda: json.dumps({}, obj={}),
    lambda: required(a=1, b=2, unknown=3),
    lambda: move_entity(entity_id=1, x=2.5, unknown=3),
    lambda: required(1, 2, 3),
    lambda: move_entity(1, 2.5, 0, 4),
):
    assert_type_error(call)

assert required(1, 2) == (1, 2)
assert packed(a=1) == (1, (), 3, {})
