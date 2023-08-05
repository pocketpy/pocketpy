#pragma once

#include "box2d/b2_world.h"
#include "box2d/box2d.h"
#include "pocketpy/pocketpy.h"

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
}

using namespace pkpy;

namespace imbox2d{

inline PyObject* get_body_object(b2Body* p){
    auto userdata = p->GetUserData().pointer;
    return reinterpret_cast<PyObject*>(userdata);
}

// maybe we will use this class later
struct PyDebugDraw: b2Draw{
    PK_ALWAYS_PASS_BY_POINTER(PyDebugDraw)

    VM* vm;
    PyObject* draw_like;

    PyDebugDraw(VM* vm): vm(vm){}

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override{
    }

    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override{
    }

    void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override{
    }

    void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override{
    }

    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override{
    }

    void DrawTransform(const b2Transform& xf) override{
    }

    void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override{
    }
};

struct PyContactListener: b2ContactListener{
    PK_ALWAYS_PASS_BY_POINTER(PyContactListener)
    VM* vm;
    PyContactListener(VM* vm): vm(vm){}

    void _contact_f(b2Contact* contact, StrName name){
        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        b2Body* bodyB = contact->GetFixtureB()->GetBody();
        PyObject* a = get_body_object(bodyA);
        PyObject* b = get_body_object(bodyB);
        PyObject* self;
        PyObject* f;
        f = vm->get_unbound_method(a, name, &self, false);
        if(f != nullptr) vm->call_method(self, f, b);
        f = vm->get_unbound_method(b, name, &self, false);
        if(f != nullptr) vm->call_method(self, f, a);
    }

	void BeginContact(b2Contact* contact) override {
        DEF_SNAME(on_contact_begin);
        _contact_f(contact, on_contact_begin);
    }

    void EndContact(b2Contact* contact) override {
        DEF_SNAME(on_contact_end);
        _contact_f(contact, on_contact_end);
    }
};

struct PyBody{
    PY_CLASS(PyBody, box2d, Body)
    PK_ALWAYS_PASS_BY_POINTER(PyBody)

    b2Body* body;
    b2Fixture* fixture;
    PyObject* node_like;

    PyBody() = default;

    void _gc_mark() {
        PK_OBJ_MARK(node_like);
    }

    PyBody& _() { return *this; }
    b2Body& _b2Body() { return *body; }
    b2Fixture& _b2Fixture() { return *fixture; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);

    // methods

    b2Vec2 get_position() const { return body->GetPosition(); }
    void set_position(b2Vec2 v){ body->SetTransform(v, body->GetAngle()); }
    float get_rotation() const { return body->GetAngle(); }
    void set_rotation(float v){ body->SetTransform(body->GetPosition(), v); }

    void apply_force(b2Vec2 force, b2Vec2 point){ body->ApplyForce(force, point, true); }
    void apply_force_to_center(b2Vec2 force){ body->ApplyForceToCenter(force, true); }
    void apply_torque(float torque){ body->ApplyTorque(torque, true); }
    void apply_impulse(b2Vec2 impulse, b2Vec2 point){
        body->ApplyLinearImpulse(impulse, point, true);
    }
    void apply_impulse_to_center(b2Vec2 impulse){
        body->ApplyLinearImpulseToCenter(impulse, true);
    }
    void apply_angular_impulse(float impulse){
        body->ApplyAngularImpulse(impulse, true);
    }
};

struct PyWorld {
    PY_CLASS(PyWorld, box2d, World)
    PK_ALWAYS_PASS_BY_POINTER(PyWorld)

    b2World world;
    PyContactListener _contact_listener;
    PyDebugDraw _debug_draw;

    PyWorld(VM* vm);

    void _gc_mark(){
        PK_OBJ_MARK(_debug_draw.draw_like);
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};


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

    void set_damping(float damping){ body->SetLinearDamping(damping); }
    float get_damping(){ return body->GetLinearDamping(); }

    void set_angular_damping(float damping){ body->SetAngularDamping(damping); }
    float get_angular_damping() const{ return body->GetAngularDamping(); }

    void set_gravity_scale(float scale){ body->SetGravityScale(scale); }
    float get_gravity_scale() const{ return body->GetGravityScale(); }

    void set_type(int type){ body->SetType(static_cast<b2BodyType>(type)); }
    int get_type() const{ return static_cast<int>(body->GetType()); }

    float get_mass() const{ return body->GetMass(); }
    float get_inertia() const{ return body->GetInertia(); }

    bool get_fixed_rotation() const{ return body->IsFixedRotation(); }
    void set_fixed_rotation(bool fixed){ body->SetFixedRotation(fixed); }

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

}   // namespace imbox2d

namespace pkpy{
    inline void add_module_box2d(VM* vm){
        PyObject* mod = vm->new_module("box2d");
        imbox2d::PyBody::register_class(vm, mod);
        imbox2d::PyWorld::register_class(vm, mod);
    }
}