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
    assert a == b
def assertNotEqual(a, b):
    assert a != b
def printFailed(function_name, *args, **kwargs):
    print("X Failed Tests for {} for args: {} {}".format(str(function_name), str(args), str(kwargs)))


BIG = 100000


def fail():
    raise SyntaxError
    yield 1


d = deque(range(-5125, -5000))
# d.__init__(range(200)) # not supported
d = deque(range(200))
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

####### TEST maxlen###############
try:
    dq = deque()
    dq.maxlen = -1
    printFailed("deque.maxlen", -1)
    exit(1)
except AttributeError:
    pass

try:
    dq = deque()
    dq.maxlen = -2
    printFailed("deque.maxlen", -2)
    exit(1)
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

####### TEST maxlen = 0###############
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


####### TEST maxlen attribute #############

assertEqual(deque().maxlen, None)
assertEqual(deque('abc').maxlen, None)
assertEqual(deque('abc', maxlen=4).maxlen, 4)
assertEqual(deque('abc', maxlen=2).maxlen, 2)
assertEqual(deque('abc', maxlen=0).maxlen, 0)
try:
    d = deque('abc')
    d.maxlen = 10
    printFailed("deque.maxlen", 10)
    exit(1)
except AttributeError:
    pass

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


class MutatingCompare:
    def __eq__(self, other):
        d.pop()
        return True


m = MutatingCompare()
d = deque([1, 2, 3, m, 4, 5])
m.d = d

try:
    d.count(3)
    printFailed("deque.count", "MutatingCompare()")
    exit(1)
except RuntimeError:
    pass

d = deque([None]*16)
for i in range(len(d)):
    d.rotate(-1)
d.rotate(1)
assertEqual(d.count(1), 0)
assertEqual(d.count(None), 16)

#### TEST comparisons == #####

d = deque('xabc')
d.popleft()
for e in [d, deque('abc'), deque('ab'), deque(), list(d)]:
    assertEqual(d == e, type(d) == type(e) and list(d) == list(e))
    assertEqual(d != e, not (type(d) == type(e) and list(d) == list(e)))

args = map(deque, ('', 'a', 'b', 'ab', 'ba', 'abc', 'xba', 'xabc', 'cba'))
for x in args:
    for y in args:
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


class MutateCmp:
    def __init__(self, deque, result):
        self.deque = deque
        self.result = result

    def __eq__(self, other):
        self.deque.clear()
        return self.result


