assert 1<2
assert 1+1==2
assert 2+1>=2

assert 1<2<3
assert 1<2<3<4
assert 1<2<3<4<5

assert 1<1+1<3
assert 1<1+1<3<4
assert 1<1+1<3<2+2<5

a = [1,2,3]
assert a[0] < a[1] < a[2]
assert a[0]+1 == a[1] < a[2]
assert a[0]+1 == a[1] < a[2]+1 < 5

assert (4>3<2) == False
# assert ((4>3)<2) == True