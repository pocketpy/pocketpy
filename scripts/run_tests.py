import os

singletypepath = 'tests/singletype'
mixedtypepath = 'tests/mixedtype'

def test_file(filepath):
    return os.system("./pocketpy " + filepath) == 0
    #return os.system("python3 " + filepath) == 0

def test_dir(path):
    print("=" * 50)
    for filename in os.listdir(path):
        if filename.endswith('.py'):
            filepath = os.path.join(path, filename)
            code = test_file(filepath)
            if not code:
                print("[x] " + filepath)
            else:
                print("[√] " + filepath)

if __name__ == '__main__':
    test_dir(singletypepath)
    test_dir(mixedtypepath)