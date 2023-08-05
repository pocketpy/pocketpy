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

}   // namespace imbox2d

namespace pkpy{
    inline void add_module_box2d(VM* vm){
        PyObject* mod = vm->new_module("box2d");
        imbox2d::PyBody::register_class(vm, mod);
        imbox2d::PyWorld::register_class(vm, mod);
    }
}