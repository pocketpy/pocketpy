from collections import Counter, deque, defaultdict
import random
import pickle
import gc

# test defaultdict
assert issubclass(defaultdict, dict)
a = defaultdict(int)
a['1'] += 1
assert a == {'1': 1}
a = defaultdict(list)
a['1'].append(1)
assert a == {'1': [1]}

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

# ADDING TESTS FROM CPYTHON's test_deque.py file

############ TEST basics###############


def assertEqual(a, b):
    if a == b:
        return
    print(a)
    print(b)
    raise AssertionError

def assertNotEqual(a, b):
    if a != b:
        return
    print(a)
    print(b)
    raise AssertionError

def printFailed(function_name, *args, **kwargs):
    print("X Failed Tests for {} for args: {} {}".format(str(function_name), str(args), str(kwargs)))


BIG = 10000


def fail():
    raise SyntaxError
    yield 1

d = deque()
assertEqual(len(d), 0)
assertEqual(list(d), [])

d = deque(range(6))
assertEqual(list(d), list(range(6)))
d = deque(range(7)) # [0, 1, 2, 3, 4, 5, 6]
# print(d._data, d._head, d._tail, d._capacity)
assertEqual(list(d), list(range(7)))
d = deque(range(8))
assertEqual(list(d), list(range(8)))
d = deque(range(9))
assertEqual(list(d), list(range(9)))

d = deque(range(200))
for i in range(200, 400):
    d.append(i)

assertEqual(len(d), 400)
assertEqual(list(d), list(range(400)))

for i in reversed(range(-200, 0)):
    d.appendleft(i)

assertEqual(len(d), 600)
assertEqual(list(d), list(range(-200, 400)))

left = [d.popleft() for i in range(250)]
assertEqual(left, list(range(-200, 50)))
assertEqual(list(d), list(range(50, 400)))

right = [d.pop() for i in range(250)]
right.reverse()
assertEqual(right, list(range(150, 400)))
assertEqual(list(d), list(range(50, 150)))

######### TEST count()#################
for s in ('', 'abracadabra', 'simsalabim'*500+'abc'):
    s = list(s)
    d = deque(s)
    for letter in 'abcdefghijklmnopqrstuvwxyz':
        assertEqual(s.count(letter), d.count(letter))
try:
    d.count()
    printFailed("deque.count")
    exit(1)
except TypeError:
    pass

try:
    d.count(1, 2)
    printFailed("deque.count", 1, 2)
    exit(1)
except TypeError:
    pass

class ArithmeticError(Exception): pass

class BadCompare:
    def __eq__(self, other):
        raise ArithmeticError
    def __ne__(self, other):
        raise ArithmeticError


d = deque([1, 2, BadCompare(), 3])

try:
    d.count(2)
    printFailed("deque.count", 2)
    exit(1)
except ArithmeticError:
    pass

d = deque([1, 2, 3])
try:
    d.count(BadCompare())
    printFailed("deque.count", "BadCompare()")
    exit(1)
except ArithmeticError:
    pass


# class MutatingCompare:
#     def __eq__(self, other):
#         d.pop()
#         return True

# m = MutatingCompare()
# d = deque([1, 2, 3, m, 4, 5])
# m.d = d

# try:
#     d.count(3)
#     printFailed("deque.count", "MutatingCompare()")
#     exit(1)
# except RuntimeError:
#     pass

#### TEST comparisons == #####

d = deque('xabc')
d.popleft()
for e in [d, deque('abc'), deque('ab'), deque(), list(d)]:
    assertEqual(d == e, type(d) == type(e) and list(d) == list(e))
    assertEqual(d != e, not (type(d) == type(e) and list(d) == list(e)))

def get_args():
    args = map(deque, ('', 'a', 'b', 'ab', 'ba', 'abc', 'xba', 'xabc', 'cba'))
    return args

for x in get_args():
    for y in get_args():
        assertEqual(x == y, list(x) == list(y))
        assertEqual(x != y, list(x) != list(y))
        # assertEqual(x <  y, list(x) <  list(y))   # not currently supported
        # assertEqual(x <= y, list(x) <= list(y))   # not currently supported
        # assertEqual(x >  y, list(x) >  list(y))   # not currently supported
        # assertEqual(x >= y, list(x) >= list(y))   # not currently supported


