from vmath import color32, rgb, rgba

a = color32(100, 200, 255, 120)
assert a.r == 100
assert a.g == 200
assert a.b == 255
assert a.a == 120
assert a.with_r(255).r == 255 
assert a.with_g(255).g == 255
assert a.with_b(255).b == 255
assert a.with_a(255).a == 255 and a.with_a(255).g == a.g

assert a.to_hex() == '#64c8ff78'
assert color32.from_hex('#64c8ff78') == a

assert rgb(100, 200, 255) != a
assert rgba(100, 200, 255, 120 / 255) == a

b = color32(75, 150, 200, 200)
assert a == a and b == b
assert a != b

assert repr(b) == 'color32(75, 150, 200, 200)'

# assert color32.alpha_blend(a, b) == color32(86, 173, 225, 162)
