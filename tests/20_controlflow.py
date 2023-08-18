# if tests
flag = False
name = 'luren'
if name == 'python':         
    flag = True              
else:
    flag               
assert flag == False


num = 9
flag = 0
if num >= 0 and num <= 10:    
    flag = 1
else:
    flag
assert flag == 1

num = 10
flag = 0
if num < 0 or num > 10:    
    flag = 1
else:
    flag
assert flag == 0

num = 5
result = 0     
if num == 3:            
    result = num        
elif num == 2:
    result = num
elif num == 1:
    result = num
elif num < 0:           
    result = num
else:
    result = num
assert result == 5    

# for tests 

k = 0
for i in range(2, 1000):
  if i % 2 == 0:
    k += 1
assert k ==499

k = 0
for x in range(100):
    if x<2:
      continue
    flag = True
    for i in range(2,x):
      if x%i == 0:
        flag = False
        break
    if flag:
      k += 1
assert k == 25

#while tests
count = 0
while (count < 1000):
   count = count + 1
assert count == 1000

# ternary operator
d = 1 if 2 > 1 else 2
assert d == 1
d = 1 if 2 < 1 else 2
assert d == 2

t = 0
for i in range(5):
    try:
        break
    except:
        pass
    t = 1
assert t == 0

t = 0
for i in range(5):
    if True and 1:
        break
    t = 1
assert t == 0

for i in range(5):
    break
else:
    assert False

for i in range(5):
    if i==3:
        break
else:
    assert False

flag = False
for i in range(5):
    if i==6:
        break
else:
    flag = True
assert flag is True

while True:
    break
else:
    assert False

flag = False
while False:
    assert False
else:
    flag = True
assert flag is True

x = 1
while 0:
    while True:
        break
else:
    x = 2
assert x == 2

if x == 2:
    while 0:
        pass
else:
    x = 3
assert x == 2