# # Test detection of mutation during iteration
d = deque(range(n))
d[n//2] = MutateCmp(d, False)
try:
    n in d
    printFailed("deque.__contains__", n)
    exit(1)
except RuntimeError:
    pass


class BadCmp:
    def __eq__(self, other):
        raise RuntimeError


# # Test detection of comparison exceptions
d = deque(range(n))
d[n//2] = BadCmp()
try:
    n in d
    printFailed("deque.__contains__", n)
    exit(1)
except RuntimeError:
    pass


##### test_contains_count_stop_crashes#####

class A:
    def __eq__(self, other):
        d.clear()
        return NotImplemented


d = deque([A(), A()])

try:
    _ = 3 in d
    printFailed("deque.__contains__", 3)
    exit(1)
except RuntimeError:
    pass

d = deque([A(), A()])
try:
    _ = d.count(3)
    printFailed("deque.count", 3)
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
d.extend(d)
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
d.extendleft(d)
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

##### TEST get_item ################

n = 200
d = deque(range(n))
l = list(range(n))
for i in range(n):
    d.popleft()
    l.pop(0)
    if random.random() < 0.5:
        d.append(i)
        l.append(i)
    for j in range(1-len(l), len(l)):
        assert d[j] == l[j]

d = deque('superman')
assertEqual(d[0], 's')
assertEqual(d[-1], 'n')
d = deque()
try:
    d.__getitem__(0)
    printFailed("deque.__getitem__", 0)
    exit(1)
except IndexError:
    pass
try:
    d.__getitem__(-1)
    printFailed("deque.__getitem__", -1)
    exit(1)
except IndexError:
    pass


######### TEST index()###############
for n in 1, 2, 30, 40, 200:

    d = deque(range(n))
    for i in range(n):
        assertEqual(d.index(i), i)

    try:
        d.index(n+1)
        printFailed("deque.index", n+1)
        exit(1)
    except ValueError:
        pass

    # Test detection of mutation during iteration
    d = deque(range(n))
    d[n//2] = MutateCmp(d, False)

    try:
        d.index(n)
        printFailed("deque.index", n)
        exit(1)
    except RuntimeError:
        pass

    # Test detection of comparison exceptions
    d = deque(range(n))
    d[n//2] = BadCmp()

    try:
        d.index(n)
        printFailed("deque.index", n)
        exit(1)
    except RuntimeError:
        pass


# Test start and stop arguments behavior matches list.index()
# COMMENT: Current List behavior doesn't support start and stop arguments, so this test is not supported
# elements = 'ABCDEFGHI'
# nonelement = 'Z'
# d = deque(elements * 2)
# s = list(elements * 2)
# for start in range(-5 - len(s)*2, 5 + len(s) * 2):
#     for stop in range(-5 - len(s)*2, 5 + len(s) * 2):
#         for element in elements + 'Z':
#             try:
#                 print(element, start, stop)
#                 target = s.index(element, start, stop)
#             except ValueError:
#                 try:
#                     d.index(element, start, stop)
#                     print("X Failed Tests!")
#                     exit(1)
#                 except ValueError:
#                     continue
#                 # with assertRaises(ValueError):
#                 #     d.index(element, start, stop)
#             assertEqual(d.index(element, start, stop), target)


# Test large start argument
d = deque(range(0, 10000, 10))
for step in range(100):
    i = d.index(8500, 700)
    assertEqual(d[i], 8500)
    # Repeat test with a different internal offset
    d.rotate()

########### test_index_bug_24913#############
d = deque('A' * 3)
try:
    d.index('A', 1, 0)
    printFailed("deque.index", 'A', 1, 0)
    exit(1)
except ValueError:
    pass

########### test_insert#############
   # Test to make sure insert behaves like lists
elements = 'ABCDEFGHI'
for i in range(-5 - len(elements)*2, 5 + len(elements) * 2):
    d = deque('ABCDEFGHI')
    s = list('ABCDEFGHI')
    d.insert(i, 'Z')
    s.insert(i, 'Z')
    assertEqual(list(d), s)


########### test_insert_bug_26194#############
data = 'ABC'
d = deque(data, maxlen=len(data))
try:
    d.insert(0, 'Z')
    printFailed("deque.insert", 0, 'Z')
    exit(1)
except IndexError:
    pass

elements = 'ABCDEFGHI'
for i in range(-len(elements), len(elements)):
    d = deque(elements, maxlen=len(elements)+1)
    d.insert(i, 'Z')
    if i >= 0:
        assertEqual(d[i], 'Z')
    else:
        assertEqual(d[i-1], 'Z')


######### test set_item #############
n = 200
d = deque(range(n))
for i in range(n):
    d[i] = 10 * i
assertEqual(list(d), [10*i for i in range(n)])
l = list(d)
for i in range(1-n, 0, -1):
    d[i] = 7*i
    l[i] = 7*i
assertEqual(list(d), l)


########## test del_item #############
n = 500         # O(n**2) test, don't make this too big
d = deque(range(n))
try:
    d.__delitem__(-n-1)
    printFailed("deque.__delitem__", -n-1)
    exit(1)
except IndexError:
    pass

try:
    d.__delitem__(n)
    printFailed("deque.__delitem__", n)
    exit(1)
except IndexError:
    pass
for i in range(n):
    assertEqual(len(d), n-i)
    j = random.randint(0, len(d)-1)
    val = d[j]
    assertEqual(val in d, True)
    del d[j]
    assertEqual(val in d, False)
assertEqual(len(d), 0)


######### test reverse()###############

n = 500         # O(n**2) test, don't make this too big
data = [random.random() for i in range(n)]
for i in range(n):
    d = deque(data[:i])
    r = d.reverse()
    assertEqual(list(d), list(reversed(data[:i])))
    assertEqual(r, None)
    d.reverse()
    assertEqual(list(d), data[:i])
try:
    d.reverse(1)
    printFailed("deque.reverse", 1)
    exit(1)
except TypeError:
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


############# test remove#############
d = deque('abcdefghcij')
d.remove('c')
assertEqual(d, deque('abdefghcij'))
d.remove('c')
assertEqual(d, deque('abdefghij'))
try:
    d.remove('c')
    printFailed("deque.remove", "c")
    exit(1)
except ValueError:
    pass
assertEqual(d, deque('abdefghij'))

# Handle comparison errors
d = deque(['a', 'b', BadCmp(), 'c'])
e = deque(d)

try:
    d.remove('c')
    printFailed("deque.remove", "c")
    exit(1)
except RuntimeError:
    pass
for x, y in zip(d, e):
    # verify that original order and values are retained.
    assertEqual(x is y, True)

# Handle evil mutator
for match in (True, False):
    d = deque(['ab'])
    d.extend([MutateCmp(d, match), 'c'])
    try:
        d.remove('c')
        printFailed("deque.remove", "c")
        exit(1)
    except IndexError:
        pass
    assertEqual(d, deque())


########### test repr#############
d = deque(range(200))
e = eval(repr(d))
assertEqual(list(d), list(e))
d.append(d)
assertEqual(repr(d)[-20:], '7, 198, 199, [...]])')


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

for d in deque(range(200)), deque(range(200), 100):
    for i in range(5 + 1):
        s = pickle.dumps(d)
        e = pickle.loads(s)
        assertNotEqual(id(e), id(d))
        assertEqual(list(e), list(d))
        assertEqual(e.maxlen, d.maxlen)

######## test pickle recursive ########
# the following doesn't work because the pickle module doesn't
# for d in deque('abc'), deque('abc', 3):
#     d.append(d)
#     for i in range(5 + 1):
#         e = pickle.loads(pickle.dumps(d))
#         assertNotEqual(id(e), id(d))
#         assertEqual(id(e[-1]), id(e))
#         assertEqual(e.maxlen, d.maxlen)


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


# probably not supported
# klass = type(reversed(deque()))
# for s in ('abcd', range(2000)):
#     assertEqual(list(klass(deque(s))), list(reversed(s)))

d = deque()
for i in range(100):
    d.append(1)
    gc.collect()
