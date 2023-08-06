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

## API list

```python
from linalg import vec2, vec4
from typing import Iterable

class _NodeLike:    # duck-type protocol
    def on_contact_begin(self, other: 'Body'): ...
    def on_contact_end(self, other: 'Body'): ...

class _DrawLike:    # duck-type protocol
    def draw_polygon(self, vertices: list[vec2], color: vec4): ...
    def draw_solid_polygon(self, vertices: list[vec2], color: vec4): ...
    def draw_circle(self, center: vec2, radius: float, color: vec4): ...
    def draw_solid_circle(self, center: vec2, radius: float, axis: vec2, color: vec4): ...
    def draw_segment(self, p1: vec2, p2: vec2, color: vec4): ...
    def draw_transform(self, position: vec2, rotation: float): ...
    def draw_point(self, p: vec2, size: float, color: vec4): ...

class World:
    gravity: vec2       # gravity of the world, by default vec2(0, 0)

    def get_bodies(self) -> Iterable['Body']:
        """return all bodies in the world."""

    def ray_cast(self, start: vec2, end: vec2) -> list['Body']:
        """raycast from start to end"""

    def box_cast(self, lower: vec2, upper: vec2) -> list['Body']:
        """query bodies in the AABB region."""

    def step(self, dt: float, velocity_iterations: int, position_iterations: int) -> None:
        """step the simulation, e.g. world.step(1/60, 8, 3)"""

	# enum
	# {
	# 	e_shapeBit				= 0x0001,	///< draw shapes
	# 	e_jointBit				= 0x0002,	///< draw joint connections
	# 	e_aabbBit				= 0x0004,	///< draw axis aligned bounding boxes
	# 	e_pairBit				= 0x0008,	///< draw broad-phase pairs
	# 	e_centerOfMassBit		= 0x0010	///< draw center of mass frame
	# };
    def debug_draw(self, flags: int):
        """draw debug shapes of all bodies in the world."""

    def set_debug_draw(self, draw: _DrawLike):
        """set the debug draw object."""

class Body:
    type: int           # 0-static, 1-kinematic, 2-dynamic, by default 2
    gravity_scale: float
    fixed_rotation: bool
    enabled: bool
    bullet: bool        # whether to use continuous collision detection

    @property
    def mass(self) -> float: ...
    @property
    def inertia(self) -> float: ...

    position: vec2
    rotation: float     # in radians (counter-clockwise)
    velocity: vec2      # linear velocity
    angular_velocity: float
    damping: float      # linear damping
    angular_damping: float

    # fixture settings
    density: float
    friction: float
    restitution: float
    restitution_threshold: float
    is_trigger: bool

    def __new__(cls, world: World, node: _NodeLike = None):
        """create a body in the world."""

    def set_box_shape(self, hx: float, hy: float): ...
    def set_circle_shape(self, radius: float): ...
    def set_polygon_shape(self, points: list[vec2]): ...
    def set_chain_shape(self, points: list[vec2]): ...

    def apply_force(self, force: vec2, point: vec2): ...
    def apply_force_to_center(self, force: vec2): ...
    def apply_torque(self, torque: float): ...
    def apply_impulse(self, impulse: vec2, point: vec2): ...
    def apply_impulse_to_center(self, impulse: vec2): ...
    def apply_angular_impulse(self, impulse: float): ...

    def get_node(self) -> _NodeLike:
        """return the node that is attached to this body."""

    def get_contacts(self) -> list['Body']:
        """return all bodies that are in contact with this body."""

    def destroy(self):
        """destroy this body."""
```

## Example

```python
import box2d

world = box2d.World()
body = box2d.Body(world)
```