############### TEST contains()#################

n = 200

d = deque(range(n))
for i in range(n):
    assertEqual(i in d, True)
assertEqual((n+1) not in d, True)


# class MutateCmp:
#     def __init__(self, deque, result):
#         self.deque = deque
#         self.result = result

#     def __eq__(self, other):
#         self.deque.clear()
#         return self.result


# # # Test detection of mutation during iteration
# d = deque(range(n))
# d[n//2] = MutateCmp(d, False)
# try:
#     n in d
#     printFailed("deque.__contains__", n)
#     exit(1)
# except RuntimeError:
#     pass


class BadCmp:
    def __eq__(self, other):
        raise RuntimeError
    def __ne__(self, other):
        raise RuntimeError


# # Test detection of comparison exceptions
d = deque(range(n))
d.append(BadCmp())
try:
    n in d
    printFailed("deque.__contains__", n)
    exit(1)
except RuntimeError:
    pass

######## TEST extend()################


d = deque('a')
try:
    d.extend(1)
    printFailed("deque.extend", 1)
    exit(1)
except TypeError:
    pass
d.extend('bcd')
assertEqual(list(d), list('abcd'))
d.extend(d.copy())
assertEqual(list(d), list('abcdabcd'))

###### TEST extend_left() ################

d = deque('a')
try:
    d.extendleft(1)
    printFailed("deque.extendleft", 1)
    exit(1)
except TypeError:
    pass
d.extendleft('bcd')
assertEqual(list(d), list(reversed('abcd')))
d.extendleft(d.copy())
assertEqual(list(d), list('abcddcba'))
d = deque()
d.extendleft(range(1000))
assertEqual(list(d), list(reversed(range(1000))))
try:
    d.extendleft(fail())
    printFailed("deque.extendleft", fail())
    exit(1)
except SyntaxError:
    pass


############ test rotate#############
s = tuple('abcde')
n = len(s)

d = deque(s)
d.rotate(1)             # verify rot(1)
assertEqual(''.join(d), 'eabcd')

d = deque(s)
d.rotate(-1)            # verify rot(-1)
assertEqual(''.join(d), 'bcdea')
d.rotate()              # check default to 1
assertEqual(tuple(d), s)

for i in range(n*3):
    d = deque(s)
    e = deque(d)
    # print(i, d, e)
    d.rotate(i)         # check vs. rot(1) n times
    for j in range(i):
        e.rotate(1)
    assertEqual(tuple(d), tuple(e))
    d.rotate(-i)        # check that it works in reverse
    assertEqual(tuple(d), s)
    e.rotate(n-i)       # check that it wraps forward
    assertEqual(tuple(e), s)

for i in range(n*3):
    d = deque(s)
    e = deque(d)
    d.rotate(-i)
    for j in range(i):
        e.rotate(-1)    # check vs. rot(-1) n times
    assertEqual(tuple(d), tuple(e))
    d.rotate(i)         # check that it works in reverse
    assertEqual(tuple(d), s)
    e.rotate(i-n)       # check that it wraps backaround
    assertEqual(tuple(e), s)

d = deque(s)
e = deque(s)
e.rotate(BIG+17)        # verify on long series of rotates
dr = d.rotate
for i in range(BIG+17):
    dr()
assertEqual(tuple(d), tuple(e))
try:
    d.rotate(1, 2)
    printFailed("deque.rotate", 1, 2)
    exit(1)
except TypeError:
    pass

try:
    d.rotate(1, 10)
    printFailed("deque.rotate", 1, 10)
    exit(1)
except TypeError:
    pass
d = deque()
d.rotate()              # rotate an empty deque
assertEqual(d, deque())


########## test len#############

d = deque('ab')
assertEqual(len(d), 2)
d.popleft()
assertEqual(len(d), 1)
d.pop()
assertEqual(len(d), 0)
try:
    d.pop()
    printFailed("deque.pop")
    exit(1)
