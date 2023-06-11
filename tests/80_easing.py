import easing

def validate(val):
    assert -2 <= val <= 2.0
    assert isinstance(val, float)

for k, f in easing.__dict__.items():
    if k.startswith("Ease"):
        validate(f(0.2))
        validate(f(0.5))
        validate(f(0.8))
        validate(f(1.0))
