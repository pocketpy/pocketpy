from linalg import mat3x3, vec2, vec3, vec4
import random
import sys
import math



# test vec2--------------------------------------------------------------------

def rotated_vec2(vec_2, radians: float):
    cos_theta = math.cos(radians)
    sin_theta = math.sin(radians)
    new_x = vec_2.x * cos_theta - vec_2.y * sin_theta
    new_y = vec_2.x * sin_theta + vec_2.y * cos_theta
    return vec2(new_x, new_y)

# 生成随机测试目标
min_num = -10.0
max_num = 10.0
test_vec2 = vec2(*tuple([random.uniform(min_num, max_num) for _ in range(2)]))
static_test_vec2_float = vec2(3.1886954323, -1098399.59932453432)
static_test_vec2_int = vec2(278, -13919730938747)

# test __repr__
assert str(static_test_vec2_float) == 'vec2(3.1887, -1.0984e+06)'
assert str(static_test_vec2_int) == 'vec2(278, -1.39197e+13)'

# test copy
element_name_list = [e for e in dir(test_vec2) if e in 'x,y,z,w']
element_value_list = [getattr(test_vec2, attr) for attr in element_name_list]
copy_element_value_list = [getattr(test_vec2.copy(), attr) for attr in element_name_list]
assert element_value_list == copy_element_value_list

# test rotate
test_vec2_copy = test_vec2.copy()
radians = random.uniform(-10*math.pi, 10*math.pi)
test_vec2_copy = rotated_vec2(test_vec2_copy, radians)
assert test_vec2.rotate(radians).__dict__ == test_vec2_copy.__dict__


# test vec3--------------------------------------------------------------------
# 生成随机测试目标
min_num = -10.0
max_num = 10.0
test_vec3 = vec3(*tuple([random.uniform(min_num, max_num) for _ in range(3)]))
static_test_vec3_float = vec3(3.1886954323, -1098399.59932453432, 9.00000000000002765)
static_test_vec3_int = vec3(278, -13919730938747, 1364223456756456)

# test __repr__
assert str(static_test_vec3_float) == 'vec3(3.1887, -1.0984e+06, 9)'
assert str(static_test_vec3_int) == 'vec3(278, -1.39197e+13, 1.36422e+15)'

# test __getnewargs__
element_name_list = [e for e in dir(test_vec3) if e in 'x,y,z,w']
element_value_list = [getattr(test_vec3, attr) for attr in element_name_list]
assert tuple(element_value_list) == test_vec3.__getnewargs__()

# test copy
element_name_list = [e for e in dir(test_vec3) if e in 'x,y,z,w']
element_value_list = [getattr(test_vec3, attr) for attr in element_name_list]
copy_element_value_list = [getattr(test_vec3.copy(), attr) for attr in element_name_list]
assert element_value_list == copy_element_value_list

# test vec4--------------------------------------------------------------------
# 生成随机测试目标
min_num = -10.0
max_num = 10.0
test_vec4 = vec4(*tuple([random.uniform(min_num, max_num) for _ in range(4)]))
static_test_vec4_float = vec4(3.1886954323, -1098399.59932453432, 9.00000000000002765, 4565400000000.0000000045)
static_test_vec4_int = vec4(278, -13919730938747, 1364223456756456, -37)

# test __repr__
assert str(static_test_vec4_float) == 'vec4(3.1887, -1.0984e+06, 9, 4.5654e+12)'
assert str(static_test_vec4_int) == 'vec4(278, -1.39197e+13, 1.36422e+15, -37)'

# test __getnewargs__
element_name_list = [e for e in dir(test_vec4) if e in 'x,y,z,w']
element_value_list = [getattr(test_vec4, attr) for attr in element_name_list]
assert tuple(element_value_list) == test_vec4.__getnewargs__()

# test copy
element_name_list = [e for e in dir(test_vec4) if e in 'x,y,z,w']
element_value_list = [getattr(test_vec4, attr) for attr in element_name_list]
copy_element_value_list = [getattr(test_vec4.copy(), attr) for attr in element_name_list]
assert element_value_list == copy_element_value_list


# test mat3x3--------------------------------------------------------------------
def mat_round(mat, pos):
    '''
    对mat的副本的每一个元素执行round(element, pos)，返回副本
    用于校对元素是浮点数的矩阵
    '''
    ret = mat.copy()
    
    for i, row in enumerate(ret):
        for j, element in enumerate(row):
            row[j] = round(element, pos)
        ret[i] = row
    
    return ret

def get_row(mat, row_index):
    '''
    返回mat的row_index行元素构成的列表
    '''
    ret = []
    for i in range(3):
        ret.append(mat[row_index, i])
    return ret

def get_col(mat, col_index):
    '''
    返回mat的col_index列元素构成的列表
    '''
    ret = []
    for i in range(3):
        ret.append(mat[i, col_index])
    return ret


# 生成随机测试目标
min_num = -10.0
max_num = 10.0
test_mat = mat3x3([[random.uniform(min_num, max_num) for _ in range(3)] for _ in range(3)])
static_test_mat_float= mat3x3([
    [7.264189733952545, -5.432187523625671, 1.8765304152872613],
    [-2.4910524352374734, 8.989660807513068, -0.7168824333280513],
    [9.558042327611506, -3.336280256662496, 4.951381528057387]]
    )

static_test_mat_int = mat3x3([
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 9]]
    )

# test incorrect number of parameters is passed
for i in range(20):
    
    if i in [0, 9]:
        continue
    
    try:
        test_mat_copy = mat3x3(*tuple([e+0.1 for e in range(i)]))
        
        # 既然参数数量不是合法的0个或9个,并且这里也没有触发TypeError,那么引发测试失败
        print(f'When there are {i} arguments, no TypeError is triggered')
        exit(1)
        
    except TypeError:
        pass

