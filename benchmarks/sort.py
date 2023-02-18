import random

a = [random.randint(-100000, 100000) for i in range(100000)]

def __qsort(a: list, L: int, R: int):
    if L >= R: return;
    mid = a[(R+L)//2];
    i, j = L, R
    while i<=j:
        while a[i]<mid: i+=1
        while a[j]>mid: j-=1
        if i<=j:
            a[i], a[j] = a[j], a[i]
            i+=1
            j-=1
    __qsort(a, L, j)
    __qsort(a, i, R)

from dis import dis
# dis(__qsort)

__qsort(a, 0, len(a)-1)