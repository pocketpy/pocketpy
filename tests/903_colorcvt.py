import colorcvt
from vmath import vec3

def oklch(expr: str) -> vec3:
    # oklch(82.33% 0.37 153)
    expr = expr[6:-1]
    l, c, h = expr.split()
    l = float(l[:-1]) / 100
    return vec3(l, float(c), float(h))

def srgb32(expr: str) -> vec3:
    # rgb(0, 239, 115)
    expr = expr[4:-1]
    r, g, b = expr.split(", ")
    r, g, b = int(r), int(g), int(b)
    c = vec3(r, g, b) / 255
    return colorcvt.srgb_to_linear_srgb(c)

def assert_equal(title: str, a: vec3, b: vec3) -> None:
    epsilon = 1e-3
    try:
        assert abs(a.x - b.x) < epsilon
        assert abs(a.y - b.y) < epsilon
        assert abs(a.z - b.z) < epsilon
    except AssertionError:
        raise AssertionError(f"{title}\nExpected: {b}, got: {a}")

def test(oklch_expr: str, srgb32_expr: str) -> None:
    oklch_color = oklch(oklch_expr)
    srgb32_color = srgb32(srgb32_expr)
    assert_equal('oklch_to_linear_srgb', colorcvt.oklch_to_linear_srgb(oklch_color), srgb32_color)
    # in range check
    oklch_color = colorcvt.linear_srgb_to_oklch(srgb32_color)
    assert_equal('oklch_to_linear_srgb+', colorcvt.oklch_to_linear_srgb(oklch_color), srgb32_color)
    assert_equal('linear_srgb_to_oklch+', colorcvt.linear_srgb_to_oklch(srgb32_color), oklch_color)

test("oklch(71.32% 0.1381 153)", "rgb(83, 187, 120)")
test("oklch(45.15% 0.037 153)", "rgb(70, 92, 76)")
test("oklch(22.5% 0.0518 153)", "rgb(4, 34, 16)")

test("oklch(100% 0.37 153)", "rgb(255, 255, 255)")
test("oklch(0% 0.0395 283.24)", "rgb(0, 0, 0)")

# hard samples
# test("oklch(95% 0.2911 264.18)", "rgb(224, 239, 255)")
# test("oklch(28.09% 0.2245 153)", "rgb(0, 54, 12)")
# test("oklch(82.33% 0.37 153)", "rgb(0, 239, 115)")
