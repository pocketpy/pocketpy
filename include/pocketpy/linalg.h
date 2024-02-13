#pragma once

#include "bindings.h"

namespace pkpy{

inline bool isclose(float a, float b){ return std::fabs(a - b) <= NumberTraits<4>::kEpsilon; }

struct Vec2{
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const Vec2& v) = default;

    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator*(const Vec2& v) const { return Vec2(x * v.x, y * v.y); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2 operator-() const { return Vec2(-x, -y); }
    bool operator==(const Vec2& v) const { return isclose(x, v.x) && isclose(y, v.y); }
    bool operator!=(const Vec2& v) const { return !isclose(x, v.x) || !isclose(y, v.y); }
    float dot(const Vec2& v) const { return x * v.x + y * v.y; }
    float cross(const Vec2& v) const { return x * v.y - y * v.x; }
    float length() const { return sqrtf(x * x + y * y); }
    float length_squared() const { return x * x + y * y; }
    Vec2 normalize() const { float l = length(); return Vec2(x / l, y / l); }
    Vec2 rotate(float radian) const { float cr = cosf(radian), sr = sinf(radian); return Vec2(x * cr - y * sr, x * sr + y * cr); }
    NoReturn normalize_() { float l = length(); x /= l; y /= l; return {}; }
    NoReturn copy_(const Vec2& v) { x = v.x; y = v.y; return {}; }
};

struct Vec3{
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(const Vec3& v) = default;

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    bool operator==(const Vec3& v) const { return isclose(x, v.x) && isclose(y, v.y) && isclose(z, v.z); }
    bool operator!=(const Vec3& v) const { return !isclose(x, v.x) || !isclose(y, v.y) || !isclose(z, v.z); }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    float length_squared() const { return x * x + y * y + z * z; }
    Vec3 normalize() const { float l = length(); return Vec3(x / l, y / l, z / l); }
    NoReturn normalize_() { float l = length(); x /= l; y /= l; z /= l; return {}; }
    NoReturn copy_(const Vec3& v) { x = v.x; y = v.y; z = v.z; return {}; }
};

struct Vec4{
    float x, y, z, w;
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec4& v) = default;

