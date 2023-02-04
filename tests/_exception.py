try:
    raise KeyError
except:
    print("exception caught")
print(123)

def f():
    try:
        raise KeyError('foo')
    except A:   # will fail to catch
        print("xx")
    except:
        print("exception caught")
    print(123)

f()