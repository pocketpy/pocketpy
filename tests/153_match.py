# match case
case, _1 = 1, 2
assert case == 1 and _1 == 2

match (404 * 1):
    case 200:
        assert False
    case 404:
        assert True
    case _: assert False

match (555 * 1):
    case 200:
        assert False
    case 404: assert False
    case _:
        assert True

match (555 * 1):
    case 200:
        assert False
    case 404:
        assert False
    # no default case


def f(case):
    match case:
        case 200:
            return True
        case 404:
            return False
        case _:
            return False

assert f(200) == True

# String patterns use the full literal, including embedded NUL bytes.
def match_nul_string(value):
    match value:
        case "abcdefghijklmnopqrstuvwxyz\0first":
            return "first"
        case "abcdefghijklmnopqrstuvwxyz\0second":
            return "second"
        case _:
            return "fallback"

assert match_nul_string("abcdefghijklmnopqrstuvwxyz\0first") == "first"
assert match_nul_string("abcdefghijklmnopqrstuvwxyz\0second") == "second"
assert match_nul_string("abcdefghijklmnopqrstuvwxyz") == "fallback"
assert match_nul_string("abcdefghijklmnopqrstuvwxyz\0different") == "fallback"

# structural match patterns: captures, wildcards, and nested sequences
def eval_ast(expr):
    match expr:
        case ("lit", value):
            return value
        case ("+", left, right):
            return eval_ast(left) + eval_ast(right)
        case _:
            raise ValueError(expr)

assert eval_ast(("+", ("lit", 1), ("+", ("lit", 2), ("lit", 3)))) == 6

def match_nested_list(expr):
    match expr:
        case ["node", ("lit", value)]:
            return value
        case ["node", ["pair", left, right]]:
            return left + right
        case _:
            return None

assert match_nested_list(["node", ("lit", 9)]) == 9
assert match_nested_list(["node", ["pair", 2, 3]]) == 5
assert match_nested_list(("node", ("lit", 9))) == 9
assert match_nested_list(["node", ["pair", 2]]) is None

def match_nested_then_sibling(value):
    match value:
        case ([left, right], tail):
            return (left, right, tail)
        case (head, [left, right], tail):
            return (head, left, right, tail)
        case _:
            return None

assert match_nested_then_sibling(([1, 2], 3)) == (1, 2, 3)
assert match_nested_then_sibling((0, [4, 5], 6)) == (0, 4, 5, 6)

def match_nested_failure_then_case(value):
    match value:
        case ([left, right], tail):
            return "wrong case"
        case ([only], tail):
            return (only, tail)
        case _:
            return "fallback"

assert match_nested_failure_then_case(([7], 8)) == (7, 8)
assert match_nested_failure_then_case(([7, 8, 9], 10)) == "fallback"

def match_single_item(value):
    match value:
        case [item]:
            return item
        case _:
            return None

assert match_single_item([5]) == 5
assert match_single_item((6,)) == 6
assert match_single_item([1, 2]) is None

def match_wildcard(expr):
    match expr:
        case ("tag", _):
            return True
        case _:
            return False

assert match_wildcard(("tag", object()))
assert not match_wildcard(["other", 1])
assert not match_wildcard(42)

def capture_whole_subject(value):
    match value:
        case result:
            return result

assert capture_whole_subject(42) == 42

# Singleton patterns compare by identity, including inside sequence patterns.
def match_singleton(value):
    match value:
        case True:
            return "true"
        case False:
            return "false"
        case None:
            return "none"
        case _:
            return "other"

assert match_singleton(True) == "true"
assert match_singleton(False) == "false"
assert match_singleton(None) == "none"
for value in (1, 0, 1.0, 0.0, "", [], ()):
    assert match_singleton(value) == "other"

def match_number(value):
    match value:
        case 1:
            return "one"
        case 0:
            return "zero"
        case _:
            return "other"

assert match_number(True) == "one"
assert match_number(False) == "zero"
assert match_number(1.0) == "one"
assert match_number(0.0) == "zero"