except IndexError:
    pass
assertEqual(len(d), 0)
d.append('c')
assertEqual(len(d), 1)
d.appendleft('d')
assertEqual(len(d), 2)
d.clear()
assertEqual(len(d), 0)


############## test underflow#############
d = deque()
try:
    d.pop()
    printFailed("deque.pop")
    exit(1)
except IndexError:
    pass
try:
    d.popleft()
    printFailed("deque.popleft")
    exit(1)
except IndexError:
    pass

############## test clear#############
d = deque(range(100))
assertEqual(len(d), 100)
d.clear()
assertEqual(len(d), 0)
assertEqual(list(d), [])
d.clear()               # clear an empty deque
assertEqual(list(d), [])

# Handle comparison errors
d = deque(['a', 'b', BadCmp(), 'c'])
e = deque(d)

for x, y in zip(d, e):
    # verify that original order and values are retained.
    assertEqual(x is y, True)

########### test repr#############
d = deque(range(200))
e = eval(repr(d))
assertEqual(list(d), list(e))
d.append(None)
assertEqual(repr(d)[-19:], '7, 198, 199, None])')


######### test init #############

try:
    deque('abc', 2, 3)
    printFailed("deque", 'abc', 2, 3)
    exit(1)
except TypeError:
    pass
try:
    deque(1)
    printFailed("deque", 1)
    exit(1)
except TypeError:
    pass


######### test hash #############
try:
    hash(deque('abcd'))
except TypeError:
    pass


###### test long steady state queue pop left ########
for size in (0, 1, 2, 100, 1000):
    d = deque(range(size))
    append, pop = d.append, d.popleft
    for i in range(size, BIG):
        append(i)
        x = pop()
        if x != i - size:
            assertEqual(x, i-size)
    assertEqual(list(d), list(range(BIG-size, BIG)))


######## test long steady state queue pop right ########
for size in (0, 1, 2, 100, 1000):
    d = deque(reversed(range(size)))
    append, pop = d.appendleft, d.pop
    for i in range(size, BIG):
        append(i)
        x = pop()
        if x != i - size:
            assertEqual(x, i-size)
    assertEqual(list(reversed(list(d))),
                list(range(BIG-size, BIG)))

###### test big queue popleft ########
d = deque()
append, pop = d.append, d.popleft
for i in range(BIG):
    append(i)
for i in range(BIG):
    x = pop()
    if x != i:
        assertEqual(x, i)

###### test big queue pop right ########
d = deque()
append, pop = d.appendleft, d.pop
for i in range(BIG):
    append(i)
for i in range(BIG):
    x = pop()
    if x != i:
        assertEqual(x, i)


####### test big stack right########
d = deque()
append, pop = d.append, d.pop
for i in range(BIG):
    append(i)
for i in reversed(range(BIG)):
    x = pop()
    if x != i:
        assertEqual(x, i)
assertEqual(len(d), 0)


##### test big stack left ########
d = deque()
append, pop = d.appendleft, d.popleft
for i in range(BIG):
    append(i)
for i in reversed(range(BIG)):
    x = pop()
    if x != i:
        assertEqual(x, i)
assertEqual(len(d), 0)


##### test roundtrip iter init ########
d = deque(range(200))
e = deque(d)
assertNotEqual(id(d), id(e))
assertEqual(list(d), list(e))


########## test pickle #############

# d = deque(range(200))
# for _ in range(5 + 1):
#     s = pickle.dumps(d)
#     e = pickle.loads(s)
#     assertNotEqual(id(e), id(d))
#     assertEqual(list(e), list(d))

### test copy ########

mut = [10]
d = deque([mut])
e = d.copy()
assertEqual(list(d), list(e))
mut[0] = 11
assertNotEqual(id(d), id(e))
assertEqual(list(d), list(e))

### test reversed#$####

for s in ('abcd', range(2000)):
    assertEqual(list(reversed(deque(s))), list(reversed(s)))

d = deque()
for i in range(100):
    d.append(1)
    gc.collect()
