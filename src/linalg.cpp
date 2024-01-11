#include "pocketpy/linalg.h"

namespace pkpy{

#define BIND_VEC_VEC_OP(D, name, op)                                        \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            PyVec##D& other = CAST(PyVec##D&, args[1]);                     \
            return VAR(self op other);                                      \
        });

#define BIND_VEC_FLOAT_OP(D, name, op)  \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            f64 other = CAST(f64, args[1]);                                 \
            return VAR(self op other);                                      \
        });

#define BIND_VEC_FUNCTION_0(D, name)        \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            return VAR(self.name());                                        \
        });

#define BIND_VEC_FUNCTION_1(D, name)        \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            PyVec##D& other = CAST(PyVec##D&, args[1]);                     \
            return VAR(self.name(other));                                   \
        });

#define BIND_VEC_FIELD(D, name)  \
        vm->bind_property(type, #name,                                      \
        [](VM* vm, ArgsView args){                                          \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            return VAR(self.name);                                          \
        }, [](VM* vm, ArgsView args){                                       \
            PyVec##D& self = _CAST(PyVec##D&, args[0]);                     \
            self.name = CAST(f64, args[1]);                                 \
            return vm->None;                                                \
        });


// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector2.cs#L289
static Vec2 SmoothDamp(Vec2 current, Vec2 target, PyVec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
{
    // Based on Game Programming Gems 4 Chapter 1.10
    smoothTime = std::max(0.0001F, smoothTime);
    float omega = 2.0F / smoothTime;

    float x = omega * deltaTime;
    float exp = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);

    float change_x = current.x - target.x;
    float change_y = current.y - target.y;
    Vec2 originalTo = target;

    // Clamp maximum speed
    float maxChange = maxSpeed * smoothTime;

    float maxChangeSq = maxChange * maxChange;
    float sqDist = change_x * change_x + change_y * change_y;
    if (sqDist > maxChangeSq)
    {
        float mag = std::sqrt(sqDist);
        change_x = change_x / mag * maxChange;
        change_y = change_y / mag * maxChange;
    }

    target.x = current.x - change_x;
    target.y = current.y - change_y;

    float temp_x = (currentVelocity.x + omega * change_x) * deltaTime;
    float temp_y = (currentVelocity.y + omega * change_y) * deltaTime;

    currentVelocity.x = (currentVelocity.x - omega * temp_x) * exp;
    currentVelocity.y = (currentVelocity.y - omega * temp_y) * exp;

    float output_x = target.x + (change_x + temp_x) * exp;
    float output_y = target.y + (change_y + temp_y) * exp;

    // Prevent overshooting
    float origMinusCurrent_x = originalTo.x - current.x;
    float origMinusCurrent_y = originalTo.y - current.y;
    float outMinusOrig_x = output_x - originalTo.x;
    float outMinusOrig_y = output_y - originalTo.y;

    if (origMinusCurrent_x * outMinusOrig_x + origMinusCurrent_y * outMinusOrig_y > 0)
    {
        output_x = originalTo.x;
        output_y = originalTo.y;

        currentVelocity.x = (output_x - originalTo.x) / deltaTime;
        currentVelocity.y = (output_y - originalTo.y) / deltaTime;
    }
    return Vec2(output_x, output_y);
}

    void PyVec2::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyVec2)

        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return VAR(Vec2(x, y));
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            return VAR(Tuple({ VAR(self.x), VAR(self.y) }));
        });

        // @staticmethod
        vm->bind(type, "smooth_damp(current: vec2, target: vec2, current_velocity: vec2, smooth_time: float, max_speed: float, delta_time: float) -> vec2", [](VM* vm, ArgsView args){
            Vec2 current = CAST(Vec2, args[0]);
            Vec2 target = CAST(Vec2, args[1]);
            PyVec2& current_velocity = CAST(PyVec2&, args[2]);
            float smooth_time = CAST_F(args[3]);
            float max_speed = CAST_F(args[4]);
            float delta_time = CAST_F(args[5]);
            Vec2 ret = SmoothDamp(current, target, current_velocity, smooth_time, max_speed, delta_time);
            return VAR(ret);
        });

        // @staticmethod
        vm->bind(type, "angle(__from: vec2, __to: vec2) -> float", [](VM* vm, ArgsView args){
            PyVec2 __from = CAST(PyVec2, args[0]);
            PyVec2 __to = CAST(PyVec2, args[1]);
            float val = atan2f(__to.y, __to.x) - atan2f(__from.y, __from.x);
            const float PI = 3.1415926535897932384f;
            if(val > PI) val -= 2*PI;
            if(val < -PI) val += 2*PI;
            return VAR(val);
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec2& self = _CAST(PyVec2&, obj);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "rotate", [](VM* vm, ArgsView args){
            Vec2 self = _CAST(PyVec2&, args[0]);
            float radian = CAST(f64, args[1]);
            float cr = cosf(radian);
            float sr = sinf(radian);
            Mat3x3 rotate(cr,   -sr,  0.0f,
                          sr,   cr,   0.0f,
                          0.0f, 0.0f, 1.0f);
            self = rotate.transform_vector(self);
            return VAR(self);
        });

        BIND_VEC_VEC_OP(2, __add__, +)
        BIND_VEC_VEC_OP(2, __sub__, -)
        BIND_VEC_FLOAT_OP(2, __mul__, *)
        BIND_VEC_FLOAT_OP(2, __rmul__, *)
        BIND_VEC_FLOAT_OP(2, __truediv__, /)
        BIND_VEC_FIELD(2, x)
        BIND_VEC_FIELD(2, y)
        BIND_VEC_FUNCTION_1(2, dot)
        BIND_VEC_FUNCTION_1(2, cross)
        BIND_VEC_FUNCTION_0(2, length)
        BIND_VEC_FUNCTION_0(2, length_squared)
        BIND_VEC_FUNCTION_0(2, normalize)
    }

    void PyVec3::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyVec3)

        vm->bind_constructor<4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            return VAR(Vec3(x, y, z));
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyVec3& self = _CAST(PyVec3&, args[0]);
            return VAR(Tuple({ VAR(self.x), VAR(self.y), VAR(self.z) }));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec3& self = _CAST(PyVec3&, obj);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << "vec3(" << self.x << ", " << self.y << ", " << self.z << ")";
            return VAR(ss.str());
        });

        BIND_VEC_VEC_OP(3, __add__, +)
        BIND_VEC_VEC_OP(3, __sub__, -)
        BIND_VEC_FLOAT_OP(3, __mul__, *)
        BIND_VEC_FLOAT_OP(3, __rmul__, *)
        BIND_VEC_FLOAT_OP(3, __truediv__, /)
        BIND_VEC_FIELD(3, x)
        BIND_VEC_FIELD(3, y)
        BIND_VEC_FIELD(3, z)
        BIND_VEC_FUNCTION_1(3, dot)
        BIND_VEC_FUNCTION_1(3, cross)
        BIND_VEC_FUNCTION_0(3, length)
        BIND_VEC_FUNCTION_0(3, length_squared)
        BIND_VEC_FUNCTION_0(3, normalize)
    }

    void PyVec4::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyVec4)

        vm->bind_constructor<1+4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            float w = CAST_F(args[4]);
            return VAR(Vec4(x, y, z, w));
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyVec4& self = _CAST(PyVec4&, args[0]);
            return VAR(Tuple({ VAR(self.x), VAR(self.y), VAR(self.z), VAR(self.w) }));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec4& self = _CAST(PyVec4&, obj);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << "vec4(" << self.x << ", " << self.y << ", " << self.z << ", " << self.w << ")";
            return VAR(ss.str());
        });

        BIND_VEC_VEC_OP(4, __add__, +)
        BIND_VEC_VEC_OP(4, __sub__, -)
        BIND_VEC_FLOAT_OP(4, __mul__, *)
        BIND_VEC_FLOAT_OP(4, __rmul__, *)
        BIND_VEC_FLOAT_OP(4, __truediv__, /)
        BIND_VEC_FIELD(4, x)
        BIND_VEC_FIELD(4, y)
        BIND_VEC_FIELD(4, z)
        BIND_VEC_FIELD(4, w)
        BIND_VEC_FUNCTION_1(4, dot)
        BIND_VEC_FUNCTION_0(4, length)
        BIND_VEC_FUNCTION_0(4, length_squared)
        BIND_VEC_FUNCTION_0(4, normalize)
    }

