from linalg import mat3x3, vec2, vec3, vec4
import random
import sys
import math

assert repr(math) == "<module 'math'>"

# 出于对精度转换的考虑,在本测试中具体将采用str(floating_num)[:6]来比较两个浮点数是否相等

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
test_vec2_2 = vec2(*tuple([random.uniform(min_num, max_num) for _ in range(2)]))
static_test_vec2_float = vec2(3.18, -1.09)
static_test_vec2_int = vec2(278, -1391)

# test __repr__
assert str(static_test_vec2_float).startswith('vec2(')
assert str(static_test_vec2_int).startswith('vec2(')

# test copy
element_name_list = [e for e in dir(test_vec2) if e in 'x,y,z,w']
element_value_list = [getattr(test_vec2, attr) for attr in element_name_list]
copy_element_value_list = [getattr(test_vec2.copy(), attr) for attr in element_name_list]
assert element_value_list == copy_element_value_list

# test rotate
test_vec2_copy = test_vec2.copy()
radians = random.uniform(-10*math.pi, 10*math.pi)
test_vec2_copy = rotated_vec2(test_vec2_copy, radians)
assert test_vec2.rotate(radians) == test_vec2_copy
test_vec2.rotate_(radians)
assert test_vec2 == test_vec2_copy

# test smooth_damp
vel = vec2(0, 0)
ret = vec2.smooth_damp(vec2(1, 2), vec2(3, 4), vel, 0.2, 0.001, 0.05)
assert isinstance(ret, vec2)
assert vel.length() > 0

# test vec3--------------------------------------------------------------------
# 生成随机测试目标
min_num = -10.0
max_num = 10.0
test_vec3 = vec3(*tuple([random.uniform(min_num, max_num) for _ in range(3)]))
static_test_vec3_float = vec3(3.1886954323, -1098399.59932453432, 9.00000000000002765)
static_test_vec3_int = vec3(278, -13919730938747, 1364223456756456)

# test __repr__
assert str(static_test_vec3_float).startswith('vec3(')
assert str(static_test_vec3_int).startswith('vec3(')

# test copy
element_name_list = ['x', 'y', 'z']
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
assert str(static_test_vec4_float).startswith('vec4(')
assert str(static_test_vec4_int).startswith('vec4(')

# test copy
element_name_list = ['x', 'y', 'z', 'w']
element_value_list = [getattr(test_vec4, attr) for attr in element_name_list]
copy_element_value_list = [getattr(test_vec4.copy(), attr) for attr in element_name_list]
assert element_value_list == copy_element_value_list


# test mat3x3--------------------------------------------------------------------
def mat_to_str_list(mat):
    ret = [[0,0,0], [0,0,0], [0,0,0]]
    for i in range(3):
        for j in range(3):
            ret[i][j] = str(round(mat[i, j], 2))[:6]
    return ret

def mat_list_to_str_list(mat_list):
    ret = [[0,0,0], [0,0,0], [0,0,0]]
    for i in range(3):
        for j in range(3):
            ret[i][j] = str(round(mat_list[i][j], 2))[:6]
    return ret

def mat_to_list(mat):
    ret = [[0,0,0], [0,0,0], [0,0,0]]
    for i in range(3):
        for j in range(3):
            ret[i][j] = mat[i, j]
    return ret
    
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

def calculate_inverse(matrix):
    '''
    返回逆矩阵
    '''
    # 获取矩阵的行数和列数
    rows = len(matrix)
    cols = len(matrix[0])
    # 确保矩阵是方阵
    if rows != cols:
        raise ValueError("输入矩阵必须是方阵")
    # 构建单位矩阵
    identity = [[1 if i == j else 0 for j in range(cols)] for i in range(rows)]
    # 将单位矩阵与输入矩阵进行初等行变换
    augmented_matrix = [row + identity[i] for i, row in enumerate(matrix)]
    # 初等行变换，将输入矩阵转化为单位矩阵，同时在另一边进行相同的行变换
    for i in range(cols):
        pivot = augmented_matrix[i][i]
        if pivot == 0:
            raise ValueError("输入矩阵不可逆")
        scale_row(augmented_matrix, i, 1/pivot)
        for j in range(cols):
            if j != i:
                scale = augmented_matrix[j][i]
                row_operation(augmented_matrix, j, i, -scale)
    # 提取逆矩阵
    inverse_matrix = [row[cols:] for row in augmented_matrix]
    return inverse_matrix

