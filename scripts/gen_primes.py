import numba
from typing import List

@numba.jit(nopython=True)
def sieve_of_eratosthenes(n: int) -> List[int]:
    assert n >= 2
    is_prime = [True] * (n + 1)
    is_prime[0] = is_prime[1] = False  # 0 和 1 不是素数
    
    for start in range(2, int(n**0.5) + 1):
        if is_prime[start]:
            for multiple in range(start*start, n + 1, start):
                is_prime[multiple] = False
    
    primes = [num for num, prime in enumerate(is_prime) if prime]
    return primes

all_primes = sieve_of_eratosthenes(2**30)
print(len(all_primes), all_primes[:10], all_primes[-10:])

index = 3
caps = [all_primes[index]]

while True:
    for i in range(index+1, len(all_primes)):
        last_cap = caps[-1]
        if last_cap < 1000:
            min_cap = last_cap * 2
        else:
            min_cap = last_cap * 1.5
        if all_primes[i] >= min_cap:
            caps.append(all_primes[i])
            index = i
            break
    else:
        break

print('-'*20)
print(caps)

print('switch(cap) {')
for i in range(len(caps)-1):
    print(f'    case {caps[i]}:', f'return {caps[i+1]};')
print('    default: c11__unreachable();')
print('}')