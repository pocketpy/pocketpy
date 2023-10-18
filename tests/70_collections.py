from collections import Counter, deque
import random
import pickle
import gc

q = deque()
q.append(1)
q.append(2)
q.appendleft(3)
q.append(4)
assert len(q) == 4
assert q == deque([3, 1, 2, 4])
assert q.popleft() == 3
assert q.pop() == 4
assert len(q) == 2
assert q == deque([1, 2])

## ADDING TESTS FROM CPYTHON's test_deque.py file

############TEST BASICS###############
def assertEqual(a, b):
    assert a == b

d = deque(range(-5125, -5000))
d.__init__(range(200))
for i in range(200, 400):
    d.append(i)
for i in reversed(range(-200, 0)):
    d.appendleft(i)

assertEqual(list(d), list(range(-200, 400)))
assertEqual(len(d), 600)

left = [d.popleft() for i in range(250)]
assertEqual(left, list(range(-200, 50)))
assertEqual(list(d), list(range(50, 400)))

right = [d.pop() for i in range(250)]
right.reverse()
assertEqual(right, list(range(150, 400)))
assertEqual(list(d), list(range(50, 150)))

#######TEST MAXLEN###############
try:
    dq = deque()
    dq.maxlen = -1
except AttributeError:
    pass

try:
    dq = deque()
    dq.maxlen = -2
except AttributeError:
    pass

it = iter(range(10))
d = deque(it, maxlen=3)
assertEqual(list(it), [])
assertEqual(repr(d), 'deque([7, 8, 9], maxlen=3)')
assertEqual(list(d), [7, 8, 9])
assertEqual(d, deque(range(10), 3))
d.append(10)
assertEqual(list(d), [8, 9, 10])
d.appendleft(7)
assertEqual(list(d), [7, 8, 9])
d.extend([10, 11])
assertEqual(list(d), [9, 10, 11])
d.extendleft([8, 7])
assertEqual(list(d), [7, 8, 9])
d = deque(range(200), maxlen=10)
d.append(d)
assertEqual(repr(d)[-30:], ', 198, 199, [...]], maxlen=10)')
d = deque(range(10), maxlen=None)
assertEqual(repr(d), 'deque([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])')

#######TEST MAXLEN SIZE 0###############
it = iter(range(100))
deque(it, maxlen=0)
assertEqual(list(it), [])

it = iter(range(100))
d = deque(maxlen=0)
d.extend(it)
assertEqual(list(it), [])

it = iter(range(100))
d = deque(maxlen=0)
d.extendleft(it)
assertEqual(list(it), [])


#######TEST MAXLEN ATTRIBUTE#############

assertEqual(deque().maxlen, None)
assertEqual(deque('abc').maxlen, None)
assertEqual(deque('abc', maxlen=4).maxlen, 4)
assertEqual(deque('abc', maxlen=2).maxlen, 2)
assertEqual(deque('abc', maxlen=0).maxlen, 0)
try:
    d = deque('abc')
    d.maxlen = 10 
    print("Failed Tests!!")
    exit(1)
except AttributeError:
    pass

######### TEST COUNT#################
for s in ('', 'abracadabra', 'simsalabim'*500+'abc'):
    s = list(s)
    d = deque(s)
    for letter in 'abcdefghijklmnopqrstuvwxyz':
        assertEqual(s.count(letter), d.count(letter))
try:
    d.count()
    print("Failed Tests!!")
    exit(1)
except TypeError:
    pass

try:
    d.count(1,2)
    print("Failed Tests!!")
    exit(1)
except TypeError:
    pass
   
class BadCompare:
    def __eq__(self, other):
        raise ArithmeticError
d = deque([1, 2, BadCompare(), 3])

try:
    d.count(2)
    print("Failed Tests!!")
    exit(1)
except ArithmeticError:
    pass

d = deque([1, 2, 3])
try:
    d.count(BadCompare())
    print("Failed Tests!!")
    exit(1)
except ArithmeticError:
    pass

class MutatingCompare:
    def __eq__(self, other):
        d.pop()
        return True
m = MutatingCompare()
d = deque([1, 2, 3, m, 4, 5])
m.d = d

try:
    d.count(3)
    print("Failed Tests!")
    exit(1)
except RuntimeError:
    pass

d = deque([None]*16)
for i in range(len(d)):
    d.rotate(-1)
d.rotate(1)
assertEqual(d.count(1), 0)
assertEqual(d.count(None), 16)

#### TEST COMPARISONS#####

d = deque('xabc')
d.popleft()
for e in [d, deque('abc'), deque('ab'), deque(), list(d)]:
    assertEqual(d==e, type(d)==type(e) and list(d)==list(e))
    assertEqual(d!=e, not(type(d)==type(e) and list(d)==list(e)))

args = map(deque, ('', 'a', 'b', 'ab', 'ba', 'abc', 'xba', 'xabc', 'cba'))
for x in args:
    for y in args:
        assertEqual(x == y, list(x) == list(y))
        assertEqual(x != y, list(x) != list(y))
        # assertEqual(x <  y, list(x) <  list(y))   # not currently supported
        # assertEqual(x <= y, list(x) <= list(y))   # not currently supported
        # assertEqual(x >  y, list(x) >  list(y))   # not currently supported
        # assertEqual(x >= y, list(x) >= list(y))   # not currently supported

###############TEST CONTAINS#################

n = 200

d = deque(range(n))
for i in range(n):
    assertEqual(i in d, True)
assertEqual((n+1) not in d, True)


class MutateCmp:
    def __init__(self, deque, result):
        self.deque = deque
        self.result = result
    def __eq__(self, other):
        self.deque.clear()
        return self.result

# Test detection of mutation during iteration
# d = deque(range(n))
# d[n//2] = MutateCmp(d, False)
# print(n in d)
# with assertRaises(RuntimeError):
#     n in d

# # Test detection of comparison exceptions
# d = deque(range(n))
# d[n//2] = BadCmp()
# with assertRaises(RuntimeError):
#     n in d







print("ALL TEST PASSED!!")