from line_profiler import LineProfiler

def my_func():
    a = 0
    for i in range(1000000):
        a += i
    return a

lp = LineProfiler()

lp.add_function(my_func)

lp.runcall(my_func)

lp.print_stats()
