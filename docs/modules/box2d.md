---
icon: package
label: box2d
---

[Box2D](https://box2d.org/) by Erin Catto, the world's best 2D physics engine now becomes a built-in module in pkpy `v1.1.3` and later.

## Setup

`box2d` module will be enabled **by default** for CMake users.
All platforms are supported, including desktop, mobile and web.

You can set option `PK_USE_BOX2D` to `OFF` in CMakeLists.txt
if you don't want to use it.

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

https://github.com/blueloveTH/pocketpy/blob/main/include/typings/box2d.pyi

## Example

```python
import box2d

world = box2d.World()
body = box2d.Body(world)
```