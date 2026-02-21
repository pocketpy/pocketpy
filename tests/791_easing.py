import easing

def validate(k, f, x):
    val = f(x)
    assert (-2 <= val <= 2.0), (k, x, val)
    assert isinstance(val, float), (k, x, val)

for k, f in easing.__dict__.items():
    if callable(f):
        validate(k, f, 0.2)
        validate(k, f, 0.5)
        validate(k, f, 0.8)
        validate(k, f, 1.0)
