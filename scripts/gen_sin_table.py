import math

def generate_sin_table(num_points = 4096 + 1):
    y_step = 1.0 / (num_points - 1)
    sin_table = []
    for i in range(num_points):
        y = i * y_step
        x = math.asin(y)
        x = max(0, min(x, math.pi / 2))
        sin_table.append(x)
    return sin_table

if __name__ == "__main__":
    sin_table = generate_sin_table()
    print(len(sin_table))
    # print(sin_table)

    filepath = 'include/pocketpy/common/_sin_table.h'
    with open(filepath, 'w') as f:
        f.write('static const double _sin_table[] = {')
        for i, val in enumerate(sin_table):
            if i % 8 == 0:
                f.write('\n    ')
            f.write(str(val) + ', ')
        f.write('\n};\n')