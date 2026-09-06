class ActorCpnt:
	def __init__(self, actor, id):
		self.actor = actor
		self.id = id
		
class Actor:
	def __init__(self, id):
		self.cpnts = [ActorCpnt(self, id)]

import pickle as pkl
a = Actor(123)
x = pkl.dumps(a)
b = pkl.loads(x)

assert isinstance(b, Actor)
assert len(b.cpnts) == 1
assert b.cpnts[0].actor is b
assert b.cpnts[0].id == 123
# self-referencing list
a = [1, 2]
a.append(a)
b = pkl.loads(pkl.dumps(a))
assert b[0] == 1 and b[1] == 2
assert b[2] is b

# self-referencing dict
d = {'k': 1}
d['me'] = d
e = pkl.loads(pkl.dumps(d))
assert e['k'] == 1
assert e['me'] is e

# two objects referencing each other
class P:
	def __init__(self):
		self.q = None
class Q:
	def __init__(self):
		self.p = None

p = P()
q = Q()
p.q = q
q.p = p
p2 = pkl.loads(pkl.dumps(p))
assert isinstance(p2, P)
assert isinstance(p2.q, Q)
assert p2.q.p is p2

# object -> dict -> object
class Node:
	def __init__(self):
		self.edges = {}

n = Node()
n.edges['self'] = n
n2 = pkl.loads(pkl.dumps(n))
assert n2.edges['self'] is n2

# longer cycle: list -> dict -> list
L = []
D = {'L': L}
L.append(D)
L2 = pkl.loads(pkl.dumps(L))
assert L2[0]['L'] is L2

# a shared object appearing both inside and outside a cycle
class Box:
	def __init__(self, v):
		self.v = v

shared = Box(7)
root = {'a': shared, 'b': [shared]}
root['self'] = root
r = pkl.loads(pkl.dumps(root))
assert r['self'] is r
assert r['a'] is r['b'][0]
assert r['a'].v == 7

# empty containers
assert pkl.loads(pkl.dumps([])) == []
assert pkl.loads(pkl.dumps({})) == {}
class Empty:
	pass
assert isinstance(pkl.loads(pkl.dumps(Empty())), Empty)
