import random

def f(x):
  if x >= 5 :
    return x+5
  else :
    return -x+5

def create_init_DNA(min,max):
# out: DNA(['0','1',....])
  ret = random.randint(min,max)
  return int_to_bin(ret)

def int_to_bin(x):
# in: int_DNA(int)
# out: DNA
  ret = []
  if x >= 0:
    ret.append(0)
  else:
    ret.append(1)
    x = -x
  for i in [32768,16384,8192,4096,2048,1024,512,256,128,64,32,16,8,4,2,1] :
    if x>=i :
      ret.append(1)
      x -= i
    else :
      ret.append(0)
  return ret

def bin_to_int(x):
# in: DNA
# out: int_DNA(int)
  ret = 0
  new_x = x[:]
  flag = -(int(new_x[0])*2-1)
  mul = 1
  new_x = reversed(new_x)
  new_x.pop()
  for i in new_x :
    ret += flag * int(i) * mul
    mul *= 2
  return ret

def reversed(x):
  ret = []
  for i in range(0,len(x)):
    ret.append(x[-i-1])
  return ret

def create_DNAs(num,min,max):
# in: num(int)
# out: DNAs([DNA,DNA,...])
  ret = []
  for i in range(num):
    ret.append(create_init_DNA(min,max))
  return ret

def bins_to_ints(x):
# in: DNAs
# out: int_DNAs([int,int,...])
  ret = []
  for i in x:
    ret.append(bin_to_int(i))
  return ret

def ints_to_bins(x):
# in: int_DNAs
# out: DNAs
  ret = []
  for i in x:
    ret.append(int_to_bin(i))
  return ret

def create_probabilitys(x):
# in: DNAs
# out: probabilitys([float,float...])
  scores = survival_scores(x)
  mid = []
  ret = []
  sum = 0
  for i in scores:
    sum+=i
  for i in scores:
    mid.append(1/(i+0.0/sum))
  sum = 0
  for i in mid:
    sum+=i
  for i in mid:
    ret.append(i/sum)
  return ret

def survival_scores(x):
# in: DNAs
# out: survival_scores[f(int_DNA),...]
  ret = []
  for i in x:
    ret.append(f(bin_to_int(i)/100))
  return ret

def choose_DNA(DNAs,probabilitys):
# in: DNAs,probabilitys
# out: choosen_DNA(DNA)
# probabilitys是由若干(0，1)之间的浮点数组成的数组，这些浮点数的和为1
  i = 0 # i记录取出元素的位置
  ran = random.random()
  max_sum = 0
  for max in probabilitys :
    max_sum += max
    if i != 0 :
      min_sum += probabilitys[i-1]
    else :
      min_sum = 0
    if (ran < max_sum) and (ran >= min_sum) :
      return DNAs[i]
    i += 1

def mating_DNAs(DNAs,num=None):
# in: DNAs,num(int)交配组DNA数量
# out: mating_DNAs(DNAs)交配组
  num = num or len(DNAs)
  ret = []
  for i in range(num) :
    ret.append(choose_DNA(DNAs,create_probabilitys(DNAs)))
  return ret

def son_DNA(DNAs):
# in: mating_DNAs(DNAs)交配组
# out: son_DNA(DNA)交叉互换后产生的子代
  father = DNAs[random.randint(0,len(DNAs)-1)]
  mother = DNAs[random.randint(0,len(DNAs)-1)]
  pos = random.randint(0,len(DNAs[0])-1)
  son_DNA = []
  son_DNA += father[0:pos]
  son_DNA += mother[pos+1:-1]
  return son_DNA

def mutation(DNA,part_rate):
# in: son_DNA(DNA)交叉互换后的子代,part_rate(float)碱基对变异概率
# out: mut_DNA(DNA)变异后的子代
  ret = DNA[:]
  for i in range(len(ret)):
    k = random.random()
    if k < part_rate :
      ret[i] = str(-int(ret[i])+1)
  return ret

def next_DNAs(DNAs,father_rate,part_rate,mut_rate,num):
# in: mating_DNAs(DNAs),father_rate(float)父代保留占比,part_rate(float)碱基对突变概率,mut_rate(float)生物变异概率,num(int)下一代数量
# out: next_DNAs(DNAs)
  ret = []
  pro_DNAs = create_probabilitys(DNAs)
  father_num = int(father_rate*num+0.5)
  son_num = num - father_num
  i = 1
  for pro in reversed(sorted(pro_DNAs)):
    if i > father_num:
      break
    ret.append(DNAs[pro_DNAs.index(pro)])
    i += 1
  for s in range(son_num):
    k = random.random()
    son_DNA=(mating_DNAs(DNAs)[0])
    if k < mut_rate :
      son_DNA = mutation(son_DNA,part_rate)
    ret.append(son_DNA)
  return ret

def iterate(DNAs,father_rate,part_rate,mut_rate,iter_num,population):
# in: DNAs父代,father_rate(float)每一次迭代保留父代的占比, num(int)迭代次数,population(int)种群数量
# out: DNAs子代
  ret = eval(repr(DNAs))
  for i in range(iter_num):
    ret = next_DNAs(ret,father_rate,part_rate,mut_rate,population)
  return ret


def 百分比化(a):
  b = str(a*100)[0:5]
  return f'{b}%'

a = create_DNAs(5,-9999,9999)



