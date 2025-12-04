from bisect import bisect_left, bisect_right, insort_left, insort_right

a = [1, 1, 2, 6, 7, 8, 16, 22]

assert bisect_left(a, 23) == 8  # 23 does not exist in the list
assert bisect_left(a, 22) == 7  # 22 exists in the list
assert bisect_left(a, 0) == 0   # 0 does not exist in the list
assert bisect_left(a, 1) == 0   # 1 exists in the list
assert bisect_left(a, 5) == 3   # 5 does not exist in the list
assert bisect_left(a, 6) == 3   # 6 does exist in the list

# test bisect_right
assert bisect_right(a, 23) == 8  # 23 does not exist in the list
assert bisect_right(a, 22) == 8  # 22 exists in the list
assert bisect_right(a, 0) == 0   # 0 does not exist in the list
assert bisect_right(a, 1) == 2   # 1 exists in the list
assert bisect_right(a, 5) == 3   # 5 does not exist in the list
assert bisect_right(a, 6) == 4   # 6 does exist in the list

# test insort_left
insort_left(a, 23)
assert a == [1, 1, 2, 6, 7, 8, 16, 22, 23]

insort_left(a, 0)
assert a == [0, 1, 1, 2, 6, 7, 8, 16, 22, 23]

insort_left(a, 5)
assert a == [0, 1, 1, 2, 5, 6, 7, 8, 16, 22, 23]

insort_left(a, 1)
assert a == [0, 1, 1, 1, 2, 5, 6, 7, 8, 16, 22, 23]

# test insort_right
insort_right(a, 23)
assert a == [0, 1, 1, 1, 2, 5, 6, 7, 8, 16, 22, 23, 23]

insort_right(a, 0)
assert a == [0, 0, 1, 1, 1, 2, 5, 6, 7, 8, 16, 22, 23, 23]

insort_right(a, 5)
assert a == [0, 0, 1, 1, 1, 2, 5, 5, 6, 7, 8, 16, 22, 23, 23]

insort_right(a, 1)
assert a == [0, 0, 1, 1, 1, 1, 2, 5, 5, 6, 7, 8, 16, 22, 23, 23]