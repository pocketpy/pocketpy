class Test:
    def __enter__(self):
        print("enter")
        return self
    
    def __exit__(self, exc_type, exc, tb):
        print("__exit__ called", exc_type)
        return False

with Test():
    print("inside")
    raise Exception("boom!")
