#include "box2d/b2_world.h"
#include "box2d/b2_world_callbacks.h"
#include "box2d_bindings.hpp"
#include "pocketpy/bindings.h"

namespace pkpy{
namespace imbox2d{

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
            body.fixture = nullptr;
            body.node_like = node;
            return obj;
        });

    PK_REGISTER_PROPERTY(PyBody, "type: int", _b2Body, GetType, SetType)
    PK_REGISTER_PROPERTY(PyBody, "gravity_scale: float", _b2Body, GetGravityScale, SetGravityScale)
    PK_REGISTER_PROPERTY(PyBody, "fixed_rotation: bool", _b2Body, IsFixedRotation, SetFixedRotation)
    PK_REGISTER_PROPERTY(PyBody, "enabled: bool", _b2Body, IsEnabled, SetEnabled)
    PK_REGISTER_PROPERTY(PyBody, "bullet: bool", _b2Body, IsBullet, SetBullet)
    
    PK_REGISTER_READONLY_PROPERTY(PyBody, "mass: float", _b2Body, GetMass)
    PK_REGISTER_READONLY_PROPERTY(PyBody, "inertia: float", _b2Body, GetInertia)

    PK_REGISTER_PROPERTY(PyBody, "position: vec2", _, get_position, set_position)
    PK_REGISTER_PROPERTY(PyBody, "rotation: float", _, get_rotation, set_rotation)
    PK_REGISTER_PROPERTY(PyBody, "velocity: vec2", _b2Body, GetLinearVelocity, SetLinearVelocity)
    PK_REGISTER_PROPERTY(PyBody, "angular_velocity: float", _b2Body, GetAngularVelocity, SetAngularVelocity)
    PK_REGISTER_PROPERTY(PyBody, "damping: float", _b2Body, GetLinearDamping, SetLinearDamping)
    PK_REGISTER_PROPERTY(PyBody, "angular_damping: float", _b2Body, GetAngularDamping, SetAngularDamping)

    PK_REGISTER_PROPERTY(PyBody, "density: float", _b2Fixture, GetDensity, SetDensity)
    PK_REGISTER_PROPERTY(PyBody, "friction: float", _b2Fixture, GetFriction, SetFriction)
    PK_REGISTER_PROPERTY(PyBody, "restitution: float", _b2Fixture, GetRestitution, SetRestitution)
    PK_REGISTER_PROPERTY(PyBody, "restitution_threshold: float", _b2Fixture, GetRestitutionThreshold, SetRestitutionThreshold)
    PK_REGISTER_PROPERTY(PyBody, "is_trigger: bool", _b2Fixture, IsSensor, SetSensor)

    // methods
    _bind(vm, type, "apply_force(self, force: vec2, point: vec2)", &PyBody::apply_force);
    _bind(vm, type, "apply_force_to_center(self, force: vec2)", &PyBody::apply_force_to_center);
    _bind(vm, type, "apply_torque(self, torque: float)", &PyBody::apply_torque);
    _bind(vm, type, "apply_impulse(self, impulse: vec2, point: vec2)", &PyBody::apply_impulse);
    _bind(vm, type, "apply_impulse_to_center(self, impulse: vec2)", &PyBody::apply_impulse_to_center);
    _bind(vm, type, "apply_angular_impulse(self, impulse: float)", &PyBody::apply_angular_impulse);

    // get_node
    vm->bind(type, "get_node(self)", [](VM* vm, ArgsView args){
        PyBody& body = CAST(PyBody&, args[1]);
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
        PyBody& body = CAST(PyBody&, args[1]);
        body.body->GetWorld()->DestroyBody(body.body);
        body.body = nullptr;
        body.fixture = nullptr;
        body.node_like = nullptr;
        return vm->None;
    });
}

}   // namespace imbox2d
}   // namespace pkpy