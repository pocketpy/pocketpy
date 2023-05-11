#pragma once

#include <cmath>
#include "cffi.h"

namespace pkpy{

static constexpr float kEpsilon = 1e-5f;
inline static bool isclose(float a, float b){ return fabsf(a - b) < kEpsilon; }

struct Vec2{
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const Vec2& v) : x(v.x), y(v.y) {}

    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    Vec2& operator/=(float s) { x /= s; y /= s; return *this; }
    Vec2 operator-() const { return Vec2(-x, -y); }
    bool operator==(const Vec2& v) const { return isclose(x, v.x) && isclose(y, v.y); }
    bool operator!=(const Vec2& v) const { return !isclose(x, v.x) || !isclose(y, v.y); }
    float dot(const Vec2& v) const { return x * v.x + y * v.y; }
    float cross(const Vec2& v) const { return x * v.y - y * v.x; }
    float length() const { return sqrtf(x * x + y * y); }
    float length_squared() const { return x * x + y * y; }
    void normalize() { float l = length(); x /= l; y /= l; }
    Vec2 normalized() const { float l = length(); return Vec2(x / l, y / l); }
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

    Mat3x3() {}
    Mat3x3(float _11, float _12, float _13,
           float _21, float _22, float _23,
           float _31, float _32, float _33)
        : _11(_11), _12(_12), _13(_13)
        , _21(_21), _22(_22), _23(_23)
        , _31(_31), _32(_32), _33(_33) {}

    void set_zeros(){ for (int i=0; i<9; ++i) v[i] = 0.0f; }
    void set_ones(){ for (int i=0; i<9; ++i) v[i] = 1.0f; }
    void set_identity(){ set_zeros(); _11 = _22 = _33 = 1.0f; }

    static Mat3x3 zeros(){
        static Mat3x3 ret(0, 0, 0, 0, 0, 0, 0, 0, 0);
        return ret;
    }

    static Mat3x3 ones(){
        static Mat3x3 ret(1, 1, 1, 1, 1, 1, 1, 1, 1);
        return ret;
    }

    static Mat3x3 identity(){
        static Mat3x3 ret(1, 0, 0, 0, 1, 0, 0, 0, 1);
        return ret;
    }

