import pickle

data1 = [1, 2, 3] * 100
data2 = [1.0, 2.0, 3.0] * 100
data3 = ['abcdefg', 'hijklmn', '_______________1'] * 100
data4 = [True, False, True] * 100
data5 = [None, None] * 100

original = {
    '1': data1,
    '2': data2,
    '3': data3,
    '45': {
        '4': data4,
        '5': data5,
    }
}

for i in range(10000):
    encoded = pickle.dumps(original)
    decoded = pickle.loads(encoded)
    if i == 0:
        assert original == decoded

