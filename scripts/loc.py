import os

def get_loc(path):
    loc = 0
    with open(path, "rt", encoding='utf-8') as f:
        loc += len(f.readlines())
    return loc

def get_loc_for_dir(path):
    loc = 0
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith('.h'):
                _i = get_loc(os.path.join(root, file))
                print(f"{file}: {_i}")
                loc += _i
    return loc

print(get_loc_for_dir('src'))