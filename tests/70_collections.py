from collections import Counter, deque

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