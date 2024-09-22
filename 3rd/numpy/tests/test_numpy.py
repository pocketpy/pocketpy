import math
import sys
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    import numpy as np
else:
    if sys.platform == 'win32':
        path = 'E:/pocketpy/3rd/numpy/build/Release/numpy.dll'
    else:
        path = '/mnt/e/pocketpy/3rd/numpy/build/libnumpy.so'
    np = __import__(path)

def assert_equal(a, b):
    assert (a == b).all()


# test dtypes
assert hasattr(np, 'int8')
assert hasattr(np, 'int16')
assert hasattr(np, 'int32')
assert hasattr(np, 'int64')
assert hasattr(np, 'int_')
assert hasattr(np, 'float32')
assert hasattr(np, 'float64')
assert hasattr(np, 'float_')
assert hasattr(np, 'bool_')


# test array int
arr1 = np.array([])
arr2 = np.array(10)
arr3 = np.array([-2, -1, 0, 1, 2])
arr4 = np.array([[1, 2], [2, 1]])
arr5 = np.array([[[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]]])
arr6 = np.array([[[[[1], [10], [100], [1000], [10000]]]]])
arr7 = np.array([[[2147483647]]])

arr8 = np.array([1, 2, 3, 4, 5], dtype='int8')
arr8 = np.array([1, 2, 3, 4, 5], np.int8)

arr9 = np.array([1, 2, 3, 4, 5], dtype='int16')
arr9 = np.array([1, 2, 3, 4, 5], np.int16)

arr10 = np.array([1, 2, 3, 4, 5], dtype='int32')
arr10 = np.array([1, 2, 3, 4, 5], np.int32)

arr11 = np.array([1, 2, 3, 4, 5], dtype='int64')
arr11 = np.array([1, 2, 3, 4, 5], np.int64)


# test array bool
arr1 = np.array([True, False, True, False])
arr2 = np.array([[True, False], [False, True]])
arr3 = np.array([[[True, False, True], [False, True, False]], [[True, False, True], [False, True, False]]])

arr4 = np.array([[1, 0], [0, 1]], dtype='bool')
arr4 = np.array([[1, 0], [0, 1]], np.bool_)


# test array float
arr1 = np.array([0.123456789])
arr2 = np.array([-2.0, -1.0, 0.0, 1.0, 2.0])
arr3 = np.array([[1.0, 2.0], [2.0, 1.0]])
arr4 = np.array([[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], [[7.0, 8.0, 9.0], [10.0, 11.0, 12.0]]])
arr5 = np.array([[[[[1.0], [10.0], [100.0], [1000.0], [10000.0]]]]])
arr6 = np.array([[[3.141592653589793]]])

arr7 = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype='float32')
arr7 = np.array([1.0, 2.0, 3.0, 4.0, 5.0], np.float32)

arr8 = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype='float64')
arr8 = np.array([1.0, 2.0, 3.0, 4.0, 5.0], np.float64)


# test array creation
assert_equal(np.ones([1, 1]), np.array([[1.0]]))
assert_equal(np.ones([1, 1, 2, 2]), np.array([[[[1.0, 1.0], [1.0, 1.0]]]]))

assert_equal(np.zeros([1, 1]), np.array([[0.0]]))
assert_equal(np.zeros([1, 1, 2, 2]), np.array([[[[0.0, 0.0], [0.0, 0.0]]]]))

assert_equal(np.full([1, 1], -1e9), np.array([[-1.000000e+09]]))
assert_equal(np.full([1, 1, 2, 2], 3.14), np.array([[[[3.14, 3.14], [3.14, 3.14]]]]))

assert_equal(np.identity(1), np.array([[1.0]]))
assert_equal(np.identity(3), np.array([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]))

assert_equal(np.arange(10), np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9]))
assert_equal(np.arange(1, 10), np.array([1, 2, 3, 4, 5, 6, 7, 8, 9]))
assert_equal(np.arange(1, 10, 2), np.array([1, 3, 5, 7, 9]))

assert_equal(np.linspace(0, 1, 5), np.array([0.0, 0.25, 0.5, 0.75, 1.0]))
assert np.allclose(np.linspace(0, 1, 5, False), np.array([0.0, 0.2, 0.4, 0.6, 0.8]))
assert np.allclose(np.linspace(0, 1, 20, True),
                   np.array([0.0, 0.05263158, 0.10526316,
                             0.15789474, 0.21052632, 0.26315789,
                             0.31578947, 0.36842105, 0.42105263,
                             0.47368421, 0.52631579, 0.57894737,
                             0.63157895, 0.68421053, 0.73684211,
                             0.78947368, 0.84210526, 0.89473684,
                             0.94736842, 1.0]))


# test array properties
arr1 = np.array([1, 2, 3])
assert arr1.size == 3
assert arr1.ndim == 1
assert arr1.shape == (3,)
assert arr1.dtype == 'int64'

arr2 = np.array([[1, 2], [3, 4]])
assert arr2.size == 4
assert arr2.ndim == 2
assert arr2.shape == (2, 2)
assert arr2.dtype == np.int64

arr3 = np.array([[1, 2, 2, 1], [3, 4, 4, 3], [5, 6, 6, 5]], np.int32)
assert arr3.size == 12
assert arr3.ndim == 2
assert arr3.shape == (3, 4)
assert arr3.dtype == 'int32'

arr4 = np.array([[[[[1.5, 2.5], [3.5, 4.5], [5.5, 6.5]]]]])
assert arr4.size == 6
assert arr4.ndim == 5
assert arr4.shape == (1, 1, 1, 3, 2)
assert arr4.dtype == np.float64