    Vec4 operator+(const Vec4& v) const { return Vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vec4 operator-(const Vec4& v) const { return Vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4 operator*(const Vec4& v) const { return Vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
    Vec4 operator/(float s) const { return Vec4(x / s, y / s, z / s, w / s); }
    Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
    bool operator==(const Vec4& v) const { return isclose(x, v.x) && isclose(y, v.y) && isclose(z, v.z) && isclose(w, v.w); }
    bool operator!=(const Vec4& v) const { return !isclose(x, v.x) || !isclose(y, v.y) || !isclose(z, v.z) || !isclose(w, v.w); }
    float dot(const Vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    float length() const { return sqrtf(x * x + y * y + z * z + w * w); }
    float length_squared() const { return x * x + y * y + z * z + w * w; }
    Vec4 normalize() const { float l = length(); return Vec4(x / l, y / l, z / l, w / l); }
    NoReturn normalize_() { float l = length(); x /= l; y /= l; z /= l; w /= l; return {}; }
    NoReturn copy_(const Vec4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return {}; }
};

struct Mat3x3{    
    union {
        struct {
            float        _11, _12, _13;
            float        _21, _22, _23;
            float        _31, _32, _33;
        };
        float m[3][3];
        float v[9];
    };

    Mat3x3();
    Mat3x3(float, float, float, float, float, float, float, float, float);
    Mat3x3(const Mat3x3& other) = default;

    static Mat3x3 zeros();
    static Mat3x3 ones();
    static Mat3x3 identity();

    Mat3x3 operator+(const Mat3x3& other) const;
    Mat3x3 operator-(const Mat3x3& other) const;
    Mat3x3 operator*(float scalar) const;
    Mat3x3 operator/(float scalar) const;

    bool operator==(const Mat3x3& other) const;
    bool operator!=(const Mat3x3& other) const;
    
    Mat3x3 matmul(const Mat3x3& other) const;
    Vec3 matmul(const Vec3& other) const;

    float determinant() const;
    Mat3x3 transpose() const;
    bool inverse(Mat3x3& out) const;

    /*************** affine transformations ***************/
    static Mat3x3 trs(Vec2 t, float radian, Vec2 s);
    bool is_affine() const;
    Vec2 _t() const;
    float _r() const;
    Vec2 _s() const;
};

struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, linalg, vec2)

    PyVec2() : Vec2() {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) = default;
    Vec2* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyVec3: Vec3 {
    PY_CLASS(PyVec3, linalg, vec3)

    PyVec3() : Vec3() {}
    PyVec3(const Vec3& v) : Vec3(v) {}
    PyVec3(const PyVec3& v) = default;
    Vec3* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyVec4: Vec4{
    PY_CLASS(PyVec4, linalg, vec4)

    PyVec4(): Vec4(){}
    PyVec4(const Vec4& v): Vec4(v){}
    PyVec4(const PyVec4& v) = default;
    Vec4* _(){ return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct PyMat3x3: Mat3x3{
    PY_CLASS(PyMat3x3, linalg, mat3x3)

    PyMat3x3(): Mat3x3(){}
    PyMat3x3(const Mat3x3& other): Mat3x3(other){}
    PyMat3x3(const PyMat3x3& other) = default;
    Mat3x3* _(){ return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

inline PyObject* py_var(VM* vm, Vec2 obj){ return VAR_T(PyVec2, obj); }
inline PyObject* py_var(VM* vm, const PyVec2& obj){ return VAR_T(PyVec2, obj);}

inline PyObject* py_var(VM* vm, Vec3 obj){ return VAR_T(PyVec3, obj); }
inline PyObject* py_var(VM* vm, const PyVec3& obj){ return VAR_T(PyVec3, obj);}

inline PyObject* py_var(VM* vm, Vec4 obj){ return VAR_T(PyVec4, obj); }
inline PyObject* py_var(VM* vm, const PyVec4& obj){ return VAR_T(PyVec4, obj);}

inline PyObject* py_var(VM* vm, const Mat3x3& obj){ return VAR_T(PyMat3x3, obj); }
inline PyObject* py_var(VM* vm, const PyMat3x3& obj){ return VAR_T(PyMat3x3, obj); }

template<> inline Vec2 py_cast<Vec2>(VM* vm, PyObject* obj) { return CAST(PyVec2&, obj); }
template<> inline Vec3 py_cast<Vec3>(VM* vm, PyObject* obj) { return CAST(PyVec3&, obj); }
template<> inline Vec4 py_cast<Vec4>(VM* vm, PyObject* obj) { return CAST(PyVec4&, obj); }
template<> inline Mat3x3 py_cast<Mat3x3>(VM* vm, PyObject* obj) { return CAST(PyMat3x3&, obj); }

template<> inline Vec2 _py_cast<Vec2>(VM* vm, PyObject* obj) { return _CAST(PyVec2&, obj); }
template<> inline Vec3 _py_cast<Vec3>(VM* vm, PyObject* obj) { return _CAST(PyVec3&, obj); }
template<> inline Vec4 _py_cast<Vec4>(VM* vm, PyObject* obj) { return _CAST(PyVec4&, obj); }
template<> inline Mat3x3 _py_cast<Mat3x3>(VM* vm, PyObject* obj) { return _CAST(PyMat3x3&, obj); }

void add_module_linalg(VM* vm);

static_assert(sizeof(Py_<PyMat3x3>) <= 64);
static_assert(std::is_trivially_copyable<PyVec2>::value);
static_assert(std::is_trivially_copyable<PyVec3>::value);
static_assert(std::is_trivially_copyable<PyVec4>::value);
static_assert(std::is_trivially_copyable<PyMat3x3>::value);

}   // namespace pkpy