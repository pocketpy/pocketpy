import os

def get_loc(path):
    loc = 0
    with open(path, "rt", encoding='utf-8') as f:
        loc += len(f.readlines())
    path = path.replace('\\', '/')
    print(f'{path}: {loc}')
    return loc

def get_loc_for_dir(path):
    loc = 0
    loc_ex = 0
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith('.h') or file.endswith('.c') or file.endswith('.h'):
                _i = get_loc(os.path.join(root, file))
                if file.startswith('_'):
                    loc_ex += _i
                else:
                    loc += _i
    return f'{path}: {loc} (+{loc_ex})'


print(get_loc_for_dir('include/pocketpy'))
print()
print(get_loc_for_dir('src'))