class EqualToEverything:
    def __eq__(self, other):
        return True
    def __ne__(self, other):
        return False

# User-defined equality cannot make an object match a singleton pattern.
assert match_singleton(EqualToEverything()) == "other"
assert match_number(EqualToEverything()) == "one"

class EqualityRaises:
    def __eq__(self, other):
        raise ValueError("equality called")
    def __ne__(self, other):
        raise ValueError("inequality called")

assert match_singleton(EqualityRaises()) == "other"
equality_raised = False
try:
    match_number(EqualityRaises())
except ValueError:
    equality_raised = True
assert equality_raised

def match_nested_singleton(value):
    match value:
        case ["tag", (True, False, None)]:
            return "singletons"
        case ["tag", (1, 0, _)]:
            return "numbers"
        case _:
            return "other"

assert match_nested_singleton(["tag", (True, False, None)]) == "singletons"
assert match_nested_singleton(["tag", (1, 0, None)]) == "numbers"
assert match_nested_singleton(["tag", (1.0, 0.0, None)]) == "numbers"
assert match_nested_singleton(["tag", (True, False, EqualToEverything())]) == "numbers"
assert match_nested_singleton(["tag", (True, False)]) == "other"

# Captures in a class body bind class attributes, without changing globals.
class_capture = "global"
class MatchedAttributes:
    match ("point", 3, 4):
        case ("point", class_capture, y):
            total = class_capture + y
    match 9:
        case whole:
            pass

assert MatchedAttributes.class_capture == 3
assert MatchedAttributes.y == 4
assert MatchedAttributes.total == 7
assert MatchedAttributes.whole == 9
assert class_capture == "global"

# Fixed-length sequence patterns can omit parentheses and include a trailing comma.
def open_sequence(value):
    match value:
        case "point", x, y:
            return x + y
        case item,:
            return item
        case _:
            return None

assert open_sequence(("point", 3, 4)) == 7
assert open_sequence(["point", 3, 4]) == 7
assert open_sequence((5,)) == 5
assert open_sequence(("point", 3)) is None
assert open_sequence("point") is None

subject_calls = []
def subject_item(value):
    subject_calls.append(value)
    return value

match subject_item(2), subject_item(3):
    case left, right,:
        subject_total = left + right
assert subject_total == 5
assert subject_calls == [2, 3]

# Aliases bind the original value, including nested sequences and grouped aliases.
def alias_sequence(value):
    match value:
        case ([1, item] as pair, tail) as whole:
            assert pair is value[0]
            assert whole is value
            return item + tail
        case ([2] as pair, tail) as whole:
            assert pair is value[0]
            assert whole is value
            return tail
        case _:
            return None

assert alias_sequence(([1, 4], 5)) == 9
assert alias_sequence(([2], 6)) == 6
assert alias_sequence(([1], 6)) is None
assert alias_sequence(([3], 6)) is None
assert alias_sequence(([1, 4],)) is None

def failed_alias(value):
    sentinel = object()
    whole = sentinel
    match value:
        case (([1, item] as pair) as duplicate, 2) as whole:
            assert pair is duplicate
            assert whole is value
            return item
        case _:
            assert whole is sentinel
            return None

for _ in range(20):
    assert failed_alias(([1, 4], 2)) == 4
    assert failed_alias(([1, 4], 3)) is None
    assert failed_alias(([0, 4], 2)) is None
    assert failed_alias(([1], 2)) is None
    assert failed_alias((1, 2)) is None

match 7:
    case ((7 as first) as second) as third:
        assert first == second == third == 7

match (2, 3):
    case left, right as last:
        assert left == 2 and right == 3 and last == 3

match object():
    case _ as original:
        assert original is not None

# Guards see all captures, run only after a successful pattern and stop on success.
guard_calls = []
def guard(label, result):
    guard_calls.append(label)
    return result