def scale_row(matrix, row, scale):
    matrix[row] = [element * scale for element in matrix[row]]

def row_operation(matrix, target_row, source_row, scale):
    matrix[target_row] = [target_element + scale * source_element for target_element, source_element in zip(matrix[target_row], matrix[source_row])]
# 生成随机测试目标
min_num = -10.0
max_num = 10.0
test_mat = mat3x3([random.uniform(min_num, max_num) for _ in range(9)])
static_test_mat_float= mat3x3(
    7.264189733952545, -5.432187523625671, 1.8765304152872613,
    -2.4910524352374734, 8.989660807513068, -0.7168824333280513,
    9.558042327611506, -3.336280256662496, 4.951381528057387
)

static_test_mat_float_inv = mat3x3( 0.32265243,  0.15808159, -0.09939472,
        0.04199553,  0.13813096,  0.00408326,
       -0.59454451, -0.21208362,  0.39658464)

static_test_mat_int = mat3x3([
        1, 2, 3,
        4, 5, 6,
        7, 8, 9]
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
element_name_list = []
for i in range(3):
    for j in range(3):
        element_name_list.append(f'_{i+1}{j+1}')
element_value_list = [getattr(test_mat, attr) for attr in element_name_list]
assert mat3x3(*tuple(element_value_list)) == test_mat

        
# test copy
test_mat_copy = test_mat.copy()
assert test_mat is not test_mat_copy
assert test_mat == test_mat_copy

# test __getitem__
for i, element in enumerate([getattr(test_mat, e) for e in element_name_list]):
    assert test_mat[int(i/3), i%3] == element

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
for i, element in enumerate([getattr(test_mat_copy, e) for e in element_name_list]):
    test_mat_copy[int(i/3), i%3] = list(range(9))[i]
assert test_mat_copy == mat3x3([0,1,2,
                                3,4,5,
                                6,7,8])

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
ones = mat3x3.ones()
result_mat = test_mat_copy.__add__(ones)
correct_result_mat = test_mat_copy.copy()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] += 1
assert result_mat == correct_result_mat

# test __sub__
test_mat_copy = test_mat.copy()
ones = mat3x3.ones()
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
result_mat = test_mat_copy @ test_mat_copy_2
correct_result_mat = mat3x3()
for i in range(3):
    for j in range(3):
        correct_result_mat[i, j] = sum([e1*e2 for e1, e2 in zip(get_row(test_mat_copy, i), get_col(test_mat_copy_2, j))])
assert result_mat == correct_result_mat

# test determinant
test_mat_copy = test_mat.copy()
test_mat_copy.determinant()

# test __repr__
assert str(static_test_mat_float)
assert str(static_test_mat_int)

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
    exit(1)
except TypeError:
    pass


# test transpose
test_mat_copy = test_mat.copy()
assert test_mat_copy.transpose_() is None
assert test_mat_copy == test_mat.transpose()
assert test_mat_copy.transpose() == test_mat_copy.transpose().transpose().transpose()

# test inverse
assert ~static_test_mat_float == static_test_mat_float_inv == static_test_mat_float.invert()
assert static_test_mat_float.invert_() is None
assert static_test_mat_float == static_test_mat_float_inv

try:
    ~mat3x3([1, 2, 3, 2, 4, 6, 3, 6, 9])
    raise Exception('未能拦截错误 ValueError("matrix is not invertible") 在 test_mat_copy 的行列式为0')
except ValueError:
    pass

# test zeros
assert mat3x3([0 for _ in range(9)]) == mat3x3.zeros()

# test ones
assert mat3x3([1 for _ in range(9)]) == mat3x3.ones()

# test identity
assert mat3x3([1,0,0,0,1,0,0,0,1]) == mat3x3.identity()


# test affine transformations-----------------------------------------------
# test trs
def trs(t, radian, s):
    cr = math.cos(radian)
    sr = math.sin(radian)
    elements = [[s[0] * cr, -s[1] * sr, t[0]],
                [s[0] * sr, s[1] * cr, t[1]],
                [0.0, 0.0, 1.0]]
    return elements

test_vec2_copy = test_vec2.copy()
test_vec2_2_copy = test_vec2_2.copy()

test_vec2_list = [test_vec2_copy.x, test_vec2_copy.y]
test_vec2_2_list = [test_vec2_2_copy.x, test_vec2_2_copy.y]

