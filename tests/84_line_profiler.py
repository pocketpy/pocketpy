from line_profiler import LineProfiler

def f2(x):
    a = 0
    for i in range(x):
        if i % 5 == 0:
            a += i
    return a

def f1(x):
    res = f2(x)
    return res

lp = LineProfiler()

lp.add_function(f2)

# lp.runcall(f2, 1000000)
# lp.print_stats()
###############################

lp.add_function(f1)
lp.runcall(f1, 1000000)
lp.print_stats()

