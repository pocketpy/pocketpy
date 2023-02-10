import random

a = [random.randint(-100000, 100000) for i in range(100000)]

def __qsort(a: list, i: int, j: int):
    if i>=j:
        return
    d1, d2 = i, j
    mid = (i+j) // 2
    a[mid], a[i] = a[i], a[mid]
    u = a[i];
    while i<j:
        while i<j and a[j]>u:
            j--
        if i<j:
            a[i] = a[j]
            i++
        while i<j and a[i]<u:
            i++
        if i<j:
            a[j] = a[i]
            j--
    a[i] = u;
    __qsort(a, d1, i-1)
    __qsort(a, i+1, d2)

from dis import dis
dis(__qsort)

__qsort(a, 0, len(a)-1)