def guarded(value):
    match value:
        case ("other", item) if guard("mismatch", True):
            return "wrong"
        case ("item", item) as whole if guard("first", item > 0):
            assert whole is value
            return item
        case ("item", item) if guard("second", item == 0):
            return 0
        case number if guard("capture", False):
            return "wrong"
        case _ if guard("wildcard", False):
            return "wrong"
        case _:
            return "fallback"

assert guarded(("item", 4)) == 4
assert guard_calls == ["first"]
guard_calls.clear()
assert guarded(("item", 0)) == 0
assert guard_calls == ["first", "second"]
guard_calls.clear()
assert guarded(("item", -1)) == "fallback"
assert guard_calls == ["first", "second", "capture", "wildcard"]

match [1, 2]:
    case left, right, if left + right == 3:
        assert left == 1 and right == 2
    case _:
        assert False

# A false guard does not undo a successful capture.
match 8:
    case captured if False:
        assert False
    case _:
        assert captured == 8
assert captured == 8

class FalseGuard:
    def __bool__(self):
        return False

match 1:
    case number if FalseGuard():
        assert False
    case number if [1]:
        assert number == 1

# Guard exceptions propagate, including exceptions during truth conversion.
def raising_guard():
    raise ValueError("guard failed")

class RaisingGuard:
    def __bool__(self):
        raise ValueError("truth conversion failed")

for check_guard in (raising_guard, RaisingGuard):
    caught = False
    try:
        match 1:
            case number if check_guard():
                assert False
            case _:
                assert False
    except ValueError:
        caught = True
    assert caught

# Dotted value patterns use equality and support repeated attribute lookups.
class Codes:
    POINT = "point"
    ONE = 1
    TRUE = True
    NONE = None

class Namespace:
    codes = Codes

def dotted(value, constants):
    match value:
        case constants.POINT, x, y as last:
            return (x, y, last)
        case Namespace.codes.ONE:
            return "one"
        case Codes.NONE:
            return "none"
        case _:
            return "other"

assert dotted(("point", 3, 4), Codes) == (3, 4, 4)
assert dotted(True, Codes) == "one"
assert dotted(1.0, Codes) == "one"
assert dotted(None, Codes) == "none"
assert dotted("other", Codes) == "other"

# Values referring to True use equality, unlike the True singleton pattern.
match 1:
    case Codes.TRUE:
        pass
    case _:
        assert False

Codes.ONE = 2
assert dotted(2, Codes) == "one"
assert dotted(1, Codes) == "other"
Codes.ONE = 1

class DynamicValues:
    def __init__(self):
        self.calls = []
    def __getattr__(self, name):
        self.calls.append(name)
        return 4

values = DynamicValues()
match (4, 5):
    case (values.FIRST, item) if False:
        assert False
    case (values.SECOND, item):
        assert item == 5
assert values.calls == ["FIRST", "SECOND"]

caught = False
try:
    match 1:
        case Codes.MISSING:
            assert False
        case _:
            assert False
except AttributeError:
    caught = True
assert caught

# Aliases use the same binding rules as other captures.
alias_global = None
def global_alias(value):
    global alias_global
    match value:
        case ("item", item) as alias_global if item > 0:
            return item
    return None

original = ("item", 5)
assert global_alias(original) == 5
assert alias_global is original

class CapturedAttributes:
    match ("point", 2, 3):
        case (Codes.POINT, x, y) as whole if x < y:
            total = x + y

assert CapturedAttributes.total == 5
assert CapturedAttributes.whole == ("point", 2, 3)

# Validate restrictions across combinations of the supported forms.
def invalid_pattern(source):
    caught = False
    try:
        compile(source, "<match syntax>", "exec")
    except SyntaxError:
        caught = True
    assert caught, source

for pattern in ("x, x", "[x] as x", "1 as _", "1 as x.y", "1 as x as y",
                "[x as y, y]", "[1] as", "Codes.", "_.VALUE"):
    invalid_pattern("match value:\n    case " + pattern + ":\n        pass\n")

for pattern in ("x", "_ as x", "(x as y)"):
    invalid_pattern("match value:\n    case " + pattern +
                    ":\n        pass\n    case _:\n        pass\n")

