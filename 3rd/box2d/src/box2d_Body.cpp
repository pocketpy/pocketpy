#include "box2d/b2_world.h"
#include "box2d/b2_world_callbacks.h"
#include "box2d_bindings.hpp"

using namespace pkpy;

namespace imbox2d{


void PyBody::_register(VM* vm, PyObject* mod, PyObject* type){
    vm->bind(type, "__new__(cls, world: World, node: _NodeLike = None)",
        [](VM* vm, ArgsView args){
            PyWorld& world = CAST(PyWorld&, args[1]);
            PyObject* node = args[2];
            PyObject* obj = VAR_T(PyBody, PyBody());
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
}

}   // namespace imbox2d