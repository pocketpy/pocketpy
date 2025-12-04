from pkpy import configmacros

if configmacros['PK_ENABLE_THREADS'] == 1:
    from pkpy import ComputeThread
else:
    print('threads is not enabled, skipping test...')
    exit()

import time

thread_1 = ComputeThread(1)
thread_2 = ComputeThread(2)

for t in [thread_1, thread_2]:
    t.exec('''
def func(a):
    from pkpy import currentvm
    print("Hello from thread", currentvm(), "a =", a)
    for i in range(500000):
        if i % 100000 == 0:
            print(i, "from thread", currentvm())
    return a
                   
x = 123
''')
assert thread_1.eval('x') == 123

# thread_1.wait_for_done()
# thread_2.wait_for_done()

thread_1.submit_call('func', [1, 2, 3])
thread_2.submit_call('func', [4, 5, 6])

while not thread_1.is_done or not thread_2.is_done:
    print("Waiting for threads to finish...")
    time.sleep(1)

print("Thread 1 last return value:", thread_1.last_retval())
print("Thread 2 last return value:", thread_2.last_retval())

