---
icon: package
label: easing
---

# easing

Convert normalized animation progress into an interpolation weight.
Each function takes `t` and returns a float.

```python
import easing

start = 10.0
end = 30.0
progress = 0.5
weight = easing.InOutQuad(progress)
position = start + (end - start) * weight
assert position == 20.0
```

Use `Linear(t)` for constant speed. Each family below provides
`In<Name>(t)`, `Out<Name>(t)`, and `InOut<Name>(t)`:

| Family | Curve |
| --- | --- |
| `Sine` | Sinusoidal easing. |
| `Quad`, `Cubic`, `Quart`, `Quint` | Polynomial easing of increasing degree. |
| `Expo` | Exponential change. |
| `Circ` | Circular curve. |
| `Back` | Overshoot before settling. |
| `Elastic` | Spring-like oscillation. |
| `Bounce` | Bouncing transition. |

For example, `OutCubic(t)` slows toward the endpoint. Clamp elapsed-time
progress to `[0, 1]` in your application. Back and Elastic curves can
deliberately overshoot that output range, so the result is not always suitable
as an opacity value without further clamping.

API declarations: [easing.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/easing.pyi).
