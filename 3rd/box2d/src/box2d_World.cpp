#include "box2dw.hpp"

namespace pkpy{

struct MyRayCastCallback: b2RayCastCallback{
    PK_ALWAYS_PASS_BY_POINTER(MyRayCastCallback)

    VM* vm;
    List result;
    MyRayCastCallback(VM* vm): vm(vm) {}
 
    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction){
        result.push_back(get_body_object(fixture->GetBody()));
        // if(only_one) return 0;
        return fraction;
    }
};

struct MyBoxCastCallback: b2QueryCallback{
    PK_ALWAYS_PASS_BY_POINTER(MyBoxCastCallback)

    VM* vm;
    List result;
    MyBoxCastCallback(VM* vm): vm(vm) {}

    bool ReportFixture(b2Fixture* fixture) override{
        result.push_back(get_body_object(fixture->GetBody()));
        return true;
    }
};

void PyContactListener::_contact_f(b2Contact* contact, StrName name){
    PyObject* a = get_body_object(contact->GetFixtureA()->GetBody());
    PyObject* b = get_body_object(contact->GetFixtureB()->GetBody());
    PyBody& bodyA = PK_OBJ_GET(PyBody, a);
    PyBody& bodyB = PK_OBJ_GET(PyBody, b);
    PyObject* self;
    PyObject* f;
    f = vm->get_unbound_method(bodyA.node_like, name, &self, false);
    if(f != nullptr) vm->call_method(self, f, b);
    f = vm->get_unbound_method(bodyB.node_like, name, &self, false);
    if(f != nullptr) vm->call_method(self, f, a);
}

/****************** PyWorld ******************/
PyWorld::PyWorld(VM* vm): world(b2Vec2(0, 0)), _contact_listener(vm), _debug_draw(vm){
    _debug_draw.draw_like = vm->None;
    world.SetAllowSleeping(true);
    world.SetAutoClearForces(true);
    world.SetContactListener(&_contact_listener);
    world.SetDebugDraw(&_debug_draw);
}

void PyWorld::_register(VM* vm, PyObject* mod, PyObject* type){
    vm->bind(type, "__new__(cls)", [](VM* vm, ArgsView args){
        return vm->heap.gcnew<PyWorld>(PyWorld::_type(vm), vm);
    });

    // gravity
    vm->bind_property(type, "gravity: vec2", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        return VAR(self.world.GetGravity());
    }, [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        self.world.SetGravity(CAST(b2Vec2, args[1]));
        return vm->None;
    });

    vm->bind(type, "get_bodies(self) -> list[Body]", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        List list;
        b2Body* p = self.world.GetBodyList();
        while(p != nullptr){
            list.push_back(get_body_object(p));
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
        self.world.RayCast(&callback, start, end);
        return VAR(std::move(callback.result));
    });

    vm->bind(type, "box_cast(self, lower: vec2, upper: vec2) -> list[Body]", [](VM* vm, ArgsView args){
        auto _lock = vm->heap.gc_scope_lock();
        PyWorld& self = _CAST(PyWorld&, args[0]);
        b2AABB aabb;
        aabb.lowerBound = CAST(b2Vec2, args[1]);
        aabb.upperBound = CAST(b2Vec2, args[2]);
        MyBoxCastCallback callback(vm);
        self.world.QueryAABB(&callback, aabb);
        return VAR(std::move(callback.result));
    });

    vm->bind(type, "point_cast(self, point: vec2) -> list[Body]", [](VM* vm, ArgsView args){
        auto _lock = vm->heap.gc_scope_lock();
        PyWorld& self = _CAST(PyWorld&, args[0]);
        b2AABB aabb;
        aabb.lowerBound = CAST(b2Vec2, args[1]);
        aabb.upperBound = CAST(b2Vec2, args[1]);
        MyBoxCastCallback callback(vm);
        self.world.QueryAABB(&callback, aabb);
        return VAR(std::move(callback.result));
    });

    vm->bind(type, "step(self, dt: float, velocity_iterations: int, position_iterations: int)",
        [](VM* vm, ArgsView args){
            // disable gc during step for safety
            auto _lock = vm->heap.gc_scope_lock();

            PyWorld& self = _CAST(PyWorld&, args[0]);
            float dt = CAST(float, args[1]);
            int velocity_iterations = CAST(int, args[2]);
            int position_iterations = CAST(int, args[3]);

            auto f = [](VM* vm, b2Body* p, StrName name){
                while(p != nullptr){
                    PyObject* body_obj = get_body_object(p);
                    PyBody& body = _CAST(PyBody&, body_obj);
                    if(!body._is_destroyed){
                        if(body.node_like != vm->None){
                            vm->call_method(body.node_like, name);
                        }
                    }
                    p = p->GetNext();
                }
            };

            DEF_SNAME(on_box2d_pre_step);
            DEF_SNAME(on_box2d_post_step);
            f(vm, self.world.GetBodyList(), on_box2d_pre_step);
            self.world.Step(dt, velocity_iterations, position_iterations);
            f(vm, self.world.GetBodyList(), on_box2d_post_step);

            // destroy bodies which are marked as destroyed
            b2Body* p = self.world.GetBodyList();
            while(p != nullptr){
                b2Body* next = p->GetNext();
                PyBody& body = _CAST(PyBody&, get_body_object(p));
                if(body._is_destroyed){
                    body.body->GetWorld()->DestroyBody(body.body);
                    body.body = nullptr;
                    body._fixture = nullptr;
                    body.node_like = nullptr;
                }
                p = next;
            }
            return vm->None;
        });

    vm->bind(type, "debug_draw(self, flags: int)", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        int flags = CAST(int, args[1]);
        self._debug_draw.SetFlags(flags);
        self.world.DebugDraw();
        return vm->None;
    });

    vm->bind(type, "set_debug_draw(self, draw: _DrawLike)", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        self._debug_draw.draw_like = args[1];
        return vm->None;
    });

    // joints
    vm->bind(type, "create_weld_joint(self, a, b)", [](VM* vm, ArgsView args){
        PyWorld& self = _CAST(PyWorld&, args[0]);
        PyBody& bodyA = CAST(PyBody&, args[1]);
        PyBody& bodyB = CAST(PyBody&, args[2]);
        b2WeldJointDef def;
        def.Initialize(bodyA.body, bodyB.body, bodyA.body->GetWorldCenter());
        b2Joint* p = self.world.CreateJoint(&def);
        return VAR(p);      // void_p
    });
}

}   // namespace pkpy