# test boolean functions
arr1 = np.array([0.0])
assert arr1.all() == False
assert arr1.any() == False

arr2 = np.array([1.0])
assert arr2.all() == True
assert arr2.any() == True

arr3 = np.array([[1, 0], [0, 1]])
assert arr3.all() == False
assert arr3.any() == True

arr4 = np.array([[[True, False, True], [False, True, False]], [[True, False, True], [False, True, False]]])
assert arr4.all() == False
assert arr4.any() == True


# test array sum
a = np.array([1.0, 2.0, 3.0])
assert math.isclose(a.sum(), 6.0)

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert arr1.sum() == 45
assert arr1.sum(0) == 45

arr2 = np.array([[1], [2], [3]])
assert arr2.sum() == 6
assert_equal(arr2.sum(0), np.array([6]))
assert_equal(arr2.sum(1), np.array([1, 2, 3]))
assert arr2.sum((0, 1)) == 6

arr3 = np.array([[[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]])
assert arr3.sum() == 40.5
assert_equal(arr3.sum(0), np.array([[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]))
assert_equal(arr3.sum(1), np.array([[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]))
assert_equal(arr3.sum(2), np.array([[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]))
assert_equal(arr3.sum(3), np.array([[[[10.5, 13.5, 16.5]]]]))
assert_equal(arr3.sum(4), np.array([[[[7.5, 13.5, 19.5]]]]))

assert_equal(arr3.sum((0, 1)), np.array([[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]))
assert_equal(arr3.sum((3, 4)), np.array([[[40.5]]]))


# test array prod
arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert arr1.prod() == 362880
assert arr1.prod(0) == 362880

arr2 = np.array([[1], [2], [3]])
assert arr2.prod() == 6
assert_equal(arr2.prod(0), np.array([6]))
assert_equal(arr2.prod(1), np.array([1, 2, 3]))
assert arr2.prod((0, 1)) == 6

arr3 = np.array([[[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]])
assert arr3.prod() == 304845.556640625
assert_equal(arr3.prod(0), np.array([[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]))
assert_equal(arr3.prod(1), np.array([[[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]]]))
assert_equal(arr3.prod(4), np.array([[[[13.125, 86.625, 268.125]]]]))
assert_equal(arr3.prod((0, 3)), np.array([[[28.875, 73.125, 144.375]]]))


# test array min
a = np.array([1.0, 2.0, 3.0])
assert math.isclose(a.min(), 1.0)

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert arr1.min() == 1

arr2 = np.array([[-1], [-2], [-3]])
assert arr2.min() == -3

arr3 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
assert arr3.min() == -5.5
assert_equal(arr3.min(0), np.array([[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]))
assert_equal(arr3.min(4), np.array([[[[-1.5, -3.5, -5.5]]]]))
assert_equal(arr3.min((0, 3)), np.array([[[1.5, -5.5]]]))
assert_equal(arr3.min((0, 4)), np.array([[[-1.5, -3.5, -5.5]]]))


# test array max
a = np.array([1.0, 2.0, 3.0])
assert math.isclose(a.max(), 3.0)

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert arr1.max() == 9

arr2 = np.array([[-1], [-2], [-3]])
assert arr2.max() == -1

arr3 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
assert arr3.max() == 5.5
assert_equal(arr3.max(0), np.array([[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]))

assert_equal(arr3.max(4), np.array([[[[1.5, 3.5, 5.5]]]]))
assert_equal(arr3.max((0, 3)), np.array([[[5.5, -1.5]]]))
assert_equal(arr3.max((0, 4)), np.array([[[1.5, 3.5, 5.5]]]))

# test array mean
a = np.array([1.0, 2.0, 3.0])
assert math.isclose(a.mean(), 2.0)

arr = np.array([[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]])
assert arr.mean() == 4.5
assert_equal(arr.mean(0), np.array([[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]))
assert_equal(arr.mean(1), np.array([[3.5, 4.5, 5.5]]))
assert_equal(arr.mean(2), np.array([[2.5, 4.5, 6.5]]))


# test array std
a = np.array([1.0, 2.0, 3.0])
assert math.isclose(a.std(), 0.816496580927726)

arr = np.array([[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]])
assert arr.std() == 1.8257418583505538
assert_equal(arr.std(0), np.array([[0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [0.0, 0.0, 0.0]]))
assert np.allclose(arr.std(1), np.array([[1.632993, 1.632993, 1.632993]]))
assert np.allclose(arr.std(2), np.array([[0.81649658, 0.81649658, 0.81649658]]))


# test array var
a = np.array([1.0, 2.0, 3.0])
assert math.isclose(a.var(), 0.6666666666666666)

arr = np.array([[[1.5, 2.5, 3.5], [3.5, 4.5, 5.5], [5.5, 6.5, 7.5]]])
assert arr.var() == 3.3333333333333335
assert_equal(arr.var(0), np.array([[0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [0.0, 0.0, 0.0]]))
assert np.allclose(arr.var(1), np.array([[2.66666667, 2.66666667, 2.66666667]]))
assert np.allclose(arr.var(2), np.array([[0.66666667, 0.66666667, 0.66666667]]))


# test array argmin
a = np.array([3, 1, 2])
assert a.argmin() == 1

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert arr1.argmin() == 0

arr2 = np.array([[-1], [-2], [-3]])
assert arr2.argmin() == 2
assert_equal(arr2.argmin(0), np.array([2]))

arr3 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
assert arr3.argmin() == 5
assert_equal(arr3.argmin(0), np.array([[[[0, 0], [0, 0], [0, 0]]]]))
assert_equal(arr3.argmin(1), np.array([[[[0, 0], [0, 0], [0, 0]]]]))
assert_equal(arr3.argmin(2), np.array([[[[0, 0], [0, 0], [0, 0]]]]))
assert_equal(arr3.argmin(3), np.array([[[[0, 2]]]]))
assert_equal(arr3.argmin(4), np.array([[[[1, 1, 1]]]]))


# test array argmax
a = np.array([3, 1, 2])
assert a.argmax() == 0

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert arr1.argmax() == 8

arr2 = np.array([[-1], [-2], [-3]])
assert arr2.argmax() == 0
assert_equal(arr2.argmax(0), np.array([0]))

arr3 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
assert arr3.argmax() == 4
assert_equal(arr3.argmax(0), np.array([[[[0, 0], [0, 0], [0, 0]]]]))
assert_equal(arr3.argmax(1), np.array([[[[0, 0], [0, 0], [0, 0]]]]))
assert_equal(arr3.argmax(2), np.array([[[[0, 0], [0, 0], [0, 0]]]]))
assert_equal(arr3.argmax(3), np.array([[[[2, 0]]]]))
assert_equal(arr3.argmax(4), np.array([[[[0, 0, 0]]]]))


# test array argsort
a = np.array([3, 1, 2])
assert_equal(a.argsort(), np.array([1, 2, 0]))

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert_equal(arr1.argsort(), np.array([0, 1, 2, 3, 4, 5, 6, 7, 8]))

arr2 = np.array([9, 8, 7, 6, 5, 4, 3, 2, 1])
assert_equal(arr2.argsort(), np.array([8, 7, 6, 5, 4, 3, 2, 1, 0]))

arr3 = np.array([[-1], [-2], [-3]])
assert_equal(arr3.argsort(), np.array([[0], [0], [0]]))
assert_equal(arr3.argsort(0), np.array([[2], [1], [0]]))

arr4 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
assert_equal(arr4.argsort(), np.array([[[[[1, 0], [1, 0], [1, 0]]]]]))
assert_equal(arr4.argsort(0), np.array([[[[[0, 0], [0, 0], [0, 0]]]]]))
assert_equal(arr4.argsort(1), np.array([[[[[0, 0], [0, 0], [0, 0]]]]]))
assert_equal(arr4.argsort(2), np.array([[[[[0, 0], [0, 0], [0, 0]]]]]))
assert_equal(arr4.argsort(3), np.array([[[[[0, 2], [1, 1], [2, 0]]]]]))
assert_equal(arr4.argsort(4), np.array([[[[[1, 0], [1, 0], [1, 0]]]]]))


# test array sort
a = np.array([3, 1, 2])
a.sort()
assert_equal(a, np.array([1, 2, 3]))

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
arr1.sort()
assert_equal(arr1, np.array([1, 2, 3, 4, 5, 6, 7, 8, 9]))

arr2 = np.array([9, 8, 7, 6, 5, 4, 3, 2, 1])
arr2.sort()
assert_equal(arr2, np.array([1, 2, 3, 4, 5, 6, 7, 8, 9]))

arr3 = np.array([[-1], [-2], [-3]])
arr3.sort(0)
assert_equal(arr3, np.array([[-3], [-2], [-1]]))

arr4 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
arr4.sort(3)
assert_equal(arr4, np.array([[[[[1.5, -5.5], [3.5, -3.5], [5.5, -1.5]]]]]))


# test array reshape
a = np.array([[1, 2], [3, 4]])
assert_equal(a.reshape([1, 4]), np.array([[1, 2, 3, 4]]))

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert_equal(arr1.reshape([3, 3]), np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]]))

arr2 = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
assert_equal(arr2.reshape([9]), np.array([1, 2, 3, 4, 5, 6, 7, 8, 9]))

arr3 = np.array([1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5])
assert_equal(arr3.reshape([3, 4]), np.array([[1.5, 2.5, 3.5, 4.5], [5.5, 6.5, 7.5, 8.5], [9.5, 10.5, 11.5, 12.5]]))
assert_equal(arr3.reshape([2, 3, 2]), np.array([[[1.5, 2.5], [3.5, 4.5], [5.5, 6.5]],
                                                [[7.5, 8.5], [9.5, 10.5], [11.5, 12.5]]]))
assert_equal(arr3.reshape([1, 1, 2, 2, 3]), np.array([[[[[1.5, 2.5, 3.5], [4.5, 5.5, 6.5]],
                                                        [[7.5, 8.5, 9.5], [10.5, 11.5, 12.5]]]]]))


# test array resize
a = np.array([[1, 2], [3, 4]])
a.resize([1, 4])
assert_equal(a, np.array([[1, 2, 3, 4]]))


# test array squeeze
a = np.array([[1, 2, 3, 4]])
assert_equal(a.squeeze(), np.array([1, 2, 3, 4]))

arr1 = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9])
assert_equal(arr1.squeeze(), np.array([1, 2, 3, 4, 5, 6, 7, 8, 9]))

arr2 = np.array([[-1], [-2], [-3]])
assert_equal(arr2.squeeze(), np.array([-1, -2, -3]))

arr3 = np.array([[[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]])
assert_equal(arr3.squeeze(), np.array([[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]))
assert_equal(arr3.squeeze(0), np.array([[[[1.5, -1.5], [3.5, -3.5], [5.5, -5.5]]]]))


# test array transpose
a = np.array([[1, 2, 3, 4]])
assert_equal(a.transpose(), np.array([[1], [2], [3], [4]]))

arr1 = np.array([[1], [2], [3]])
assert_equal(arr1.transpose(), np.array([[1, 2, 3]]))

arr2 = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
assert_equal(arr2.transpose(), np.array([[1, 4, 7], [2, 5, 8], [3, 6, 9]]))
assert_equal(arr2.transpose(0, 1), np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]]))
assert_equal(arr2.transpose(1, 0), np.array([[1, 4, 7], [2, 5, 8], [3, 6, 9]]))

arr3 = np.array([[[[[1.5, -5.5], [3.5, -3.5], [5.5, -1.5]]]]])
assert_equal(arr3.transpose(), np.array([[[[[1.5]]], [[[3.5]]], [[[5.5]]]],
                                         [[[[-5.5]]], [[[-3.5]]], [[[-1.5]]]]]))
assert_equal(arr3.transpose(0, 1, 2, 3, 4), np.array([[[[[1.5, -5.5],
                                                         [3.5, -3.5],
                                                         [5.5, -1.5]]]]]))
assert_equal(arr3.transpose((3, 4, 0, 2, 1)), np.array([[[[[1.5]]], [[[-5.5]]]],
                                                        [[[[3.5]]], [[[-3.5]]]],
                                                        [[[[5.5]]], [[[-1.5]]]]]))


# test array repeat
a = np.array([[1, 2, 3, 4]])
assert_equal(a.repeat(2), np.array([[1, 1, 2, 2, 3, 3, 4, 4]]))

arr1 = np.array([[1, 2], [3, 4]])
assert_equal(arr1.repeat(2, 0), np.array([[1, 2], [1, 2], [3, 4], [3, 4]]))

arr2 = np.array([[[[1, 2, 3], [4, 5, 6]]]])
assert_equal(arr2.repeat(4, 3), np.array([[[[1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3],
                                            [4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6]]]]))
assert_equal(arr2.repeat([1, 2, 3], 3), np.array([[[[1, 2, 2, 3, 3, 3],
                                                    [4, 5, 5, 6, 6, 6]]]]))


# test array flatten
a = np.array([[1, 2, 3, 4]])
assert_equal(a.transpose().flatten(), np.array([1, 2, 3, 4]))

assert_equal(a.repeat(2), np.array([[1, 1, 2, 2, 3, 3, 4, 4]]))
arr1 = np.array([[1, 2], [3, 4]])
assert_equal(arr1.flatten(), np.array([1, 2, 3, 4]))

arr2 = np.array([[[1., 2.], [3., 4.], [5., 6.]], [[7., 8.], [9., 10.], [11., 12.]]])
assert_equal(arr2.flatten(), np.array([1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12.]))


# test array copy
a = np.array([1.1, 2.2, 3.3])
assert_equal(a.copy(), a)

arr1 = np.array([[1, 2], [3, 4]])
arr2 = arr1
arr3 = arr1.copy()

arr1[0] = [10, 20]
assert_equal(arr2, np.array([[10, 20], [3, 4]])) # Shallow copy
assert_equal(arr3, np.array([[1, 2], [3, 4]]))   # Deep copy


# test array astype
a = np.array([1.1, 2.2, 3.3])
assert_equal(a.astype(np.int32), np.array([1, 2, 3]))

arr1 = np.array([1, 2, 2.5])

assert_equal(arr1.astype('int_'), np.array([1, 2, 2]))
assert arr1.dtype == 'float64'

assert_equal(arr1.astype('float64'), np.array([1.0, 2.0, 2.5]))
assert arr1.dtype == 'float64'


# test array round
a = np.array([1.1, 2.2, 3.3])
assert_equal(a.round(), np.array([1., 2., 3.]))

assert_equal(np.round(np.array([40, 20, 30, 10])), np.array([40, 20, 30, 10]))
assert_equal(np.round(np.array([0.37, 1.64])), np.array([0.0, 2.0]))
assert_equal(np.round(np.array([.5, 1.5, 2.5, 3.5, 4.5])),np.array([1., 2., 3., 4., 5.]))


# test array floor
assert_equal(np.floor(np.array([40, 20, 30, 10])), np.array([40, 20, 30, 10]))
assert_equal(np.floor(np.array([0.37, 1.64])), np.array([0., 1.]))
assert_equal(np.floor(np.array([.5, 1.5, 2.5, 3.5, 4.5])), np.array([0., 1., 2., 3., 4.]))


# test array ceil
assert_equal(np.ceil(np.array([40, 20, 30, 10])), np.array([40, 20, 30, 10]))
assert_equal(np.ceil(np.array([0.37, 1.64])), np.array([1., 2.]))
assert_equal(np.ceil(np.array([.5, 1.5, 2.5, 3.5, 4.5])), np.array([1., 2., 3., 4., 5.]))


# test array abs
assert_equal(np.abs(np.array([[-1.2, 1.2], [-10, 10]])), np.array([[1.2, 1.2], [10, 10]]))
assert_equal(np.abs(np.linspace(0, -10)), np.linspace(0, 10))


# test __repr__ __str__
a = np.array([[1, 2], [3, 4]])
assert repr(a) == '''
array([[1, 2],
       [3, 4]])
'''.strip()


# test array add
a = np.ones([2, 3])
assert_equal(a + 1, np.array([[2., 2., 2.], [2., 2., 2.]]))

arr1 = np.array([1, 2, 3, 4, 5])

assert_equal(arr1 + 1, np.array([2, 3, 4, 5, 6]))
assert_equal(arr1 + 2.5, np.array([3.5, 4.5, 5.5, 6.5, 7.5]))
assert_equal(1 + arr1, np.array([2, 3, 4, 5, 6]))
assert_equal(2.5 + arr1, np.array([3.5, 4.5, 5.5, 6.5, 7.5]))
assert_equal(arr1 + arr1, np.array([2, 4, 6, 8, 10]))
assert_equal(arr1 + np.array([-1, -2, -3, -4, -5]), np.array([0, 0, 0, 0, 0]))

arr2 = np.array([[1.33, 2.66], [3.99, 5.33]])

assert_equal(arr2 + 1, np.array([[2.33, 3.66], [4.99, 6.33]]))
assert_equal(arr2 + 1.66, np.array([[2.99, 4.32], [5.65, 6.99]]))
assert_equal(1 + arr2, np.array([[2.33, 3.66], [4.99, 6.33]]))
assert_equal(1.66 + arr2, np.array([[2.99, 4.32], [5.65, 6.99]]))
assert_equal(arr2 + arr2, np.array([[2.66, 5.32], [7.98, 10.66]]))

array1 = np.ones([2, 2, 2, 2, 2])
array2 = np.array([[[[[1.1, 1.2], [1.3, 1.4]], [[1.5, 1.6], [1.7, 1.8]]],
                    [[[2.1, 2.2], [2.3, 2.4]], [[2.5, 2.6], [2.7, 2.8]]]],
                   [[[[3.1, 3.2], [3.3, 3.4]], [[3.5, 3.6], [3.7, 3.8]]],
                    [[[4.1, 4.2], [4.3, 4.4]], [[4.5, 4.6], [4.7, 4.8]]]]])

assert_equal(array1 + array2, np.array([[[[[2.1, 2.2], [2.3, 2.4]],
                                          [[2.5, 2.6], [2.7, 2.8]]],
                                         [[[3.1, 3.2], [3.3, 3.4]],
                                          [[3.5, 3.6], [3.7, 3.8]]]],
                                        [[[[4.1, 4.2], [4.3, 4.4]],
                                          [[4.5, 4.6], [4.7, 4.8]]],
                                         [[[5.1, 5.2], [5.3, 5.4]],
                                          [[5.5, 5.6], [5.7, 5.8]]]]]))


# test array sub
a = np.ones([2, 3])
assert_equal(a - 1, np.array([[0., 0., 0.], [0., 0., 0.]]))

arr1 = np.array([1, 2, 3, 4, 5])

assert_equal(arr1 - 1, np.array([0, 1, 2, 3, 4]))
assert_equal(arr1 - 2.5, np.array([-1.5, -0.5, 0.5, 1.5, 2.5]))
assert_equal(1 - arr1, np.array([0, -1, -2, -3, -4]))
assert_equal(2.5 - arr1, np.array([1.5, 0.5, -0.5, -1.5, -2.5]))
assert_equal(arr1 - arr1, np.array([0, 0, 0, 0, 0]))

arr2 = np.array([[1.33, 2.66], [3.99, 5.33]])

assert np.allclose(arr2 - 1, np.array([[0.33, 1.66], [2.99, 4.33]]))
assert np.allclose(arr2 - 1.66, np.array([[-0.33, 1.], [2.33, 3.67]]))
assert np.allclose(1 - arr2, np.array([[-0.33, -1.66], [-2.99, -4.33]]))
assert np.allclose(1.66 - arr2, np.array([[0.33, -1.], [-2.33, -3.67]]))
assert_equal(arr2 - arr2, np.array([[0, 0], [0, 0]]))

array1 = np.ones([2, 2, 2, 2, 2])
array2 = np.array([[[[[1.1, 1.2], [1.3, 1.4]], [[1.5, 1.6], [1.7, 1.8]]],
                    [[[2.1, 2.2], [2.3, 2.4]], [[2.5, 2.6], [2.7, 2.8]]]],
                   [[[[3.1, 3.2], [3.3, 3.4]], [[3.5, 3.6], [3.7, 3.8]]],
                    [[[4.1, 4.2], [4.3, 4.4]], [[4.5, 4.6], [4.7, 4.8]]]]])

assert np.allclose(array1 - array2, np.array([[[[[-0.1, -0.2], [-0.3, -0.4]],
                                                [[-0.5, -0.6], [-0.7, -0.8]]],
                                               [[[-1.1, -1.2], [-1.3, -1.4]],
                                                [[-1.5, -1.6], [-1.7, -1.8]]]],
                                              [[[[-2.1, -2.2], [-2.3, -2.4]],
                                                [[-2.5, -2.6], [-2.7, -2.8]]],
                                               [[[-3.1, -3.2], [-3.3, -3.4]],
                                                [[-3.5, -3.6], [-3.7, -3.8]]]]]))


# test array mul
a = np.ones([2, 3])
assert_equal(a * 2, np.array([[2., 2., 2.], [2., 2., 2.]]))

arr1 = np.array([1, 2, 3, 4, 5])

assert_equal(arr1 * 2, np.array([2, 4, 6, 8, 10]))
assert_equal(arr1 * 2.5, np.array([2.5, 5.0, 7.5, 10.0, 12.5]))
assert_equal(4 * arr1, np.array([4, 8, 12, 16, 20]))
assert_equal(5.0 * arr1, np.array([5.0, 10.0, 15.0, 20.0, 25.0]))
assert_equal(arr1 * arr1, np.array([1, 4, 9, 16, 25]))

arr2 = np.array([[1.33, 2.66], [3.99, 5.33]])

assert np.allclose(arr2 * 2, np.array([[2.66, 5.32], [7.98, 10.66]]))
assert np.allclose(arr2 * 2.5, np.array([[3.325, 6.65], [9.975, 13.325]]))
assert np.allclose(4 * arr2, np.array([[5.32, 10.64], [15.96, 21.32]]))
assert np.allclose(5.0 * arr2, np.array([[6.65, 13.3], [19.95, 26.65]]))
assert np.allclose(arr2 * arr2, np.array([[1.7689, 7.0756], [15.9201, 28.4089]]))


# test array div
a = np.ones([2, 3])
assert_equal(a / 2, np.array([[0.5, 0.5, 0.5], [0.5, 0.5, 0.5]]))
arr1 = np.array([1, 2, 3, 4, 5])

assert np.allclose(arr1 / 2, np.array([0.5, 1.0, 1.5, 2.0, 2.5]))
assert np.allclose(arr1 / 2.5, np.array([0.4, 0.8, 1.2, 1.6, 2.0]))
assert np.allclose(4 / arr1, np.array([4.0, 2.0, 1.3333333333333333, 1.0, 0.8]))
assert np.allclose(5.0 / arr1, np.array([5.0, 2.5, 1.6666666666666667, 1.25, 1.0]))

arr2 = np.array([[1.33, 2.66], [3.99, 5.33]])

assert np.allclose(arr2 / 2, np.array([[0.665, 1.33], [1.995, 2.665]]))
assert np.allclose(arr2 / 1.33, np.array([[1.0, 2.0], [3.0, 4.007519]]))
assert np.allclose(4 / arr2, np.array([[3.0075188, 1.5037594], [1.00250627, 0.75046904]]))
assert np.allclose(1.33 / arr2, np.array([[1.0, 0.5], [0.33333333, 0.24953096]]))
assert np.allclose(arr2 / arr2, np.array([[1.0, 1.0], [1.0, 1.0]]))


# test array matmul
a = np.ones([2, 3])
assert_equal(a @ np.ones([3, 4]), np.ones([2, 4]) * 3)

arr1 = np.array([[1, 2], [3, 4]])
arr2 = np.array([[5, 6], [7, 8]])

assert_equal(arr1 @ arr2, np.array([[19, 22], [43, 50]]))
assert_equal(arr2 @ arr1, np.array([[23, 34], [31, 46]]))

arr3 = np.array([[1.0, 2.0], [2.0, 3.0], [4.0, 5.0]])
arr4 = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])

assert_equal(arr3 @ arr4, np.array([[9.0, 12.0, 15.0],
                                    [14.0, 19.0, 24.0],
                                    [24.0, 33.0, 42.0]]))
assert_equal(arr4 @ arr3, np.array([[17.0, 23.0],
                                    [38.0, 53.0]]))


# test array len
arr1 = np.array([1, 2, 3, 4, 5])
assert len(arr1) == 5

arr2 = np.array([[1, 2], [3, 4], [5, 6]])
assert len(arr2) == 3

arr3 = np.array([[1, 2], [3, 4]])
assert len(arr3) == 2

arr4 = np.array([[[[1.5, -1.5, 2.5], [3.5, -3.5, 4.5], [5.5, -5.5, 6.5]]]])
assert len(arr4) == 1


# test array pow
assert_equal(np.ones([2, 3]) ** 2, np.array([[1., 1., 1.], [1., 1., 1.]]))

a = np.array([[[1, 2], [3, 4]], [[5, 6], [7, 8]]])
b = np.array([[[2, 2], [2, 2]], [[3, 3], [3, 3]]])

assert np.allclose(a ** b, np.array([[[1, 4], [9, 16]], [[125, 216], [343, 512]]]))

arr1 = np.array([1, 2, 3, 4, 5])
assert_equal(arr1 ** 2, np.array([1, 4, 9, 16, 25]))
assert np.allclose(arr1 ** 2.5, np.array([1.0, 5.656854, 15.588457, 32.0, 55.901699]))
assert_equal(2 ** arr1, np.array([2, 4, 8, 16, 32]))
assert np.allclose(2.5 ** arr1, np.array([2.5, 6.25, 15.625, 39.0625, 97.65625]))

arr2 = np.array([[1.33, 2.66], [3.99, 5.33]])
assert np.allclose(arr2 ** 2, np.array([[1.7689, 7.0756], [15.9201, 28.4089]]))
assert np.allclose(arr2 ** 2.5, np.array([[2.039995, 11.53995437], [31.80037484, 65.58703869]]))
assert np.allclose(4 ** arr2, np.array([[6.32033049, 39.94657756],
                                        [252.47557235, 1618.0046067]]))
assert np.allclose(5.0 ** arr2, np.array([[8.50413422, 72.32029875],
                                          [615.0215271, 5315.08388464]]))


# test array binary
a = np.array([[1, 0], [0, 1]], dtype=np.bool_)
'''
array([[ True, False],
       [False,  True]])
'''

assert_equal(a & True, np.array([[1, 0], [0, 1]], dtype=np.bool_))
assert_equal(a | True, np.array([[1, 1], [1, 1]], dtype=np.bool_))
assert_equal(a ^ True, np.array([[0, 1], [1, 0]], dtype=np.bool_))
assert_equal(~a, np.array([[False,  True], [ True, False]]))


# test array trigonometry
arr1 = np.array([np.pi / 6, np.pi / 4, np.pi / 3, np.pi / 2, np.pi])
assert np.allclose(np.sin(arr1), np.array([0.5, 0.707107, 0.866025, 1.0, 0.0]))
assert np.allclose(np.cos(arr1), np.array([0.866025, 0.707107, 0.5, 0.0, -1.0]))
assert np.allclose(np.tan(arr1), np.array([0.57735, 1.0, 1.73205, np.inf, 0.0]))

arr2 = np.array([0.5, 0.707107, 0.866025, 1.0, 0.0])
assert np.allclose(np.arcsin(arr2), np.array([np.pi / 6, np.pi / 4, np.pi / 3, np.pi / 2, 0.0]))
assert np.allclose(np.arccos(arr2), np.array([np.pi / 3, np.pi / 4, np.pi / 6, 0.0, np.pi / 2]))
assert np.allclose(np.arctan(arr2), np.array([0.463648, 0.61548, 0.713724, 0.785398, 0.0]))


# test array exponential
arr1 = np.array([0.0, 1.0, 2.0, 3.0, 4.0])

assert np.allclose(np.exp(arr1), np.array([1.0, 2.718282, 7.389056, 20.085537, 54.598150]))
assert np.allclose(np.log(arr1), np.array([-np.inf, 0.0, 0.693147, 1.098612, 1.386294]))
assert np.allclose(np.log2(arr1), np.array([-np.inf, 0.0, 1.0, 1.584963, 2.0]))
assert np.allclose(np.log10(arr1), np.array([-np.inf, 0.0, 0.30103, 0.477121, 0.60206]))


# test array getitem
arr1 = np.arange(30).reshape([3, 2, 5])

assert_equal(arr1[0], np.array([[0, 1, 2, 3, 4],
                                [5, 6, 7, 8, 9]]))
assert_equal(arr1[1], np.array([[10, 11, 12, 13, 14],
                                [15, 16, 17, 18, 19]]))
assert_equal(arr1[2], np.array([[20, 21, 22, 23, 24],
                                [25, 26, 27, 28, 29]]))
assert_equal(arr1[-1], np.array([[20, 21, 22, 23, 24],
                                 [25, 26, 27, 28, 29]]))
assert_equal(arr1[-2], np.array([[10, 11, 12, 13, 14],
                                 [15, 16, 17, 18, 19]]))

assert_equal(arr1[0, 0], np.array([0, 1, 2, 3, 4]))
assert_equal(arr1[1, 1], np.array([15, 16, 17, 18, 19]))
assert_equal(arr1[2, 0], np.array([20, 21, 22, 23, 24]))

assert_equal(arr1[(0,)], np.array([[0, 1, 2, 3, 4],
                                   [5, 6, 7, 8, 9]]))
assert_equal(arr1[(0, 1)], np.array([5, 6, 7, 8, 9]))
assert_equal(arr1[(1, 0)], np.array([10, 11, 12, 13, 14]))
assert_equal(arr1[(-1,)], np.array([[20, 21, 22, 23, 24],
                                    [25, 26, 27, 28, 29]]))
assert_equal(arr1[(-3, -1)], np.array([5, 6, 7, 8, 9]))
assert arr1[(0, 1, 2)] == 7
assert arr1[(2, 1, 0)] == 25
assert arr1[(-3, -2, -1)] == 4
assert arr1[(-1, -2, -3)] == 22

assert_equal(arr1[[0, ]], np.array([[[0, 1, 2, 3, 4],
                                     [5, 6, 7, 8, 9]]]))
assert_equal(arr1[[0, 1]], np.array([[[0, 1, 2, 3, 4],
                                      [5, 6, 7, 8, 9]],
                                     [[10, 11, 12, 13, 14],
                                      [15, 16, 17, 18, 19]]]))
assert_equal(arr1[[1, 2]], np.array([[[10, 11, 12, 13, 14],
                                      [15, 16, 17, 18, 19]],
                                     [[20, 21, 22, 23, 24],
                                      [25, 26, 27, 28, 29]]]))
assert_equal(arr1[[2, 1]], np.array([[[20, 21, 22, 23, 24],
                                      [25, 26, 27, 28, 29]],
                                     [[10, 11, 12, 13, 14],
                                      [15, 16, 17, 18, 19]]]))
assert_equal(arr1[[2, 2]], np.array([[[20, 21, 22, 23, 24],
                                      [25, 26, 27, 28, 29]],
                                     [[20, 21, 22, 23, 24],
                                      [25, 26, 27, 28, 29]]]))
assert_equal(arr1[[0, 1, 2]], np.array([[[0, 1, 2, 3, 4],
                                         [5, 6, 7, 8, 9]],
                                        [[10, 11, 12, 13, 14],
                                         [15, 16, 17, 18, 19]],
                                        [[20, 21, 22, 23, 24],
                                         [25, 26, 27, 28, 29]]]))
assert_equal(arr1[[2, 1, 0]], np.array([[[20, 21, 22, 23, 24],
                                         [25, 26, 27, 28, 29]],
                                        [[10, 11, 12, 13, 14],
                                         [15, 16, 17, 18, 19]],
                                        [[0, 1, 2, 3, 4],
                                         [5, 6, 7, 8, 9]]]))

assert_equal(arr1[0:1], np.array([[[0, 1, 2, 3, 4],
                                   [5, 6, 7, 8, 9]]]))
assert_equal(arr1[2:3], np.array([[[20, 21, 22, 23, 24],
                                   [25, 26, 27, 28, 29]]]))
assert_equal(arr1[0:2], np.array([[[0, 1, 2, 3, 4],
                                   [5, 6, 7, 8, 9]],
                                  [[10, 11, 12, 13, 14],
                                   [15, 16, 17, 18, 19]]]))
assert_equal(arr1[1:3], np.array([[[10, 11, 12, 13, 14],
                                   [15, 16, 17, 18, 19]],
                                  [[20, 21, 22, 23, 24],
                                   [25, 26, 27, 28, 29]]]))
assert_equal(arr1[1:3:3], np.array([[[10, 11, 12, 13, 14],
                                     [15, 16, 17, 18, 19]]]))
assert_equal(arr1[0:3:2], np.array([[[0, 1, 2, 3, 4],
                                     [5, 6, 7, 8, 9]],
                                    [[20, 21, 22, 23, 24],
                                     [25, 26, 27, 28, 29]]]))

assert_equal(arr1[1:], np.array([[[10, 11, 12, 13, 14],
                                   [15, 16, 17, 18, 19]],
                                  [[20, 21, 22, 23, 24],
                                   [25, 26, 27, 28, 29]]]))
assert_equal(arr1[:2], np.array([[[0, 1, 2, 3, 4],
                                   [5, 6, 7, 8, 9]],
                                  [[10, 11, 12, 13, 14],
                                   [15, 16, 17, 18, 19]]]))

assert_equal(arr1[::2], np.array([[[0, 1, 2, 3, 4],
                                   [5, 6, 7, 8, 9]],
                                  [[20, 21, 22, 23, 24],
                                   [25, 26, 27, 28, 29]]]))
assert_equal(arr1[-2:3:1], np.array([[[10, 11, 12, 13, 14],
                                      [15, 16, 17, 18, 19]],
                                     [[20, 21, 22, 23, 24],
                                      [25, 26, 27, 28, 29]]]))
assert_equal(arr1[3::-2], np.array([[[20, 21, 22, 23, 24],
                                     [25, 26, 27, 28, 29]],
                                    [[0, 1, 2, 3, 4],
                                     [5, 6, 7, 8, 9]]]))
assert_equal(arr1[::-1], np.array([[[20, 21, 22, 23, 24],
                                    [25, 26, 27, 28, 29]],
                                   [[10, 11, 12, 13, 14],
                                    [15, 16, 17, 18, 19]],
                                   [[0, 1, 2, 3, 4],
                                    [5, 6, 7, 8, 9]]]))
assert_equal(arr1[::], np.array([[[0, 1, 2, 3, 4],
                                  [5, 6, 7, 8, 9]],
                                 [[10, 11, 12, 13, 14],
                                  [15, 16, 17, 18, 19]],
                                 [[20, 21, 22, 23, 24],
                                  [25, 26, 27, 28, 29]]]))


# test array setitem
arr1 = np.arange(30).reshape([3, 2, 5])

arr1[0] = 10
assert_equal(arr1[0], np.array([[10, 10, 10, 10, 10],
                                [10, 10, 10, 10, 10]]))

arr1[1] = [1, 2, 3, 4, 5]
assert_equal(arr1[1], np.array([[1, 2, 3, 4, 5],
                                [1, 2, 3, 4, 5]]))

arr1[2] = [[1, 2, 3, 4, 5], [6, 7, 8, 9, 10]]
assert_equal(arr1[2], np.array([[1, 2, 3, 4, 5],
                                [6, 7, 8, 9, 10]]))

arr1[-1] = 0
assert_equal(arr1[-1], np.array([[0, 0, 0, 0, 0],
                                 [0, 0, 0, 0, 0]]))

arr1[0, 0] = 5
assert_equal(arr1[0, 0], np.array([5, 5, 5, 5, 5]))

arr1[1, 1] = [1, 2, 3, 4, 5]
assert_equal(arr1[1, 1], np.array([1, 2, 3, 4, 5]))

arr1[2, 0] = [2.5]
assert_equal(arr1[2, 0], np.array([2, 2, 2, 2, 2]))

arr1[(0,)] = 10.5
assert_equal(arr1[(0,)], np.array([[10, 10, 10, 10, 10],
                                   [10, 10, 10, 10, 10]]))

arr1[(0, 1)] = 0
assert_equal(arr1[(0, 1)], np.array([0, 0, 0, 0, 0]))

arr1[(-3, -1)] = 1
assert_equal(arr1[(-3, -1)], np.array([1, 1, 1, 1, 1]))

arr1[(-1, -2, -3)] = 3.14159
assert arr1[(-1, -2, -3)] == 3

arr1[[0, ]] = 0
assert_equal(arr1[[0, ]], np.array([[[0, 0, 0, 0, 0],
                                     [0, 0, 0, 0, 0]]]))

arr1[[0, 1]] = 1
assert_equal(arr1[[0, 1]], np.array([[[1, 1, 1, 1, 1],
                                      [1, 1, 1, 1, 1]],
                                     [[1, 1, 1, 1, 1],
                                      [1, 1, 1, 1, 1]]]))

arr1[[2, 2]] = [[1, 2, 3, 4, 5], [6, 7, 8, 9, 10]]
assert_equal(arr1[[2, 2]], np.array([[[1, 2, 3, 4, 5],
                                      [6, 7, 8, 9, 10]],
                                     [[1, 2, 3, 4, 5],
                                      [6, 7, 8, 9, 10]]]))

arr1[0:1] = 0
assert_equal(arr1[0:1], np.array([[[0, 0, 0, 0, 0],
                                   [0, 0, 0, 0, 0]]]))

arr1[2:3] = 1
assert_equal(arr1[2:3], np.array([[[1, 1, 1, 1, 1],
                                   [1, 1, 1, 1, 1]]]))

arr1[::2] = [[1, 2, 3, 4, 5], [6, 7, 8, 9, 10]]
assert_equal(arr1[::2], np.array([[[1, 2, 3, 4, 5],
                                   [6, 7, 8, 9, 10]],
                                  [[1, 2, 3, 4, 5],
                                   [6, 7, 8, 9, 10]]]))

arr1[-2:3:1] = [[[1, 2, 3, 4, 5],
                 [6, 7, 8, 9, 10]],
                [[11, 12, 13, 14, 15],
                 [16, 17, 18, 19, 20]]]
assert_equal(arr1[-2:3:1], np.array([[[1, 2, 3, 4, 5],
                                      [6, 7, 8, 9, 10]],
                                     [[11, 12, 13, 14, 15],
                                      [16, 17, 18, 19, 20]]]))

print("ALL TESTS PASSED")