#undef BIND_VEC_ADDR
#undef BIND_VEC_VEC_OP
#undef BIND_VEC_FLOAT_OP
#undef BIND_VEC_FIELD
#undef BIND_VEC_FUNCTION_0
#undef BIND_VEC_FUNCTION_1

    void PyMat3x3::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(PyMat3x3)

        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+0) return VAR_T(PyMat3x3, Mat3x3::zeros());
            if(args.size() == 1+9){
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = CAST_F(args[1+i]);
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
                        mat.m[i][j] = CAST_F(b[j]);
                    }
                }
                return VAR_T(PyMat3x3, mat);
            }
            vm->TypeError(fmt("Mat3x3.__new__ takes 0 or 1 or 9 arguments, got ", args.size()-1));
            return vm->None;
        });

        vm->bind_method<0>(type, "__getnewargs__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            Tuple t(9);
            for(int i=0; i<9; i++) t[i] = VAR(self.v[i]);
            return VAR(std::move(t));
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

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << "mat3x3([[" << self._11 << ", " << self._12 << ", " << self._13 << "],\n";
            ss << "        [" << self._21 << ", " << self._22 << ", " << self._23 << "],\n";
            ss << "        [" << self._31 << ", " << self._32 << ", " << self._33 << "]])";
            return VAR(ss.str());
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            Tuple& t = CAST(Tuple&, index);
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

        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            Tuple& t = CAST(Tuple&, index);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers");
                return;
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
                return;
            }
            self.m[i][j] = CAST_F(value);
        });

