#include "box2d_bindings.hpp"

namespace pkpy{

void add_module_box2d(VM *vm){
    PyObject* mod = vm->new_module("box2d");
    imbox2d::PyBody::register_class(vm, mod);
    imbox2d::PyWorld::register_class(vm, mod);
}

namespace imbox2d{

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

    void PyBody::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<PyBody>(type);
        PK_REGISTER_READONLY_PROPERTY(PyBody, debug_color, "vec4");

        PK_REGISTER_PROPERTY(PyBody, position, "vec2");
        PK_REGISTER_PROPERTY(PyBody, rotation, "float");
        PK_REGISTER_PROPERTY(PyBody, velocity, "vec2");
        PK_REGISTER_PROPERTY(PyBody, angular_velocity, "float");
        PK_REGISTER_PROPERTY(PyBody, linear_damping, "float");
        PK_REGISTER_PROPERTY(PyBody, angular_damping, "float");
        PK_REGISTER_PROPERTY(PyBody, gravity_scale, "float");
        PK_REGISTER_PROPERTY(PyBody, type, "int");
        PK_REGISTER_READONLY_PROPERTY(PyBody, mass, "float");
        PK_REGISTER_READONLY_PROPERTY(PyBody, inertia, "float");

        // fixture settings
        PK_REGISTER_PROPERTY(PyBody, density, "float");
        PK_REGISTER_PROPERTY(PyBody, friction, "float");
        PK_REGISTER_PROPERTY(PyBody, restitution, "float");
        PK_REGISTER_PROPERTY(PyBody, restitution_threshold, "float");
        PK_REGISTER_PROPERTY(PyBody, is_trigger, "bool");

