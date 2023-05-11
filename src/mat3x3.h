#pragma once

#include <cmath>
#include "cffi.h"

namespace pkpy{

struct Mat3x3{
    static constexpr float kEpsilon = 1e-5f;
    union {
        struct {
            float        _11, _12, _13;
            float        _21, _22, _23;
            float        _31, _32, _33;
        };
        float m[3][3];
        float v[9];
    };

    void set_zeros(){
        for (int i=0; i<9; ++i) v[i] = 0.0f;
    }

    void set_ones(){
        for (int i=0; i<9; ++i) v[i] = 1.0f;
    }

    void set_identity(){
        _11 = 1.0f;  _12 = 0.0f;  _13 = 0.0f;
        _21 = 0.0f;  _22 = 1.0f;  _23 = 0.0f;
        _31 = 0.0f;  _32 = 0.0f;  _33 = 1.0f;
    }

    void add_(const Mat3x3& other){ for (int i=0; i<9; ++i) v[i] += other.v[i]; }
    void sub_(const Mat3x3& other){ for (int i=0; i<9; ++i) v[i] -= other.v[i]; }
    void mul_(float s){ for (int i=0; i<9; ++i) v[i] *= s; }
    void div_(float s){ for (int i=0; i<9; ++i) v[i] /= s; }

    Mat3x3 add(const Mat3x3& other) const{ Mat3x3 ret(*this); ret.add_(other); return ret; }
    Mat3x3 sub(const Mat3x3& other) const{ Mat3x3 ret(*this); ret.sub_(other); return ret; }
    Mat3x3 mul(float s) const{ Mat3x3 ret(*this); ret.mul_(s); return ret; }
    Mat3x3 div(float s) const{ Mat3x3 ret(*this); ret.div_(s); return ret; }

    void matmul(const Mat3x3& other, Mat3x3& ret) const{
        ret._11 = _11 * other._11 + _12 * other._21 + _13 * other._31;
        ret._12 = _11 * other._12 + _12 * other._22 + _13 * other._32;
        ret._13 = _11 * other._13 + _12 * other._23 + _13 * other._33;
        ret._21 = _21 * other._11 + _22 * other._21 + _23 * other._31;
        ret._22 = _21 * other._12 + _22 * other._22 + _23 * other._32;
        ret._23 = _21 * other._13 + _22 * other._23 + _23 * other._33;
        ret._31 = _31 * other._11 + _32 * other._21 + _33 * other._31;
        ret._32 = _31 * other._12 + _32 * other._22 + _33 * other._32;
        ret._33 = _31 * other._13 + _32 * other._23 + _33 * other._33;
    }

    Mat3x3 matmul(const Mat3x3& other) const{
        Mat3x3 ret;
        matmul(other, ret);
        return ret;
    }

    bool operator==(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (v[i] != other.v[i]) return false;
        }
        return true;
    }

    bool operator!=(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (v[i] != other.v[i]) return true;
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

    bool is_identity() const{
        return _11 == 1.0f && _12 == 0.0f && _13 == 0.0f
            && _21 == 0.0f && _22 == 1.0f && _23 == 0.0f
            && _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
    }

    /*************** affine transform (no bindings) ***************/
    bool is_affine() const{
        float det = _11 * _22 - _12 * _21;
        if(fabsf(det) < kEpsilon) return false;
        return _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
    }

    void set_translate(float x, float y){
        _11 = 1.0f;  _12 = 0.0f;  _13 = x;
        _21 = 0.0f;  _22 = 1.0f;  _23 = y;
        _31 = 0.0f;  _32 = 0.0f;  _33 = 1.0f;
    }

    void set_rotate(float radian){
        float c = cosf(radian);
        float s = sinf(radian);
        _11 = c;    _12 = -s;   _13 = 0.0f;
        _21 = s;    _22 = c;    _23 = 0.0f;
        _31 = 0.0f; _32 = 0.0f; _33 = 1.0f;
    }

    void set_scale(float sx, float sy){
        _11 = sx;    _12 = 0.0f;  _13 = 0.0f;
        _21 = 0.0f;  _22 = sy;    _23 = 0.0f;
        _31 = 0.0f;  _32 = 0.0f;  _33 = 1.0f;
    }

    void set_trs(float x, float y, float radian, float sx, float sy){
        float c = cosf(radian);
        float s = sinf(radian);
        _11 = sx * c;   _12 = -sy * s;  _13 = x;
        _21 = sx * s;   _22 = sy * c;   _23 = y;
        _31 = 0.0f;     _32 = 0.0f;     _33 = 1.0f;
    }

    void inverse_affine(Mat3x3& ret) const{
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
    }

    void matmul_affine(const Mat3x3& other, Mat3x3& ret) const{
        ret._11 = _11 * other._11 + _12 * other._21;
        ret._12 = _11 * other._12 + _12 * other._22;
        ret._13 = _11 * other._13 + _12 * other._23 + _13;
        ret._21 = _21 * other._11 + _22 * other._21;
        ret._22 = _21 * other._12 + _22 * other._22;
        ret._23 = _21 * other._13 + _22 * other._23 + _23;
        ret._31 = 0.0f;
        ret._32 = 0.0f;
        ret._33 = 1.0f;
    }

    Mat3x3 matmul_affine(const Mat3x3& other) const{
        Mat3x3 ret;
        matmul_affine(other, ret);
        return ret;
    }

    float x() const { return _13; }
    float y() const { return _23; }
    float rotation() const { return atan2f(_21, _11); }
    float scale_x() const { return sqrtf(_11 * _11 + _21 * _21); }
    float scale_y() const { return sqrtf(_12 * _12 + _22 * _22); }

    void transform_point(float& x, float& y) const {
        float tx = x;
        float ty = y;
        x = _11 * tx + _12 * ty + _13;
        y = _21 * tx + _22 * ty + _23;
    }

    void transform_vector(float& x, float& y) const {
        float tx = x;
        float ty = y;
        x = _11 * tx + _12 * ty;
        y = _21 * tx + _22 * ty;
    }

    void set_ortho(float left, float right, float bottom, float top){
        _11 = 2.0f / (right - left);    _12 = 0.0f;                      _13 = -(right + left) / (right - left);
        _21 = 0.0f;                     _22 = 2.0f / (top - bottom);     _23 = -(top + bottom) / (top - bottom);
        _31 = 0.0f;                     _32 = 0.0f;                      _33 = 1.0f;
    }
};


struct PyMat3x3: Mat3x3{
    PY_CLASS(PyMat3x3, builtins, mat3x3)

    PyMat3x3(): Mat3x3(){}
    PyMat3x3(const Mat3x3& other): Mat3x3(other){}
    PyMat3x3(const PyMat3x3& other): Mat3x3(other){}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+0) return VAR_T(PyMat3x3);
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
            return VAR_T(PyMat3x3, self.add(other));
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
            return VAR_T(PyMat3x3, self.sub(other));
        });

        vm->bind_method<1>(type, "__mul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(PyMat3x3, self.mul(other));
        });

        vm->bind_method<1>(type, "__truediv__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(PyMat3x3, self.div(other));
        });

        vm->bind_method<1>(type, "__matmul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            PyMat3x3& other = CAST(PyMat3x3&, args[1]);
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

        vm->bind_method<0>(type, "deteminant", [](VM* vm, ArgsView args){
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

        vm->bind_method<0>(type, "is_identity", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.is_identity());
        });
    }
};

}   // namespace pkpy