    Mat3x3 operator+(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] + other.v[i];
        return ret;
    }

    Mat3x3 operator-(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] - other.v[i];
        return ret;
    }

    Mat3x3 operator*(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] * scalar;
        return ret;
    }

    Mat3x3 operator/(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] / scalar;
        return ret;
    }

    Mat3x3& operator+=(const Mat3x3& other){ 
        for (int i=0; i<9; ++i) v[i] += other.v[i];
        return *this;
    }

    Mat3x3& operator-=(const Mat3x3& other){ 
        for (int i=0; i<9; ++i) v[i] -= other.v[i];
        return *this;
    }

    Mat3x3& operator*=(float scalar){ 
        for (int i=0; i<9; ++i) v[i] *= scalar;
        return *this;
    }

    Mat3x3& operator/=(float scalar){ 
        for (int i=0; i<9; ++i) v[i] /= scalar;
        return *this;
    }

    Mat3x3 matmul(const Mat3x3& other) const{
        Mat3x3 ret;
        ret._11 = _11 * other._11 + _12 * other._21 + _13 * other._31;
        ret._12 = _11 * other._12 + _12 * other._22 + _13 * other._32;
        ret._13 = _11 * other._13 + _12 * other._23 + _13 * other._33;
        ret._21 = _21 * other._11 + _22 * other._21 + _23 * other._31;
        ret._22 = _21 * other._12 + _22 * other._22 + _23 * other._32;
        ret._23 = _21 * other._13 + _22 * other._23 + _23 * other._33;
        ret._31 = _31 * other._11 + _32 * other._21 + _33 * other._31;
        ret._32 = _31 * other._12 + _32 * other._22 + _33 * other._32;
        ret._33 = _31 * other._13 + _32 * other._23 + _33 * other._33;
        return ret;
    }

    bool operator==(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return false;
        }
        return true;
    }

    bool operator!=(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return true;
        }
        return false;
    }

    float determinant() const{
        return _11 * _22 * _33 + _12 * _23 * _31 + _13 * _21 * _32
             - _11 * _23 * _32 - _12 * _21 * _33 - _13 * _22 * _31;
    }

    Mat3x3 transpose() const{
        Mat3x3 ret;
        ret._11 = _11;  ret._12 = _21;  ret._13 = _31;
        ret._21 = _12;  ret._22 = _22;  ret._23 = _32;
        ret._31 = _13;  ret._32 = _23;  ret._33 = _33;
        return ret;
    }

    bool inverse(Mat3x3& ret) const{
        float det = determinant();
        if (fabsf(det) < kEpsilon) return false;
        float inv_det = 1.0f / det;
        ret._11 = (_22 * _33 - _23 * _32) * inv_det;
        ret._12 = (_13 * _32 - _12 * _33) * inv_det;
        ret._13 = (_12 * _23 - _13 * _22) * inv_det;
        ret._21 = (_23 * _31 - _21 * _33) * inv_det;
        ret._22 = (_11 * _33 - _13 * _31) * inv_det;
        ret._23 = (_13 * _21 - _11 * _23) * inv_det;
        ret._31 = (_21 * _32 - _22 * _31) * inv_det;
        ret._32 = (_12 * _31 - _11 * _32) * inv_det;
        ret._33 = (_11 * _22 - _12 * _21) * inv_det;
        return true;
    }

    /*************** affine transformations ***************/
    static Mat3x3 translate(Vec2 v){
        return Mat3x3(1.0f, 0.0f, v.x,
                      0.0f, 1.0f, v.y,
                      0.0f, 0.0f, 1.0f);
    }

    static Mat3x3 rotate(float radian){
        float cr = cosf(radian);
        float sr = sinf(radian);
        return Mat3x3(cr,   -sr,  0.0f,
                      sr,   cr,   0.0f,
                      0.0f, 0.0f, 1.0f);
    }

    static Mat3x3 scale(Vec2 s){
        return Mat3x3(s.x,  0.0f,   0.0f,
                      0.0f, s.y,    0.0f,
                      0.0f, 0.0f,   1.0f);
    }

    static Mat3x3 trs(Vec2 t, float radian, Vec2 s){
        float cr = cosf(radian);
        float sr = sinf(radian);
        return Mat3x3(s.x * cr,   -s.y * sr,  t.x,
                      s.x * sr,   s.y * cr,   t.y,
                      0.0f,       0.0f,       1.0f);
    }

    static Mat3x3 ortho(float left, float right, float bottom, float top){
        return Mat3x3(2.0f / (right - left),  0.0f,                       -(right + left) / (right - left),
                      0.0f,                   2.0f / (top - bottom),      -(top + bottom) / (top - bottom),
                      0.0f,                   0.0f,                       1.0f);
    }

    bool is_affine() const{
        float det = _11 * _22 - _12 * _21;
        if(fabsf(det) < kEpsilon) return false;
        return _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
    }

    Mat3x3 inverse_affine() const{
        Mat3x3 ret;
        float det = _11 * _22 - _12 * _21;
        float inv_det = 1.0f / det;
        ret._11 = _22 * inv_det;
        ret._12 = -_12 * inv_det;
        ret._13 = (_12 * _23 - _13 * _22) * inv_det;
        ret._21 = -_21 * inv_det;
        ret._22 = _11 * inv_det;
        ret._23 = (_13 * _21 - _11 * _23) * inv_det;
        ret._31 = 0.0f;
        ret._32 = 0.0f;
        ret._33 = 1.0f;
        return ret;
    }

    Mat3x3 matmul_affine(const Mat3x3& other) const{
        Mat3x3 ret;
        ret._11 = _11 * other._11 + _12 * other._21;
        ret._12 = _11 * other._12 + _12 * other._22;
        ret._13 = _11 * other._13 + _12 * other._23 + _13;
        ret._21 = _21 * other._11 + _22 * other._21;
        ret._22 = _21 * other._12 + _22 * other._22;
        ret._23 = _21 * other._13 + _22 * other._23 + _23;
        ret._31 = 0.0f;
        ret._32 = 0.0f;
        ret._33 = 1.0f;
        return ret;
    }

    Vec2 translation() const { return Vec2(_13, _23); }
    float rotation() const { return atan2f(_21, _11); }
    Vec2 scale() const {
        return Vec2(
            sqrtf(_11 * _11 + _21 * _21),
            sqrtf(_12 * _12 + _22 * _22)
        );
    }

    Vec2 transform_point(Vec2 v) const {
        return Vec2(_11 * v.x + _12 * v.y + _13, _21 * v.x + _22 * v.y + _23);
    }

    Vec2 transform_vector(Vec2 v) const {
        return Vec2(_11 * v.x + _12 * v.y, _21 * v.x + _22 * v.y);
    }
};

struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, builtins, vec2)

    PyVec2() : Vec2() {}
    PyVec2(float x, float y) : Vec2(x, y) {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) : Vec2(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = vm->num_to_float(args[1]);
            float y = vm->num_to_float(args[2]);
            return VAR_T(PyVec2, x, y);
        });

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            std::stringstream ss;
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR_T(PyVec2, self);
        });

        vm->bind_method<1>(type, "__add__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR_T(PyVec2, self + other);
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR_T(PyVec2, self - other);
        });

        vm->bind_method<1>(type, "__mul__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(PyVec2, self * other);
        });

        vm->bind_method<1>(type, "__truediv__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(PyVec2, self / other);
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR(self == other);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR(self != other);
        });

        type->attr().set("x", vm->property([](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR(self.x);
        }, [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            self.x = vm->num_to_float(args[1]);
            return vm->None;
        }));

        type->attr().set("y", vm->property([](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR(self.y);
        }, [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            self.y = vm->num_to_float(args[1]);
            return vm->None;
        }));

        vm->bind_method<1>(type, "dot", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR(self.dot(other));
        });

        vm->bind_method<1>(type, "cross", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR(self.cross(other));
        });

        vm->bind_method<0>(type, "length", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR(self.length());
        });

        vm->bind_method<0>(type, "length_squared", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR(self.length_squared());
        });

        vm->bind_method<0>(type, "normalize", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            self.normalize();
            return vm->None;
        });

        vm->bind_method<0>(type, "normalized", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR_T(PyVec2, self.normalized());
        });
    }
};

struct PyMat3x3: Mat3x3{
    PY_CLASS(PyMat3x3, builtins, mat3x3)

    PyMat3x3(): Mat3x3(){}
    PyMat3x3(const Mat3x3& other): Mat3x3(other){}
    PyMat3x3(const PyMat3x3& other): Mat3x3(other){}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+0) return VAR_T(PyMat3x3, Mat3x3::zeros());
            if(args.size() == 1+9){
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = vm->num_to_float(args[1+i]);
                return VAR_T(PyMat3x3, mat);
            }
            if(args.size() == 1+1){
                List& a = CAST(List&, args[1]);
                if(a.size() != 3) vm->ValueError("Mat3x3.__new__ takes 3x3 list");
                Mat3x3 mat;
                for(int i=0; i<3; i++){
                    List& b = CAST(List&, a[i]);
                    if(b.size() != 3) vm->ValueError("Mat3x3.__new__ takes 3x3 list");
                    for(int j=0; j<3; j++){
                        mat.m[i][j] = vm->num_to_float(b[j]);
                    }
                }
                return VAR_T(PyMat3x3, mat);
            }
            vm->TypeError("Mat3x3.__new__ takes 0 or 1 arguments");
            return vm->None;
        });

#define METHOD_PROXY_NONE(name)  \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){    \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);               \
            self.name();                                              \
            return vm->None;                                          \
        });

        METHOD_PROXY_NONE(set_zeros)
        METHOD_PROXY_NONE(set_ones)
        METHOD_PROXY_NONE(set_identity)

#undef METHOD_PROXY_NONE

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(4);
            ss << "mat3x3([[" << self._11 << ", " << self._12 << ", " << self._13 << "],\n";
            ss << "        [" << self._21 << ", " << self._22 << ", " << self._23 << "],\n";
            ss << "        [" << self._31 << ", " << self._32 << ", " << self._33 << "]])";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self);
        });

        vm->bind_method<1>(type, "__getitem__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Tuple& t = CAST(Tuple&, args[1]);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__getitem__ takes a tuple of 2 integers");
                return vm->None;
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
                return vm->None;
            }
            return VAR(self.m[i][j]);
        });

        vm->bind_method<2>(type, "__setitem__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Tuple& t = CAST(Tuple&, args[1]);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers");
                return vm->None;
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
                return vm->None;
            }
            self.m[i][j] = vm->num_to_float(args[2]);
            return vm->None;
        });