#define PROPERTY_FIELD(field) \
        vm->bind_property(type, #field ": float", \
        [](VM* vm, ArgsView args){      \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);                     \
            return VAR(self.field);                                         \
        }, [](VM* vm, ArgsView args){                                       \
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);                     \
            self.field = CAST(f64, args[1]);                                \
            return vm->None;                                                \
        });

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

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            PyMat3x3& other = CAST(PyMat3x3&, _1);
            return VAR_T(PyMat3x3, self + other);
        });

        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            PyMat3x3& other = CAST(PyMat3x3&, _1);
            return VAR_T(PyMat3x3, self - other);
        });

        vm->bind__mul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            f64 other = CAST_F(_1);
            return VAR_T(PyMat3x3, self * other);
        });

        vm->bind_method<1>(type, "__rmul__", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            f64 other = CAST_F(args[1]);
            return VAR_T(PyMat3x3, self * other);
        });

        vm->bind__truediv__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            f64 other = CAST_F(_1);
            return VAR_T(PyMat3x3, self / other);
        });

        vm->bind__matmul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            PyMat3x3& self = _CAST(PyMat3x3&, _0);
            if(is_non_tagged_type(_1, PyMat3x3::_type(vm))){
                PyMat3x3& other = _CAST(PyMat3x3&, _1);
                return VAR_T(PyMat3x3, self.matmul(other));
            }
            if(is_non_tagged_type(_1, PyVec3::_type(vm))){
                PyVec3& other = _CAST(PyVec3&, _1);
                return VAR_T(PyVec3, self.matmul(other));
            }
            return vm->NotImplemented;
        });

        vm->bind_method<0>(type, "determinant", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.determinant());
        });

        vm->bind_method<0>(type, "transpose", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyMat3x3, self.transpose());
        });

        vm->bind__invert__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyMat3x3& self = _CAST(PyMat3x3&, obj);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(PyMat3x3, ret);
        });

        vm->bind_func<0>(type, "zeros", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::zeros());
        });

        vm->bind_func<0>(type, "ones", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::ones());
        });

        vm->bind_func<0>(type, "identity", [](VM* vm, ArgsView args){
            PK_UNUSED(args);
            return VAR_T(PyMat3x3, Mat3x3::identity());
        });

        /*************** affine transformations ***************/
        vm->bind_func<3>(type, "trs", [](VM* vm, ArgsView args){
            PyVec2& t = CAST(PyVec2&, args[0]);
            f64 r = CAST_F(args[1]);
            PyVec2& s = CAST(PyVec2&, args[2]);
            return VAR_T(PyMat3x3, Mat3x3::trs(t, r, s));
        });

        vm->bind_method<0>(type, "is_affine", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self.is_affine());
        });

        vm->bind_method<0>(type, "_t", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self._t());
        });

        vm->bind_method<0>(type, "_r", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR(self._r());
        });

        vm->bind_method<0>(type, "_s", [](VM* vm, ArgsView args){
            PyMat3x3& self = _CAST(PyMat3x3&, args[0]);
            return VAR_T(PyVec2, self._s());
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


void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    PyVec2::register_class(vm, linalg);
    PyVec3::register_class(vm, linalg);
    PyVec4::register_class(vm, linalg);
    PyMat3x3::register_class(vm, linalg);

    PyObject* float_p = vm->_modules["c"]->attr("float_p");
    linalg->attr().set("vec2_p", float_p);
    linalg->attr().set("vec3_p", float_p);
    linalg->attr().set("vec4_p", float_p);
    linalg->attr().set("mat3x3_p", float_p);
}

}   // namespace pkpy