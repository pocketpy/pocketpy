import random
import builtins

builtins.print = lambda *x: None

def f(x):
  return -x**2+10

def create_init_DNA():
# out: DNA
  ret = random.randint(-100,100)
  return int_to_bin(ret)

def int_to_bin(x):
# in: int_DNA
# out: DNA
  ret = []
  if x >= 0:
    ret.append(0)
  else:
    ret.append(1)
    x = -x
  for i in [4096,2048,1024,512,256,128,64,32,16,8,4,2,1] :
    if x>=i :
      ret.append(1)
      x -= i
    else :
      ret.append(0)
  return ret

def bin_to_int(x):
# in: DNA
# out: int_DNA
  ret = 0
  new_x = x[:]
  flag = -(new_x[0]*2-1)
  mul = 1
  new_x = reversed(new_x)
  new_x.pop()
  for i in new_x :
    ret += flag * i * mul
    mul *= 2
  return ret

def reversed(x):
  ret = []
  for i in range(0,len(x)):
    ret.append(x[-i-1])
  return ret

def create_DNAs(num):
# in: int
# out: DNAs
  ret = []
  for i in range(num):
    ret.append(create_init_DNA())
  return ret

def bins_to_ints(x):
# in: DNAs
# out: int_DNAs
  ret = []
  for i in x:
    ret.append(bin_to_int(i))
  return ret

def create_probabilitys(x):
# in: DNAs
# out: probabilitys
  scores = survival_scores(x)
  mid = []
  ret = []
  sum = 0
  for i in scores:
    sum+=i
  for i in scores:
    mid.append(1/(i/sum))
  sum = 0
  for i in mid:
    sum+=i
  for i in mid:
    ret.append(i/sum)
  return ret

def survival_scores(x):
# in: DNAs
# out: survival_scores
  ret = []
  for i in x:
    ret.append(f(bin_to_int(i)))
  return ret

def choose_DNA(DNAs,probabilitys):
# in: DNAs,probabilitys
# out: DNA
# probabilitys是由若干(0，1)之间的浮点数组成的数组，这些浮点数的和为1
  i = 0 # i记录取出元素的位置
  ran = random.random()
# a=0
  for max in probabilitys :
    if i != 0 :
      min = probabilitys[i-1]
    else :
      min = 0
    if ran < max and ran >= min :
      return DNAs[i]


def next_gen_DNAs(DNAs,num=None):
  num = num or len(DNAs)
  ret = []
  for i in range(num) :
    ret.append(choose_DNA(DNAs,create_probabilitys(DNAs)))


a = create_DNAs(10)
print(a)
print(bins_to_ints(a))
print(survival_scores(a))
print(create_probabilitys(a))