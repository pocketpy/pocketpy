# match = 2
# assert match == 2
# case = 3
# assert case == 3

# def f(match):
#     match match:
#         case 1: return 1
#         case 2: return 2
#         case _:
#             return 999
#     return 0

# assert f(1) == 1
# assert f(2) == 2
# assert f(3) == 999
# assert f(4) == 999

# def f():
#     a = []
#     try:
#         match case:
#             case a[1]: return 1
#     except IndexError:
#         return 'IndexError'
#     return 0

# assert f() == 'IndexError'


# def f(pos):
#     match pos:
#         case 'str': return 'str'
#         case 0: return 0
#         case (1, 2): return '1, 2'
#         case (3, 4): return '3, 4'
#         case _: return 'other'

# assert f('str') == 'str'
# assert f(0) == 0
# assert f((1, 2)) == '1, 2'
# assert f((3, 4)) == '3, 4'
# assert f((1, 3)) == 'other'
# assert f((1, 2, 3)) == 'other'
