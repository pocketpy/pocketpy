def test(n):
  k = 0
  for x in range(n):
    if x<2:
      continue
    flag = True
    for i in range(2,x):
      if x%i == 0:
        flag = False
        break
    if flag:
      k += 1
  return k

print(test(10000))