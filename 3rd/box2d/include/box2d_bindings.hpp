#pragma once

#include "box2d/box2d.h"
#include "pocketpy/pocketpy.h"
#include <cstdlib>

namespace pkpy{

template<>
inline b2Vec2 py_cast<b2Vec2>(VM* vm, PyObject* obj){
    Vec2 v = py_cast<Vec2>(vm, obj);
    return b2Vec2(v.x, v.y);
}

template<>
inline b2Vec2 _py_cast<b2Vec2>(VM* vm, PyObject* obj){
    Vec2 v = _py_cast<Vec2>(vm, obj);
    return b2Vec2(v.x, v.y);
}

inline PyObject* py_var(VM* vm, b2Vec2 v){
    return py_var(vm, Vec2(v.x, v.y));
}

namespace imbox2d{

struct Body final{
    b2Body* body;
    b2Fixture* fixture;
    PyObject* obj;
    Vec4 debug_color;

    Body(b2World* world, PyObject* obj){
        b2BodyDef def;
        def.type = b2_dynamicBody;
        // a weak reference to the object, no need to mark it
        def.userData.pointer = reinterpret_cast<uintptr_t>(this);
        body = world->CreateBody(&def);
        fixture = nullptr;
        this->obj = obj;
        this->debug_color = Vec4(std::rand() / float(RAND_MAX), std::rand() / float(RAND_MAX), std::rand() / float(RAND_MAX), 1.0f);
    }

    void _update_fixture(b2Shape* shape){
        body->DestroyFixture(fixture);  // this takes care of NULL case
        fixture = body->CreateFixture(shape, 1.0f);
    }

    Vec4 get_debug_color() const{ return debug_color; }

    b2Vec2 get_position() const{ return body->GetPosition(); }
    void set_position(b2Vec2 v){ body->SetTransform(v, get_rotation()); }

    void set_rotation(float angle){ body->SetTransform(get_position(), angle); }
    float get_rotation() const{ return body->GetAngle(); }

    void set_velocity(b2Vec2 v){ body->SetLinearVelocity(v); }
    b2Vec2 get_velocity() const{ return body->GetLinearVelocity(); }

    void set_angular_velocity(float omega){ body->SetAngularVelocity(omega); }
    float get_angular_velocity() const{ return body->GetAngularVelocity(); }

    void set_linear_damping(float damping){ body->SetLinearDamping(damping); }
    float get_linear_damping(){ return body->GetLinearDamping(); }

    void set_angular_damping(float damping){ body->SetAngularDamping(damping); }
    float get_angular_damping() const{ return body->GetAngularDamping(); }

    void set_gravity_scale(float scale){ body->SetGravityScale(scale); }
    float get_gravity_scale() const{ return body->GetGravityScale(); }

    void set_type(int type){ body->SetType(static_cast<b2BodyType>(type)); }
    int get_type() const{ return static_cast<int>(body->GetType()); }

    float get_mass() const{ return body->GetMass(); }
    float get_inertia() const{ return body->GetInertia(); }

    // fixture settings
    float get_density() const{ return fixture->GetDensity(); }
    void set_density(float density){ fixture->SetDensity(density); }

    float get_friction() const{ return fixture->GetFriction(); }
    void set_friction(float friction){ fixture->SetFriction(friction); }

    float get_restitution() const{ return fixture->GetRestitution(); }
    void set_restitution(float restitution){ fixture->SetRestitution(restitution); }

    float get_restitution_threshold() const{ return fixture->GetRestitutionThreshold(); }
    void set_restitution_threshold(float threshold){ fixture->SetRestitutionThreshold(threshold); }

    bool get_is_trigger() const{ return fixture->IsSensor(); }
    void set_is_trigger(bool trigger){ fixture->SetSensor(trigger); }

    // methods
    void apply_force(b2Vec2 force, b2Vec2 point){
        body->ApplyForce(force, point, true);
    }

    void apply_force_to_center(b2Vec2 force){
        body->ApplyForceToCenter(force, true);
    }

    void apply_torque(float torque){
        body->ApplyTorque(torque, true);
    }

    void apply_linear_impulse(b2Vec2 impulse, b2Vec2 point){
        body->ApplyLinearImpulse(impulse, point, true);
    }

    void apply_linear_impulse_to_center(b2Vec2 impulse){
        body->ApplyLinearImpulseToCenter(impulse, true);
    }

    void apply_angular_impulse(float impulse){
        body->ApplyAngularImpulse(impulse, true);
    }

    void destroy(){
        if(body == nullptr) return;
        body->GetWorld()->DestroyBody(body);
        body = nullptr;
    }
};

struct PyBody: OpaquePointer<Body>{
    PY_CLASS(PyBody, box2d, Body)

    using OpaquePointer<Body>::OpaquePointer;
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyWorld: OpaquePointer<b2World>{
    PY_CLASS(PyWorld, box2d, World)

    using OpaquePointer<b2World>::OpaquePointer;
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

}   // namespace imbox2d

void add_module_box2d(VM* vm);

}   // namespace pkpy