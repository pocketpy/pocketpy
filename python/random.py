_inst = Random()

seed = _inst.seed
random = _inst.random
uniform = _inst.uniform
randint = _inst.randint

def shuffle(L):
    for i in range(len(L)):
        j = randint(i, len(L) - 1)
        L[i], L[j] = L[j], L[i]

def choice(L):
    return L[randint(0, len(L) - 1)]