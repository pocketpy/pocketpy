from linalg import mat3x3, vec2, vec3
import random
import sys

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
# test_mat = mat3x3([
#         [1, 2, 3],
#         [4, 5, 6],
#         [7, 8, 9]]
#     )

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

# test __setitem__
test_mat_copy = test_mat.copy()
element_name_list = [e for e in dir(test_mat_copy) if e[:2] != '__' and e[0] == '_']
for i, element in enumerate([getattr(test_mat_copy, e) for e in element_name_list]):
    test_mat_copy.__setitem__((int(i/3), i%3), list(range(9))[i])
assert test_mat_copy == mat3x3([[0,1,2], [3,4,5], [6,7,8]])

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
test_mat_copy = test_mat.copy()
print(test_mat_copy[0,0])
assert test_mat_copy.__repr__() == f'mat3x3([[{test_mat_copy[0,0].round(4)}, {test_mat_copy[1,0].round(4)}, {test_mat_copy[2,0].round(4)}],\n        [{test_mat_copy[0,1].round(4)}, {test_mat_copy[1,1].round(4)}, {test_mat_copy[2,1].round(4)}],\n        [{test_mat_copy[0,2].round(4)}, {test_mat_copy[1,2].round(4)}, {test_mat_copy[2,2].round(4)}]])'
