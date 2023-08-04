#include "box2d/b2_world.h"
#include "box2d/b2_world_callbacks.h"
#include "box2d_bindings.hpp"

namespace pkpy{
    namespace imbox2d{

// This class captures the closest hit shape.
class MyRayCastCallback : public b2RayCastCallback
{
    VM* vm;
public:
    List result;
    MyRayCastCallback(VM* vm): vm(vm) {}
 
    float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                        const b2Vec2& normal, float fraction)
    {
        auto userdata = fixture->GetBody()->GetUserData().pointer;
        Body* body = reinterpret_cast<Body*>(userdata);
        result.push_back(VAR_T(PyBody, body));
        // if(only_one) return 0;
        return fraction;
    }
};

class MyBoxCastCallback: public b2QueryCallback{
    VM* vm;
public:
    List result;
    MyBoxCastCallback(VM* vm): vm(vm) {}

    bool ReportFixture(b2Fixture* fixture) override{
        auto userdata = fixture->GetBody()->GetUserData().pointer;
        Body* body = reinterpret_cast<Body*>(userdata);
        result.push_back(VAR_T(PyBody, body));
        return true;
    }
};

// maybe we will use this class later
class PyDebugDraw: public b2Draw{
    VM* vm;
public:
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

class PyContactListener : public b2ContactListener{
    VM* vm;
public:
    PyContactListener(VM* vm): vm(vm){}

    void _contact_f(b2Contact* contact, StrName name){
        auto a = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        auto b = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        Body* bodyA = reinterpret_cast<Body*>(a);
        Body* bodyB = reinterpret_cast<Body*>(b);
        PyObject* self;
        PyObject* f;
        f = vm->get_unbound_method(bodyA->obj, name, &self, false);
        if(f != nullptr) vm->call_method(self, f, VAR_T(PyBody, bodyB));
        f = vm->get_unbound_method(bodyB->obj, name, &self, false);
        if(f != nullptr) vm->call_method(self, f, VAR_T(PyBody, bodyA));
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

// implement placement VAR_T...!!!!
struct PyWorld {
    PY_CLASS(PyWorld, box2d, World)

    // this object is too large, so we use unique_ptr
    std::unique_ptr<b2World> world;
    std::unique_ptr<PyContactListener> _contact_listener;
    std::unique_ptr<PyDebugDraw> _debug_draw;

    PyWorld(VM* vm):
            world(new b2World(b2Vec2(0, 0))),
            _contact_listener(new PyContactListener(vm)),
            _debug_draw(new PyDebugDraw(vm)){
        world->SetAllowSleeping(true);
        world->SetAutoClearForces(true);
        world->SetContactListener(_contact_listener.get());
        world->SetDebugDraw(_debug_draw.get());
    }

    PyWorld(const PyWorld&) = delete;
    PyWorld& operator=(const PyWorld&) = delete;

    static void _register(VM* vm, PyObject* mod, PyObject* type){
    vm->bind(type, "__new__(cls)", [](VM* vm, ArgsView args){
        return VAR_T(PyWorld, PyWorld(vm));
    });

    // gravity
    vm->bind_property(type, "gravity", "vec2", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        return VAR(self.world->GetGravity());
    }, [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        self.world->SetGravity(CAST(b2Vec2, args[1]));
        return vm->None;
    });

    vm->bind(type, "get_bodies(self) -> list[Body]", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        List list;
        b2Body* p = self.world->GetBodyList();
        while(p != nullptr){
            Body* body = (Body*)p->GetUserData().pointer;
            list.push_back(VAR_T(PyBody, body));
            p = p->GetNext();
        }
        return VAR(std::move(list));
    });

    vm->bind(type, "ray_cast(self, start: vec2, end: vec2) -> list[Body]", [](VM* vm, ArgsView args){
        auto _lock = vm->heap.gc_scope_lock();
        PyWorld& self = _CAST(PyWorld&, args[0]);
        b2Vec2 start = CAST(b2Vec2, args[1]);
        b2Vec2 end = CAST(b2Vec2, args[2]);
        MyRayCastCallback callback(vm);
        self.world->RayCast(&callback, start, end);
        return VAR(std::move(callback.result));
    });

    vm->bind(type, "box_cast(self, lower: vec2, upper: vec2) -> list[Body]", [](VM* vm, ArgsView args){
        auto _lock = vm->heap.gc_scope_lock();
        PyWorld& self = _CAST(PyWorld&, args[0]);
        b2AABB aabb;
        aabb.lowerBound = CAST(b2Vec2, args[1]);
        aabb.upperBound = CAST(b2Vec2, args[2]);
        MyBoxCastCallback callback(vm);
        self.world->QueryAABB(&callback, aabb);
        return VAR(std::move(callback.result));
    });

    vm->bind(type, "step(self, dt: float, velocity_iterations: int, position_iterations: int)",
        [](VM* vm, ArgsView args){
            PyWorld& self = _CAST(PyWorld&, args[0]);
            float dt = CAST(float, args[1]);
            int velocity_iterations = CAST(int, args[2]);
            int position_iterations = CAST(int, args[3]);

            auto f = [](VM* vm, b2Body* p, StrName name){
                while(p != nullptr){
                    Body* body = (Body*)p->GetUserData().pointer;
                    vm->call_method(body->obj, name);
                    p = p->GetNext();
                }
            };

            DEF_SNAME(on_box2d_pre_step);
            DEF_SNAME(on_box2d_post_step);
            f(vm, self.world->GetBodyList(), on_box2d_pre_step);
            self.world->Step(dt, velocity_iterations, position_iterations);
            f(vm, self.world->GetBodyList(), on_box2d_post_step);
            return vm->None;
        });
    }
};

    }   // namespace imbox2d
}   // namespace pkpy