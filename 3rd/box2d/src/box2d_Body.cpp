#include "box2dw.hpp"

namespace pkpy{

void PyBody::_register(VM* vm, PyObject* mod, PyObject* type){
    vm->bind(type, "__new__(cls, world: World, node: _NodeLike = None)",
        [](VM* vm, ArgsView args){
            PyWorld& world = CAST(PyWorld&, args[1]);
            PyObject* node = args[2];
            PyObject* obj = vm->heap.gcnew<PyBody>(PyBody::_type(vm));
            PyBody& body = _CAST(PyBody&, obj);
            b2BodyDef def;
            def.type = b2_dynamicBody;
            // a weak reference to this object
            def.userData.pointer = reinterpret_cast<uintptr_t>(obj);
            body.body = world.world.CreateBody(&def);
            body.node_like = node;
            return obj;
        });

    PY_PROPERTY(PyBody, "type: int", _b2Body, GetType, SetType)
    PY_PROPERTY(PyBody, "gravity_scale: float", _b2Body, GetGravityScale, SetGravityScale)
    PY_PROPERTY(PyBody, "fixed_rotation: bool", _b2Body, IsFixedRotation, SetFixedRotation)
    PY_PROPERTY(PyBody, "enabled: bool", _b2Body, IsEnabled, SetEnabled)
    PY_PROPERTY(PyBody, "bullet: bool", _b2Body, IsBullet, SetBullet)
    
    PY_READONLY_PROPERTY(PyBody, "mass: float", _b2Body, GetMass)
    PY_READONLY_PROPERTY(PyBody, "inertia: float", _b2Body, GetInertia)

    PY_PROPERTY(PyBody, "position: vec2", _, get_position, set_position)
    PY_PROPERTY(PyBody, "rotation: float", _, get_rotation, set_rotation)
    PY_PROPERTY(PyBody, "velocity: vec2", _, get_velocity, set_velocity)
    PY_PROPERTY(PyBody, "angular_velocity: float", _b2Body, GetAngularVelocity, SetAngularVelocity)
    PY_PROPERTY(PyBody, "damping: float", _b2Body, GetLinearDamping, SetLinearDamping)
    PY_PROPERTY(PyBody, "angular_damping: float", _b2Body, GetAngularDamping, SetAngularDamping)

    PY_PROPERTY(PyBody, "density: float", _b2Fixture, GetDensity, SetDensity)
    PY_PROPERTY(PyBody, "friction: float", _b2Fixture, GetFriction, SetFriction)
    PY_PROPERTY(PyBody, "restitution: float", _b2Fixture, GetRestitution, SetRestitution)
    PY_PROPERTY(PyBody, "restitution_threshold: float", _b2Fixture, GetRestitutionThreshold, SetRestitutionThreshold)
    PY_PROPERTY(PyBody, "is_sensor: bool", _b2Fixture, IsSensor, SetSensor)

    vm->bind(type, "set_box_shape(self, hx: float, hy: float)",
        [](VM* vm, ArgsView args){
            PyBody& body = CAST(PyBody&, args[0]);
            float hx = CAST(float, args[1]);
            float hy = CAST(float, args[2]);
            b2PolygonShape shape;
            shape.SetAsBox(hx, hy);
            body._set_b2Fixture(body.body->CreateFixture(&shape, 1.0f));
            return vm->None;
        });

    vm->bind(type, "set_circle_shape(self, radius: float)",
        [](VM* vm, ArgsView args){
            PyBody& body = CAST(PyBody&, args[0]);
            float radius = CAST(float, args[1]);
            b2CircleShape shape;
            shape.m_radius = radius;
            body._set_b2Fixture(body.body->CreateFixture(&shape, 1.0f));
            return vm->None;
        });

    vm->bind(type, "set_polygon_shape(self, points: list[vec2])",
        [](VM* vm, ArgsView args){
            PyBody& body = CAST(PyBody&, args[0]);
            List& points = CAST(List&, args[1]);
            if(points.size() < 3 || points.size() > b2_maxPolygonVertices){
                vm->ValueError("invalid vertices count");
            }
            b2PolygonShape shape;
            std::vector<b2Vec2> vertices;
            for(auto& point : points){
                Vec2 vec = CAST(Vec2, point);
                vertices.push_back(b2Vec2(vec.x, vec.y));
            }
            shape.Set(vertices.data(), vertices.size());
            body._set_b2Fixture(body.body->CreateFixture(&shape, 1.0f));
            return vm->None;
        });

    vm->bind(type, "set_chain_shape(self, points: list[vec2])",
        [](VM* vm, ArgsView args){
            PyBody& body = CAST(PyBody&, args[0]);
            List& points = CAST(List&, args[1]);
            if(points.size() < 3){
                vm->ValueError("invalid vertices count");
            }
            b2ChainShape shape;
            std::vector<b2Vec2> vertices;
            for(auto& point : points){
                Vec2 vec = CAST(Vec2, point);
                vertices.push_back(b2Vec2(vec.x, vec.y));
            }
            shape.CreateLoop(vertices.data(), vertices.size());
            body._set_b2Fixture(body.body->CreateFixture(&shape, 1.0f));
            return vm->None;
        });

    // methods
    _bind(vm, type, "apply_force(self, force: vec2, point: vec2)", &PyBody::apply_force);
    _bind(vm, type, "apply_force_to_center(self, force: vec2)", &PyBody::apply_force_to_center);
    _bind(vm, type, "apply_torque(self, torque: float)", &PyBody::apply_torque);
    _bind(vm, type, "apply_impulse(self, impulse: vec2, point: vec2)", &PyBody::apply_impulse);
    _bind(vm, type, "apply_impulse_to_center(self, impulse: vec2)", &PyBody::apply_impulse_to_center);
    _bind(vm, type, "apply_angular_impulse(self, impulse: float)", &PyBody::apply_angular_impulse);

    // get_node
    vm->bind(type, "get_node(self)", [](VM* vm, ArgsView args){
        PyBody& body = CAST(PyBody&, args[0]);
        return body.node_like;
    });

    // get_contacts
    vm->bind(type, "get_contacts(self) -> list[Body]", [](VM* vm, ArgsView args){
        PyBody& self = _CAST(PyBody&, args[0]);
        b2ContactEdge* edge = self.body->GetContactList();
        List list;
        while(edge){
            b2Fixture* fixtureB = edge->contact->GetFixtureB();
            b2Body* bodyB = fixtureB->GetBody();
            list.push_back(get_body_object(bodyB));
            edge = edge->next;
        }
        return VAR(std::move(list));
    });

    // destroy
    vm->bind(type, "destroy(self)", [](VM* vm, ArgsView args){
        PyBody& body = CAST(PyBody&, args[0]);
        body._is_destroyed = true;  // mark as destroyed
        return vm->None;
    });
}

}   // namespace pkpy