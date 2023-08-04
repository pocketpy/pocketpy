#include "box2d_bindings.hpp"

namespace pkpy{

namespace imbox2d{



    void PyBody::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<PyBody>(type);
        PK_REGISTER_READONLY_PROPERTY(PyBody, debug_color, "vec4");

        PK_REGISTER_PROPERTY(PyBody, position, "vec2");
        PK_REGISTER_PROPERTY(PyBody, rotation, "float");
        PK_REGISTER_PROPERTY(PyBody, velocity, "vec2");
        PK_REGISTER_PROPERTY(PyBody, angular_velocity, "float");
        PK_REGISTER_PROPERTY(PyBody, damping, "float");
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






}

}   // namespace pkpy