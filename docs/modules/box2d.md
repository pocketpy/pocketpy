---
icon: package-dependencies
label: box2d
---

!!!
This module was moved to `carrotlib`, i.e. not a builtin module anymore.
!!!

[Box2D](https://box2d.org/) by Erin Catto, the world's best 2D physics engine.
All platforms are supported, including desktop, mobile and web.

## Overview

The `box2d` module in pkpy provides a high-level, also simplified, interface to Box2D engine, which is suitable for most use cases.
There are two classes in `box2d` module: `World` and `Body`.

`World` is the world of Box2D, it is the container of all `Body`s.
In most cases, you only need one `World` instance.
`World` class provides methods to create, destroy and query `Body`s
and also methods to step the simulation.

A `Body` instance is a physical entity in the world.
A `Body` can only have one shape at a time.
For example, a circle, a rectangle, a polygon, etc.
You are allowed to change the shape of a `Body` at runtime.
`Body`s can be static, dynamic or kinematic.
A static `Body` is not affected by forces or collisions.
A dynamic `Body` is fully simulated.
A kinematic `Body` moves according to its velocity.
`Body` class provides methods to set its properties, such as position, velocity, etc.
It also provides methods to apply forces and impulses to it.

!!!
A `box2d.Body` in pkpy is an unified wrapper of Box2D's `b2Body`,
`b2Shape` and `b2Fixture`.
It hides the details of Box2D's API and provides a high-level interface.
!!!

## APIs

https://github.com/pocketpy/pocketpy/blob/main/include/typings/box2d.pyi

## Example

### Simple simulation

```python
import box2d
from linalg import vec2

# create a world
world = box2d.World()

# create a body with a box shape
body = box2d.Body(world)
body.set_box_shape(1, 1)

# give it a velocity
body.velocity = vec2(1, 0)

# step the simulation
for i in range(30):
    world.step(1/30, 8, 3)
    print(i, body.position)
```

```
0 vec2(0.033, 0.000)
1 vec2(0.067, 0.000)
2 vec2(0.100, 0.000)
3 vec2(0.133, 0.000)
4 vec2(0.167, 0.000)
5 vec2(0.200, 0.000)
6 vec2(0.233, 0.000)
7 vec2(0.267, 0.000)
8 vec2(0.300, 0.000)
9 vec2(0.333, 0.000)
10 vec2(0.367, 0.000)
11 vec2(0.400, 0.000)
12 vec2(0.433, 0.000)
13 vec2(0.467, 0.000)
14 vec2(0.500, 0.000)
15 vec2(0.533, 0.000)
16 vec2(0.567, 0.000)
17 vec2(0.600, 0.000)
18 vec2(0.633, 0.000)
19 vec2(0.667, 0.000)
20 vec2(0.700, 0.000)
21 vec2(0.733, 0.000)
22 vec2(0.767, 0.000)
23 vec2(0.800, 0.000)
24 vec2(0.833, 0.000)
25 vec2(0.867, 0.000)
26 vec2(0.900, 0.000)
27 vec2(0.933, 0.000)
28 vec2(0.967, 0.000)
29 vec2(1.000, 0.000)
```

### Two bodies with collision

```python
import box2d
from linalg import vec2

world = box2d.World()

"""
   12/s
B ----->          A
-|-|-|-|-|-|-|-|-|-
0 1 2 3 4 5 6 7 8 9
"""

# body_a is a static body with a box shape at (9, 0)
body_a = box2d.Body(world)
body_a.type = 0     # static type
body_a.set_box_shape(0.5, 0.5)
body_a.position = vec2(9, 0)

class Node:
    def on_box2d_contact_begin(self, other):
        print('body_a collides with body_b!!')

    def on_box2d_pre_step(self):
        pass

    def on_box2d_post_step(self):
        pass

# body_b is a dynamic body with a circle shape at (0, 0)
body_b = box2d.Body(world, Node())
body_b.set_circle_shape(0.5)
body_b.position = vec2(0, 0)

# give body_b a velocity and it will collide with body_a at some point
body_b.velocity = vec2(12, 0)

for i in range(30):
    world.step(1/30, 8, 3)
    print(i, body_b.position)
```

```
0 vec2(0.400, 0.000)
1 vec2(0.800, 0.000)
2 vec2(1.200, 0.000)
3 vec2(1.600, 0.000)
4 vec2(2.000, 0.000)
5 vec2(2.400, 0.000)
6 vec2(2.800, 0.000)
7 vec2(3.200, 0.000)
8 vec2(3.600, 0.000)
9 vec2(4.000, 0.000)
10 vec2(4.400, 0.000)
11 vec2(4.800, 0.000)
12 vec2(5.200, 0.000)
13 vec2(5.600, 0.000)
14 vec2(6.000, 0.000)
15 vec2(6.400, 0.000)
16 vec2(6.800, 0.000)
17 vec2(7.200, 0.000)
18 vec2(7.600, 0.000)
19 vec2(8.000, 0.000)
body_a collides with body_b!!
20 vec2(7.999, 0.000)
21 vec2(7.998, 0.000)
22 vec2(7.998, 0.000)
23 vec2(7.997, 0.000)
24 vec2(7.997, 0.000)
25 vec2(7.996, 0.000)
26 vec2(7.996, 0.000)
27 vec2(7.996, 0.000)
28 vec2(7.996, 0.000)
29 vec2(7.996, 0.000)
```

## Caveats

You should set the shape of the body first before accessing fixture properties.
```python
class Body:
    ...

    # fixture settings
    density: float
    friction: float
    restitution: float
    restitution_threshold: float
    is_sensor: bool
```

```python
import box2d
world = box2d.World()
body = box2d.Body(world)

body.is_sensor = True       # this will raise an error
```

The correct usage is:
```python
body = box2d.Body(world)
body.set_box_shape(1, 1)    # set shape first

body.is_sensor = True       # OK
```