radian = random.uniform(-10*math.pi, 10*math.pi)

mat3x3.trs(test_vec2_copy, radian, test_vec2_2_copy)

a = mat3x3.zeros()
a.copy_trs_(test_vec2_copy, radian, test_vec2_2_copy)
assert a == mat3x3.trs(test_vec2_copy, radian, test_vec2_2_copy)
b = mat3x3.identity()
b.copy_t_(test_vec2_copy)
b.copy_r_(radian)
b.copy_s_(test_vec2_2_copy)
assert a == b

# test is_affine
def mat_is_affine(mat_list):
    return mat_list[2][0] == 0 and mat_list[2][1] == 0 and mat_list[2][2] == 1

# 通过random.unifrom的返回值不可能是整数0或1, 因此认为test_mat不可能is_affine
test_mat_copy = test_mat.copy()
assert test_mat_copy.is_affine() == mat_is_affine(mat_to_list(test_mat_copy))

test_mat_copy[2,0] = 0
test_mat_copy[2,1] = 0
test_mat_copy[2,2] = 1
assert test_mat_copy.is_affine() == mat_is_affine(mat_to_list(test_mat_copy))


# test translation
test_mat_copy = test_mat.copy()
assert test_mat_copy._t() == vec2(test_mat_copy[0, 2], test_mat_copy[1, 2])

# 该方法的测试未验证计算的准确性
# test rotation
test_mat_copy = test_mat.copy()
assert type(test_mat_copy._r()) is float


# test scale
test_mat_copy = test_mat.copy()
temp_vec2 = test_mat_copy._s()

# test transform_point
test_mat_copy = test_mat.copy()
test_mat_copy = test_mat.copy()
test_vec2_copy = test_vec2.copy()
temp_vec2 = test_mat_copy.transform_point(test_vec2_copy)

# test transform_vector
test_mat_copy = test_mat.copy()
test_mat_copy = test_mat.copy()
test_vec2_copy = test_vec2.copy()
temp_vec2 = test_mat_copy.transform_vector(test_vec2_copy)

import c
a = vec2(1, 2)
b = a.to_struct()
assert a.sizeof() == 8
assert b.sizeof() == 8
assert vec2.from_struct(b) == a

val = vec2.angle(vec2(-1, 0), vec2(0, -1))
assert 1.57 < val < 1.58

# test about staticmethod
class mymat3x3(mat3x3):
    def f(self):
        _0 = self.zeros()
        _1 = super().zeros()
        _2 = mat3x3.zeros()
        return _0 == _1 == _2
    
assert mymat3x3().f()


# test assign
a = vec2(1, 2)
assert a.copy_(vec2(3, 4)) is None
assert a == vec2(3, 4)

b = vec3(1, 2, 3)
assert b.copy_(vec3(4, 5, 6)) is None
assert b == vec3(4, 5, 6)

c = vec4(1, 2, 3, 4)
assert c.copy_(vec4(5, 6, 7, 8)) is None
assert c == vec4(5, 6, 7, 8)

d = mat3x3.identity()
assert d.copy_(mat3x3.zeros()) is None
assert d == mat3x3.zeros()

d = mat3x3.identity()
assert d.matmul(mat3x3.zeros()) == mat3x3.zeros()
assert d == mat3x3.identity()
assert d.matmul(mat3x3.zeros(), out=d) is None
assert d == mat3x3.zeros()

try:
    assert d[6, 6]
    exit(1)
except IndexError:
    pass

# test vec * vec
assert vec2(1, 2) * vec2(3, 4) == vec2(3, 8)
assert vec3(1, 2, 3) * vec3(4, 5, 6) == vec3(4, 10, 18)
assert vec4(1, 2, 3, 4) * vec4(5, 6, 7, 8) == vec4(5, 12, 21, 32)

# test vec.__getitem__
assert vec2(1, 2)[0] == 1 and vec2(1, 2)[1] == 2
assert vec3(1, 2, 3)[0] == 1 and vec3(1, 2, 3)[1] == 2 and vec3(1, 2, 3)[2] == 3
assert vec4(1, 2, 3, 4)[0] == 1 and vec4(1, 2, 3, 4)[1] == 2 and vec4(1, 2, 3, 4)[2] == 3 and vec4(1, 2, 3, 4)[3] == 4
