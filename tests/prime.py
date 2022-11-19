def is_prime(x):
  if x<2:
    return False
  for i in range(2,x):
    if x%i == 0:
      return False
  return True

def test(n):
  k = 0
  for i in range(n):
    if is_prime(i):
      k += 1
  return k

assert test(100) == 25