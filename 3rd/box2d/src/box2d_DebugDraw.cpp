#include "box2dw.hpp"

namespace pkpy{

// def draw_polygon(self, vertices: list[vec2], color: vec4): ...
// def draw_solid_polygon(self, vertices: list[vec2], color: vec4): ...
// def draw_circle(self, center: vec2, radius: float, color: vec4): ...
// def draw_solid_circle(self, center: vec2, radius: float, axis: vec2, color: vec4): ...
// def draw_segment(self, p1: vec2, p2: vec2, color: vec4): ...
// def draw_transform(self, position: vec2, rotation: float): ...
// def draw_point(self, p: vec2, size: float, color: vec4): ...

static Vec4 color_to_vec4(const b2Color& color){
    return Vec4(color.r, color.g, color.b, color.a);
}

void PyDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color){
    DEF_SNAME(draw_polygon);
    List v(vertexCount);
    for(int i = 0; i < vertexCount; i++) v[i] = VAR(vertices[i]);
    PyObject* col = VAR(color_to_vec4(color));
    vm->call_method(draw_like, draw_polygon, VAR(std::move(v)), col);
}

void PyDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color){
    DEF_SNAME(draw_solid_polygon);
    List v(vertexCount);
    for(int i = 0; i < vertexCount; i++) v[i] = VAR(vertices[i]);
    PyObject* col = VAR(color_to_vec4(color));
    vm->call_method(draw_like, draw_solid_polygon, VAR(std::move(v)), col);
}

void PyDebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color){
    DEF_SNAME(draw_circle);
    PyObject* col = VAR(color_to_vec4(color));
    vm->call_method(draw_like, draw_circle, VAR(center), VAR(radius), col);
}

void PyDebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color){
    DEF_SNAME(draw_solid_circle);
    PyObject* col = VAR(color_to_vec4(color));
    vm->call_method(draw_like, draw_solid_circle, VAR(center), VAR(radius), VAR(axis), col);
}

void PyDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color){
    DEF_SNAME(draw_segment);
    PyObject* col = VAR(color_to_vec4(color));
    vm->call_method(draw_like, draw_segment, VAR(p1), VAR(p2), col);
}

void PyDebugDraw::DrawTransform(const b2Transform& xf){
    DEF_SNAME(draw_transform);
    vm->call_method(draw_like, draw_transform, VAR(xf.p), VAR(xf.q.GetAngle()));
}

void PyDebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color){
    DEF_SNAME(draw_point);
    PyObject* col = VAR(color_to_vec4(color));
    vm->call_method(draw_like, draw_point, VAR(p), VAR(size), col);
}

}