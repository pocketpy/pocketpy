# if tests
flag = False
name = 'luren'
if name == 'python':         
    flag = True              
else:
    flag               
assert flag == False


num = 9
flag = 0
if num >= 0 and num <= 10:    
    flag = 1
else:
    flag
assert flag == 1

num = 10
flag = 0
if num < 0 or num > 10:    
    flag = 1
else:
    flag
assert flag == 0

num = 5
result = 0     
if num == 3:            
    result = num        
elif num == 2:
    result = num
elif num == 1:
    result = num
elif num < 0:           
    result = num
else:
    result = num
assert result == 5    

# for tests 

k = 0
for i in range(2, 1000):
  if i % 2 == 0:
    k += 1
assert k ==499

k = 0
for x in range(100):
    if x<2:
      continue
    flag = True
    for i in range(2,x):
      if x%i == 0:
        flag = False
        break
    if flag:
      k += 1
assert k == 25

#while tests
count = 0
while (count < 1000):
   count = count + 1
assert count == 1000

# ternary operator
d = 1 if 2 > 1 else 2
assert d == 1
d = 1 if 2 < 1 else 2
assert d == 2

t = 0
for i in range(5):
    if True and 1:
        break
    t = 1
assert t == 0

for i in range(5):
    break
else:
    assert False

for i in range(5):
    if i==3:
        break
else:
    assert False

flag = False
for i in range(5):
    if i==6:
        break
else:
    flag = True
assert flag is True

while True:
    break
else:
    assert False

flag = False
while False:
    assert False
else:
    flag = True
assert flag is True

x = 1
while 0:
    while True:
        break
else:
    x = 2
assert x == 2

if x == 2:
    while 0:
        pass
else:
    x = 3
assert x == 2

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

# extras

t = 0
for i in range(5):
    try:
        break
    except:
        pass
    t = 1
assert t == 0