#define PROPERTY_FIELD(field) \
        type->attr().set(#field, vm->property([](VM* vm, ArgsView args){    \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);                     \
            return VAR(self.field);                                         \
        }, [](VM* vm, ArgsView args){                                       \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);                     \
            self.field = vm->num_to_float(args[1]);                         \
            return vm->None;                                                \
        }));

        PROPERTY_FIELD(_11)
        PROPERTY_FIELD(_12)
        PROPERTY_FIELD(_13)
        PROPERTY_FIELD(_21)
        PROPERTY_FIELD(_22)
        PROPERTY_FIELD(_23)
        PROPERTY_FIELD(_31)
        PROPERTY_FIELD(_32)
        PROPERTY_FIELD(_33)

#undef PROPERTY_FIELD

        vm->bind_method<1>(type, "__add__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self + other);
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self - other);
        });

        vm->bind_method<1>(type, "__mul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(PyMat3x3, self * other);
        });

        vm->bind_method<1>(type, "__truediv__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(PyMat3x3, self / other);
        });

        vm->bind_method<1>(type, "__matmul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self.matmul(other));
        });

        vm->bind_method<1>(type, "matmul", [](VM* vm, ArgsView args){
            PyMat3x3& other = CAST(PyMat3x3&, args[0]);
            PyMat3x3& self = _CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self.matmul(other));
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = _CAST(PyMat3x3&, args[1]);
            return VAR(self == other);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR(self != other);
        });

        vm->bind_method<0>(type, "determinant", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.determinant());
        });

        vm->bind_method<0>(type, "transpose", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self.transpose());
        });

        vm->bind_method<0>(type, "inverse", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(PyMat3x3, ret);
        });

        vm->bind_func<0>(type, "zeros", [](VM* vm, ArgsView args){
            return VAR_T(PyMat3x3, Mat3x3::zeros());
        });

        vm->bind_func<0>(type, "ones", [](VM* vm, ArgsView args){
            return VAR_T(PyMat3x3, Mat3x3::ones());
        });

        vm->bind_func<0>(type, "identity", [](VM* vm, ArgsView args){
            return VAR_T(PyMat3x3, Mat3x3::identity());
        });

        /*************** affine transformations ***************/
        vm->bind_func<1>(type, "translate", [](VM* vm, ArgsView args){
            PyVec2& v = CAST(PyVec2&, args[0]);
            return VAR_T(PyMat3x3, Mat3x3::translate(v));
        });

        vm->bind_func<1>(type, "rotate", [](VM* vm, ArgsView args){
            f64 angle = vm->num_to_float(args[0]);
            return VAR_T(PyMat3x3, Mat3x3::rotate(angle));
        });

        vm->bind_func<1>(type, "scale", [](VM* vm, ArgsView args){
            PyVec2& v = CAST(PyVec2&, args[0]);
            return VAR_T(PyMat3x3, Mat3x3::scale(v));
        });

        vm->bind_func<3>(type, "trs", [](VM* vm, ArgsView args){
            PyVec2& t = CAST(PyVec2&, args[0]);
            f64 r = vm->num_to_float(args[1]);
            PyVec2& s = CAST(PyVec2&, args[2]);
            return VAR_T(PyMat3x3, Mat3x3::trs(t, r, s));
        });

        vm->bind_func<4>(type, "ortho", [](VM* vm, ArgsView args){
            f64 left = vm->num_to_float(args[0]);
            f64 right = vm->num_to_float(args[1]);
            f64 bottom = vm->num_to_float(args[2]);
            f64 top = vm->num_to_float(args[3]);
            return VAR_T(PyMat3x3, Mat3x3::ortho(left, right, bottom, top));
        });

        vm->bind_method<0>(type, "is_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.is_affine());
        });

        vm->bind_method<0>(type, "inverse_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self.inverse_affine());
        });

        vm->bind_method<1>(type, "matmul_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self.matmul_affine(other));
        });


        vm->bind_method<0>(type, "translation", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self.translation());
        });

        vm->bind_method<0>(type, "rotation", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.rotation());
        });

        vm->bind_method<0>(type, "scale", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self.scale());
        });

        vm->bind_method<1>(type, "transform_point", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyVec2& v = CAST(PyVec2&, args[1]);
            return VAR_T(PyVec2, self.transform_point(v));
        });

        vm->bind_method<1>(type, "transform_vector", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyVec2& v = CAST(PyVec2&, args[1]);
            return VAR_T(PyVec2, self.transform_vector(v));
        });
    }
};

}   // namespace pkpy