# test 9 floating parameters is passed
test_mat_copy = test_mat.copy()
element_name_list = [e for e in dir(test_mat_copy) if e[:2] != '__' and e[0] == '_']
element_value_list = [getattr(test_mat, attr) for attr in element_name_list]
assert mat3x3(*tuple(element_value_list)) == test_mat

        
# test copy
test_mat_copy = test_mat.copy()
assert test_mat is not test_mat_copy
element_name_list = [e for e in dir(test_mat_copy) if e[:2] != '__' and e[0] == '_']
for i, element in enumerate([getattr(test_mat_copy, e) for e in element_name_list]):
    assert [getattr(test_mat, e) for e in element_name_list][i] == element

# test setzeros
test_mat_copy = test_mat.copy()
test_mat_copy.set_zeros()
assert test_mat_copy == mat3x3([[0,0,0],[0,0,0],[0,0,0]])

# test set_ones
test_mat_copy = test_mat.copy()
test_mat_copy.set_ones()
assert test_mat_copy == mat3x3([[1,1,1],[1,1,1],[1,1,1]])

# test set_identity
test_mat_copy = test_mat.copy()
test_mat_copy.set_identity()
assert test_mat_copy == mat3x3([[1, 0, 0],[0, 1, 0],[0, 0, 1]])

# test __getitem__
element_name_list = [e for e in dir(test_mat) if e[:2] != '__' and e[0] == '_']
for i, element in enumerate([getattr(test_mat, e) for e in element_name_list]):
    assert test_mat.__getitem__((int(i/3), i%3)) == element

try:
    test_mat[1,2,3]
    raise Exception('未能触发错误拦截, 此处应当报错 IndexError("index out of range")')
except:
    pass

try:
    test_mat[-1][4]
    raise Exception('未能触发错误拦截, 此处应当报错 IndexError("index out of range")')
except:
    pass

# test __setitem__
test_mat_copy = test_mat.copy()
element_name_list = [e for e in dir(test_mat_copy) if e[:2] != '__' and e[0] == '_']
for i, element in enumerate([getattr(test_mat_copy, e) for e in element_name_list]):
    test_mat_copy.__setitem__((int(i/3), i%3), list(range(9))[i])
assert test_mat_copy == mat3x3([[0,1,2], [3,4,5], [6,7,8]])

try:
    test_mat[1,2,3] = 1
    raise Exception('未能触发错误拦截, 此处应当报错 TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers")')
except:
    pass

try:
    test_mat[-1][4] = 1
    raise Exception('未能触发错误拦截, 此处应当报错 IndexError("index out of range")')
except:
    pass

# test __add__
test_mat_copy = test_mat.copy()
ones = mat3x3()
ones.set_ones()
result_mat = test_mat_copy.__add__(ones)
correct_result_mat = test_mat_copy.copy()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] += 1
assert result_mat == correct_result_mat

# test __sub__
test_mat_copy = test_mat.copy()
ones = mat3x3()
ones.set_ones()
result_mat = test_mat_copy.__sub__(ones)
correct_result_mat = test_mat_copy.copy()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] -= 1
assert result_mat == correct_result_mat

# test __mul__
test_mat_copy = test_mat.copy()
result_mat = test_mat_copy.__mul__(12.345)
correct_result_mat = test_mat_copy.copy()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] *= 12.345
# print(result_mat)
# print(correct_result_mat)
assert result_mat == correct_result_mat


# test matmul
test_mat_copy = test_mat.copy()
test_mat_copy_2 = test_mat.copy()
result_mat = test_mat_copy.matmul(test_mat_copy_2)
correct_result_mat = mat3x3()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] = sum([e1*e2 for e1, e2 in zip(get_row(test_mat_copy, i), get_col(test_mat_copy_2, j))])
assert result_mat == correct_result_mat

# test determinant
test_mat_copy = test_mat.copy()
for i in range(3):
    for j in range(3):
        test_mat_copy[i, j] = test_mat[j, i]

# test __repr__
assert str(static_test_mat_float) == 'mat3x3([[7.2642, -5.4322, 1.8765],\n        [-2.4911, 8.9897, -0.7169],\n        [9.5580, -3.3363, 4.9514]])'
assert str(static_test_mat_int) == 'mat3x3([[1.0000, 2.0000, 3.0000],\n        [4.0000, 5.0000, 6.0000],\n        [7.0000, 8.0000, 9.0000]])'


# test __getnewargs__
test_mat_copy = test_mat.copy()
element_name_list = [e for e in dir(test_mat_copy) if e[:2] != '__' and e[0] == '_']
element_value_list = [getattr(test_mat, attr) for attr in element_name_list]
assert tuple(element_value_list) == test_mat.__getnewargs__()

# test __truediv__
test_mat_copy = test_mat.copy()
result_mat = test_mat_copy.__truediv__(12.345)
correct_result_mat = test_mat_copy.copy()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] /= 12.345
assert result_mat == correct_result_mat



# test __rmul__
test_mat_copy = test_mat.copy()
result_mat = 12.345 * test_mat_copy
correct_result_mat = test_mat_copy.copy()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] *= 12.345

assert result_mat == correct_result_mat


# 此处测试不完全, 未验证正确性
# test interface of "@" "matmul" "__matmul__" with vec3 and error handling
test_mat_copy = test_mat.copy()
test_mat_copy @ vec3(83,-9.12, 0.2983)
try:
    test_mat_copy @ 12345
    raise Exception('未能拦截错误 BinaryOptError("@") 在处理表达式 test_mat_copy @ 12345')