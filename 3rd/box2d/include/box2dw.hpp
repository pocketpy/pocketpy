#pragma once

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

inline PyObject* get_body_object(b2Body* p){
    auto userdata = p->GetUserData().pointer;
    return reinterpret_cast<PyObject*>(userdata);
}

// maybe we will use this class later
struct PyDebugDraw: b2Draw{
    PK_ALWAYS_PASS_BY_POINTER(PyDebugDraw)

    VM* vm;
    PyObject* draw_like;    // world will mark this

    PyDebugDraw(VM* vm): vm(vm){}

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
    void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
    void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
    void DrawTransform(const b2Transform& xf) override;
    void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
};

struct PyContactListener: b2ContactListener{
    PK_ALWAYS_PASS_BY_POINTER(PyContactListener)
    VM* vm;
    PyContactListener(VM* vm): vm(vm){}

    void _contact_f(b2Contact* contact, StrName name);

	void BeginContact(b2Contact* contact) override {
        DEF_SNAME(on_box2d_contact_begin);
        _contact_f(contact, on_box2d_contact_begin);
    }

    void EndContact(b2Contact* contact) override {
        DEF_SNAME(on_box2d_contact_end);
        _contact_f(contact, on_box2d_contact_end);
    }
};

struct PyBody{
    PY_CLASS(PyBody, box2d, Body)
    PK_ALWAYS_PASS_BY_POINTER(PyBody)

    b2Body* body;
    b2Fixture* _fixture;
    PyObject* node_like;

    bool _is_destroyed;
    PyBody(): body(nullptr), _fixture(nullptr), node_like(nullptr), _is_destroyed(false){}

    void _gc_mark() {
        if(node_like != nullptr){
            PK_OBJ_MARK(node_like);
        }
    }

    PyBody* _() { return this; }
    b2Body* _b2Body() { return body; }

    b2Fixture* _b2Fixture() {
        if(_fixture == nullptr) throw std::runtime_error("`_fixture == nullptr` in PyBody::_b2Fixture()");
        return _fixture;
    }

    void _set_b2Fixture(b2Fixture* fixture){
        if(_fixture != nullptr){
            body->DestroyFixture(_fixture);
        }
        _fixture = fixture;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type);

    // methods
    b2Vec2 get_position() const { return body->GetPosition(); }
    void set_position(b2Vec2 v){ body->SetTransform(v, body->GetAngle()); }
    float get_rotation() const { return body->GetAngle(); }
    void set_rotation(float v){ body->SetTransform(body->GetPosition(), v); }
    b2Vec2 get_velocity() const { return body->GetLinearVelocity(); }
    void set_velocity(b2Vec2 v){ body->SetLinearVelocity(v); }

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

inline void add_module_box2d(VM* vm){
    PyObject* mod = vm->new_module("box2d");
    PyBody::register_class(vm, mod);
    PyWorld::register_class(vm, mod);
}

}   // namespace pkpy