        // methods
        _bind_opaque<PyBody>(vm, type, "apply_force(self, force: vec2, point: vec2)", &Body::apply_force);
        _bind_opaque<PyBody>(vm, type, "apply_force_to_center(self, force: vec2)", &Body::apply_force_to_center);
        _bind_opaque<PyBody>(vm, type, "apply_torque(self, torque: float)", &Body::apply_torque);
        _bind_opaque<PyBody>(vm, type, "apply_linear_impulse(self, impulse: vec2, point: vec2)", &Body::apply_linear_impulse);
        _bind_opaque<PyBody>(vm, type, "apply_linear_impulse_to_center(self, impulse: vec2)", &Body::apply_linear_impulse_to_center);
        _bind_opaque<PyBody>(vm, type, "apply_angular_impulse(self, impulse: float)", &Body::apply_angular_impulse);

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            PyBody& self = _CAST(PyBody&, lhs);
            if(is_non_tagged_type(rhs, PyBody::_type(vm))) return vm->NotImplemented;
            PyBody& other = _CAST(PyBody&, rhs);
            return VAR(self->body == other->body);
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyBody& self = _CAST(PyBody&, obj);
            return VAR(fmt("<Body* at ", self->body, ">"));
        });

        // destroy
        _bind_opaque<PyBody>(vm, type, "destroy(self)", &Body::destroy);

        // contacts
        vm->bind(type, "get_contacts(self) -> list", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            b2ContactEdge* edge = self->body->GetContactList();
            List list;
            while(edge){
                b2Fixture* fixtureB = edge->contact->GetFixtureB();
                b2Body* bodyB = fixtureB->GetBody();
                PyObject* objB = reinterpret_cast<Body*>(bodyB->GetUserData().pointer)->obj;
                list.push_back(objB);
                edge = edge->next;
            }
            return VAR(std::move(list));
        });

        // userdata
        vm->bind(type, "get_node(self)", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            return self->obj;
        });

        // shape
        vm->bind(type, "set_box_shape(self, hx: float, hy: float)", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            float hx = CAST(float, args[1]);
            float hy = CAST(float, args[2]);
            b2PolygonShape shape;
            shape.SetAsBox(hx, hy);
            self->_update_fixture(&shape);
            return vm->None;
        });

        vm->bind(type, "set_circle_shape(self, radius: float)", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            float radius = CAST(float, args[1]);
            b2CircleShape shape;
            shape.m_radius = radius;
            self->_update_fixture(&shape);
            return vm->None;
        });

        vm->bind(type, "set_polygon_shape(self, points: list[vec2])", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            List& points = CAST(List&, args[1]);
            if(points.size() > b2_maxPolygonVertices || points.size() < 3){
                vm->ValueError(fmt("invalid polygon vertices count: ", points.size()));
                return vm->None;
            }
            std::vector<b2Vec2> vertices(points.size());
            for(int i = 0; i < points.size(); ++i){
                vertices[i] = CAST(b2Vec2, points[i]);
            }
            b2PolygonShape shape;
            shape.Set(vertices.data(), vertices.size());
            self->_update_fixture(&shape);
            return vm->None;
        });

        vm->bind(type, "set_chain_shape(self, points: list[vec2])", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            List& points = CAST(List&, args[1]);
            std::vector<b2Vec2> vertices(points.size());
            for(int i = 0; i < points.size(); ++i){
                vertices[i] = CAST(b2Vec2, points[i]);
            }
            b2ChainShape shape;
            shape.CreateLoop(vertices.data(), vertices.size());
            self->_update_fixture(&shape);
            return vm->None;
        });

        vm->bind(type, "get_shape_info(self) -> tuple", [](VM* vm, ArgsView args){
            PyBody& self = _CAST(PyBody&, args[0]);
            b2Shape* shape = self->fixture->GetShape();
            switch(shape->GetType()){
                case b2Shape::e_polygon:{
                    b2PolygonShape* poly = static_cast<b2PolygonShape*>(shape);
                    Tuple points(poly->m_count + 1);
                    for(int i = 0; i < poly->m_count; ++i){
                        points[i] = VAR(poly->m_vertices[i]);
                    }
                    points[poly->m_count] = points[0];
                    return VAR(Tuple({
                        VAR("polygon"), VAR(std::move(points))
                    }));
                }
                case b2Shape::e_circle:{
                    b2CircleShape* circle = static_cast<b2CircleShape*>(shape);
                    return VAR(Tuple({
                        VAR("circle"), VAR(circle->m_radius)
                    }));
                }
                case b2Shape::e_chain:{
                    b2ChainShape* chain = static_cast<b2ChainShape*>(shape);
                    Tuple points(chain->m_count);
                    for(int i = 0; i < chain->m_count; ++i){
                        points[i] = VAR(chain->m_vertices[i]);
                    }
                    return VAR(Tuple({
                        VAR("chain"), VAR(std::move(points))
                    }));
                }
                default:
                    vm->ValueError("unsupported shape type");
                    return vm->None;
            }
        });
    }

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


    void PyWorld::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind(type, "__new__(cls)", [](VM* vm, ArgsView args){
            b2World* w = new b2World(b2Vec2(0, 0));
            w->SetAllowSleeping(true);
            w->SetAutoClearForces(true);
            // the contact listener will leak memory after the world is destroyed
            // but it's not a big deal since the world is only destroyed when the game exits
            w->SetContactListener(new PyContactListener(vm));
            w->SetDebugDraw(new PyDebugDraw(vm));
            return VAR_T(PyWorld, w);
        });

        // gravity
        vm->bind_property(type, "gravity", "vec2", [](VM* vm, ArgsView args){
            PyWorld& self = _CAST(PyWorld&, args[0]);
            return VAR(self->GetGravity());
        }, [](VM* vm, ArgsView args){
            PyWorld& self = _CAST(PyWorld&, args[0]);
            self->SetGravity(CAST(b2Vec2, args[1]));
            return vm->None;
        });

        // body
        vm->bind(type, "create_body(self, obj) -> Body", [](VM* vm, ArgsView args){
            PyWorld& self = _CAST(PyWorld&, args[0]);
            return VAR_T(PyBody, new Body(self.ptr, args[1]));
        });
        vm->bind(type, "get_bodies(self) -> list[Body]", [](VM* vm, ArgsView args){
            PyWorld& self = _CAST(PyWorld&, args[0]);
            List list;
            b2Body* p = self->GetBodyList();
            while(p != nullptr){
                Body* body = (Body*)p->GetUserData().pointer;
                list.push_back(VAR_T(PyBody, body));
                p = p->GetNext();
            }
            return VAR(std::move(list));
        });

        // step
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
                f(vm, self->GetBodyList(), on_box2d_pre_step);
                self->Step(dt, velocity_iterations, position_iterations);
                f(vm, self->GetBodyList(), on_box2d_post_step);
                return vm->None;
            });

        // raycast
        vm->bind(type, "raycast(self, start: vec2, end: vec2) -> list[Body]", [](VM* vm, ArgsView args){
            auto _lock = vm->heap.gc_scope_lock();
            PyWorld& self = _CAST(PyWorld&, args[0]);
            b2Vec2 start = CAST(b2Vec2, args[1]);
            b2Vec2 end = CAST(b2Vec2, args[2]);
            MyRayCastCallback callback(vm);
            self->RayCast(&callback, start, end);
            return VAR(std::move(callback.result));
        });
    }

}

}   // namespace pkpy