#include "pocketpy/linalg.h"

namespace pkpy{

#define BIND_VEC_VEC_OP(D, name, op)                                                    \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            Vec##D& self = _CAST(Vec##D&, _0);                                      \
            Vec##D& other = CAST(Vec##D&, _1);                                      \
            return VAR(self op other);                                                  \
        });

#define BIND_VEC_FLOAT_OP(D, name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            Vec##D& self = _CAST(Vec##D&, _0);                                      \
            f64 other = CAST(f64, _1);                                                  \
            return VAR(self op other);                                                  \
        });

#define BIND_VEC_FUNCTION_0(D, name)        \
        vm->bind_method<0>(type, #name, [](VM* vm, ArgsView args){          \
            Vec##D& self = _CAST(Vec##D&, args[0]);                     \
            return VAR(self.name());                                        \
        });

#define BIND_VEC_FUNCTION_1(D, name)        \
        vm->bind_method<1>(type, #name, [](VM* vm, ArgsView args){          \
            Vec##D& self = _CAST(Vec##D&, args[0]);                     \
            Vec##D& other = CAST(Vec##D&, args[1]);                     \
            return VAR(self.name(other));                                   \
        });

#define BIND_VEC_MUL_OP(D)                                                                \
        vm->bind__mul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){     \
            Vec##D& self = _CAST(Vec##D&, _0);                                          \
            if(is_type(_1, Vec##D::_type(vm))){                                \
                Vec##D& other = _CAST(Vec##D&, _1);                                     \
                return VAR(self * other);                                                   \
            }                                                                               \
            f64 other = CAST(f64, _1);                                                      \
            return VAR(self * other);                                                       \
        });                                                                                 \
        vm->bind_method<1>(type, "__rmul__", [](VM* vm, ArgsView args){                     \
            Vec##D& self = _CAST(Vec##D&, args[0]);                                     \
            f64 other = CAST(f64, args[1]);                                                 \
            return VAR(self * other);                                                       \
        });                                                                                 \
        vm->bind__truediv__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){ \
            Vec##D& self = _CAST(Vec##D&, _0);                                          \
            f64 other = CAST(f64, _1);                                                      \
            return VAR(self / other);                                                       \
        });

#define BIND_VEC_GETITEM(D) \
        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index){ \
            Vec##D& self = _CAST(Vec##D&, obj); \
            i64 i = CAST(i64, index); \
            if(i < 0 || i >= D) vm->IndexError("index out of range"); \
            float* v = &self.x; \
            return VAR(v[i]); \
        });

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector2.cs#L289
static Vec2 SmoothDamp(Vec2 current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
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

    void Vec2::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(Vec2)

        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return vm->heap.gcnew<Vec2>(PK_OBJ_GET(Type, args[0]), Vec2(x, y));
        });

        // @staticmethod
        vm->bind(type, "smooth_damp(current: vec2, target: vec2, current_velocity_: vec2, smooth_time: float, max_speed: float, delta_time: float) -> vec2", [](VM* vm, ArgsView args){
            Vec2 current = CAST(Vec2, args[0]);
            Vec2 target = CAST(Vec2, args[1]);
            Vec2& current_velocity_ = CAST(Vec2&, args[2]);
            float smooth_time = CAST_F(args[3]);
            float max_speed = CAST_F(args[4]);
            float delta_time = CAST_F(args[5]);
            Vec2 ret = SmoothDamp(current, target, current_velocity_, smooth_time, max_speed, delta_time);
            return VAR(ret);
        }, {}, BindType::STATICMETHOD);

        // @staticmethod
        vm->bind(type, "angle(__from: vec2, __to: vec2) -> float", [](VM* vm, ArgsView args){
            Vec2 __from = CAST(Vec2, args[0]);
            Vec2 __to = CAST(Vec2, args[1]);
            float val = atan2f(__to.y, __to.x) - atan2f(__from.y, __from.x);
            const float PI = 3.1415926535897932384f;
            if(val > PI) val -= 2*PI;
            if(val < -PI) val += 2*PI;
            return VAR(val);
        }, {}, BindType::STATICMETHOD);

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Vec2 self = _CAST(Vec2&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "rotate", [](VM* vm, ArgsView args){
            Vec2 self = _CAST(Vec2&, args[0]);
            float radian = CAST(f64, args[1]);
            return VAR_T(Vec2, self.rotate(radian));
        });

        vm->bind_method<1>(type, "rotate_", [](VM* vm, ArgsView args){
            Vec2& self = _CAST(Vec2&, args[0]);
            float radian = CAST(f64, args[1]);
            self = self.rotate(radian);
            return vm->None;
        });

        PY_FIELD(Vec2, "x", _, x)
        PY_FIELD(Vec2, "y", _, y)

        BIND_VEC_VEC_OP(2, __add__, +)
        BIND_VEC_VEC_OP(2, __sub__, -)
        BIND_VEC_MUL_OP(2)
        BIND_VEC_FLOAT_OP(2, __truediv__, /)
        BIND_VEC_FUNCTION_1(2, dot)
        BIND_VEC_FUNCTION_1(2, cross)
        BIND_VEC_FUNCTION_1(2, copy_)
        BIND_VEC_FUNCTION_0(2, length)
        BIND_VEC_FUNCTION_0(2, length_squared)
        BIND_VEC_FUNCTION_0(2, normalize)
        BIND_VEC_FUNCTION_0(2, normalize_)
        BIND_VEC_GETITEM(2)
    }

    void Vec3::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(Vec3)

        vm->bind_constructor<4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            return vm->heap.gcnew<Vec3>(PK_OBJ_GET(Type, args[0]), Vec3(x, y, z));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Vec3 self = _CAST(Vec3&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "vec3(" << self.x << ", " << self.y << ", " << self.z << ")";
            return VAR(ss.str());
        });

        PY_FIELD(Vec3, "x", _, x)
        PY_FIELD(Vec3, "y", _, y)
        PY_FIELD(Vec3, "z", _, z)

        BIND_VEC_VEC_OP(3, __add__, +)
        BIND_VEC_VEC_OP(3, __sub__, -)
        BIND_VEC_MUL_OP(3)
        BIND_VEC_FUNCTION_1(3, dot)
        BIND_VEC_FUNCTION_1(3, cross)
        BIND_VEC_FUNCTION_1(3, copy_)
        BIND_VEC_FUNCTION_0(3, length)
        BIND_VEC_FUNCTION_0(3, length_squared)
        BIND_VEC_FUNCTION_0(3, normalize)
        BIND_VEC_FUNCTION_0(3, normalize_)
        BIND_VEC_GETITEM(3)
    }

    void Vec4::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(Vec4)

        vm->bind_constructor<1+4>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            float z = CAST_F(args[3]);
            float w = CAST_F(args[4]);
            return vm->heap.gcnew<Vec4>(PK_OBJ_GET(Type, args[0]), Vec4(x, y, z, w));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Vec4 self = _CAST(Vec4&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "vec4(" << self.x << ", " << self.y << ", " << self.z << ", " << self.w << ")";
            return VAR(ss.str());
        });

        PY_FIELD(Vec4, "x", _, x)
        PY_FIELD(Vec4, "y", _, y)
        PY_FIELD(Vec4, "z", _, z)
        PY_FIELD(Vec4, "w", _, w)

        BIND_VEC_VEC_OP(4, __add__, +)
        BIND_VEC_VEC_OP(4, __sub__, -)
        BIND_VEC_MUL_OP(4)
        BIND_VEC_FUNCTION_1(4, dot)
        BIND_VEC_FUNCTION_1(4, copy_)
        BIND_VEC_FUNCTION_0(4, length)
        BIND_VEC_FUNCTION_0(4, length_squared)
        BIND_VEC_FUNCTION_0(4, normalize)
        BIND_VEC_FUNCTION_0(4, normalize_)
        BIND_VEC_GETITEM(4)
    }

#undef BIND_VEC_VEC_OP
#undef BIND_VEC_MUL_OP
#undef BIND_VEC_FUNCTION_0
#undef BIND_VEC_FUNCTION_1
#undef BIND_VEC_GETITEM

    void Mat3x3::_register(VM* vm, PyObject* mod, PyObject* type){
        PY_STRUCT_LIKE(Mat3x3)

        vm->bind_func<-1>(type, __new__, [](VM* vm, ArgsView args){
            if(args.size() == 1+0) return vm->heap.gcnew<Mat3x3>(PK_OBJ_GET(Type, args[0]), Mat3x3::zeros());
            if(args.size() == 1+1){
                const List& list = CAST(List&, args[1]);
                if(list.size() != 9) vm->TypeError("Mat3x3.__new__ takes a list of 9 floats");
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = CAST_F(list[i]);
                return vm->heap.gcnew<Mat3x3>(PK_OBJ_GET(Type, args[0]), mat);
            }
            if(args.size() == 1+9){
                Mat3x3 mat;
                for(int i=0; i<9; i++) mat.v[i] = CAST_F(args[1+i]);
                return vm->heap.gcnew<Mat3x3>(PK_OBJ_GET(Type, args[0]), mat);
            }
            vm->TypeError(_S("Mat3x3.__new__ takes 0 or 1 or 9 arguments, got ", args.size()-1));
            return vm->None;
        });

        vm->bind_method<1>(type, "copy_", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            const Mat3x3& other = CAST(Mat3x3&, args[1]);
            self = other;
            return vm->None;
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            const Mat3x3& self = _CAST(Mat3x3&, obj);
            SStream ss;
            ss.setprecision(3);
            ss << "mat3x3([" << self._11 << ", " << self._12 << ", " << self._13 << ",\n";
            ss << "        " << self._21 << ", " << self._22 << ", " << self._23 << ",\n";
            ss << "        " << self._31 << ", " << self._32 << ", " << self._33 << "])";
            return VAR(ss.str());
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index){
            Mat3x3& self = _CAST(Mat3x3&, obj);
            Tuple& t = CAST(Tuple&, index);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__getitem__ takes a tuple of 2 integers");
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
            }
            return VAR(self.m[i][j]);
        });

        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){
            Mat3x3& self = _CAST(Mat3x3&, obj);
            const Tuple& t = CAST(Tuple&, index);
            if(t.size() != 2){
                vm->TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers");
            }
            i64 i = CAST(i64, t[0]);
            i64 j = CAST(i64, t[1]);
            if(i < 0 || i >= 3 || j < 0 || j >= 3){
                vm->IndexError("index out of range");
            }
            self.m[i][j] = CAST_F(value);
        });

        PY_FIELD(Mat3x3, "_11", _, _11)
        PY_FIELD(Mat3x3, "_12", _, _12)
        PY_FIELD(Mat3x3, "_13", _, _13)
        PY_FIELD(Mat3x3, "_21", _, _21)
        PY_FIELD(Mat3x3, "_22", _, _22)
        PY_FIELD(Mat3x3, "_23", _, _23)
        PY_FIELD(Mat3x3, "_31", _, _31)
        PY_FIELD(Mat3x3, "_32", _, _32)
        PY_FIELD(Mat3x3, "_33", _, _33)

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Mat3x3& self = _CAST(Mat3x3&, _0);
            Mat3x3& other = CAST(Mat3x3&, _1);
            return VAR_T(Mat3x3, self + other);
        });

        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Mat3x3& self = _CAST(Mat3x3&, _0);
            Mat3x3& other = CAST(Mat3x3&, _1);
            return VAR_T(Mat3x3, self - other);
        });

        vm->bind__mul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Mat3x3& self = _CAST(Mat3x3&, _0);
            f64 other = CAST_F(_1);
            return VAR_T(Mat3x3, self * other);
        });

        vm->bind_method<1>(type, "__rmul__", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            f64 other = CAST_F(args[1]);
            return VAR_T(Mat3x3, self * other);
        });

        vm->bind__truediv__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Mat3x3& self = _CAST(Mat3x3&, _0);
            f64 other = CAST_F(_1);
            return VAR_T(Mat3x3, self / other);
        });

        vm->bind__matmul__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Mat3x3& self = _CAST(Mat3x3&, _0);
            if(is_type(_1, Mat3x3::_type(vm))){
                const Mat3x3& other = _CAST(Mat3x3&, _1);
                return VAR_T(Mat3x3, self.matmul(other));
            }
            if(is_type(_1, Vec3::_type(vm))){
                const Vec3& other = _CAST(Vec3&, _1);
                return VAR_T(Vec3, self.matmul(other));
            }
            return vm->NotImplemented;
        });

        vm->bind(type, "matmul(self, other: mat3x3, out: mat3x3 = None)", [](VM* vm, ArgsView args){
            const Mat3x3& self = _CAST(Mat3x3&, args[0]);
            const Mat3x3& other = CAST(Mat3x3&, args[1]);
            if(args[2] == vm->None){
                return VAR_T(Mat3x3, self.matmul(other));
            }else{
                Mat3x3& out = CAST(Mat3x3&, args[2]);
                out = self.matmul(other);
                return vm->None;
            }
        });

        vm->bind_method<0>(type, "determinant", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            return VAR(self.determinant());
        });

        vm->bind_method<0>(type, "transpose", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            return VAR_T(Mat3x3, self.transpose());
        });

        vm->bind__invert__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            Mat3x3& self = _CAST(Mat3x3&, obj);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(Mat3x3, ret);
        });

        vm->bind_method<0>(type, "invert", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            return VAR_T(Mat3x3, ret);
        });

        vm->bind_method<0>(type, "invert_", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Mat3x3 ret;
            bool ok = self.inverse(ret);
            if(!ok) vm->ValueError("matrix is not invertible");
            self = ret;
            return vm->None;
        });

        vm->bind_method<0>(type, "transpose_", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            self = self.transpose();
            return vm->None;
        });

        // @staticmethod
        vm->bind(type, "zeros()", [](VM* vm, ArgsView args){
            return VAR_T(Mat3x3, Mat3x3::zeros());
        }, {}, BindType::STATICMETHOD);

        // @staticmethod
        vm->bind(type, "ones()", [](VM* vm, ArgsView args){
            return VAR_T(Mat3x3, Mat3x3::ones());
        }, {}, BindType::STATICMETHOD);

        // @staticmethod
        vm->bind(type, "identity()", [](VM* vm, ArgsView args){
            return VAR_T(Mat3x3, Mat3x3::identity());
        }, {}, BindType::STATICMETHOD);

        /*************** affine transformations ***************/
        // @staticmethod
        vm->bind(type, "trs(t: vec2, r: float, s: vec2)", [](VM* vm, ArgsView args){
            Vec2 t = CAST(Vec2, args[0]);
            f64 r = CAST_F(args[1]);
            Vec2 s = CAST(Vec2, args[2]);
            return VAR_T(Mat3x3, Mat3x3::trs(t, r, s));
        }, {}, BindType::STATICMETHOD);

        vm->bind(type, "copy_trs_(self, t: vec2, r: float, s: vec2)", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Vec2 t = CAST(Vec2, args[1]);
            f64 r = CAST_F(args[2]);
            Vec2 s = CAST(Vec2, args[3]);
            self = Mat3x3::trs(t, r, s);
            return vm->None;
        });

        vm->bind(type, "copy_t_(self, t: vec2)", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Vec2 t = CAST(Vec2, args[1]);
            self = Mat3x3::trs(t, self._r(), self._s());
            return vm->None;
        });

        vm->bind(type, "copy_r_(self, r: float)", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            f64 r = CAST_F(args[1]);
            self = Mat3x3::trs(self._t(), r, self._s());
            return vm->None;
        });

        vm->bind(type, "copy_s_(self, s: vec2)", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Vec2 s = CAST(Vec2, args[1]);
            self = Mat3x3::trs(self._t(), self._r(), s);
            return vm->None;
        });

        vm->bind_method<0>(type, "is_affine", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            return VAR(self.is_affine());
        });

        vm->bind_method<0>(type, "_t", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            return VAR_T(Vec2, self._t());
        });

        vm->bind_method<0>(type, "_r", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            return VAR(self._r());
        });

        vm->bind_method<0>(type, "_s", [](VM* vm, ArgsView args){
            Mat3x3& self = _CAST(Mat3x3&, args[0]);
            return VAR_T(Vec2, self._s());
        });

        vm->bind_method<1>(type, "transform_point", [](VM* vm, ArgsView args){
            const Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Vec2 v = CAST(Vec2, args[1]);
            Vec2 res = Vec2(self._11 * v.x + self._12 * v.y + self._13, self._21 * v.x + self._22 * v.y + self._23);
            return VAR_T(Vec2, res);
        });

        vm->bind_method<1>(type, "transform_vector", [](VM* vm, ArgsView args){
            const Mat3x3& self = _CAST(Mat3x3&, args[0]);
            Vec2 v = CAST(Vec2, args[1]);
            Vec2 res = Vec2(self._11 * v.x + self._12 * v.y, self._21 * v.x + self._22 * v.y);
            return VAR_T(Vec2, res);
        });
    }


void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    Vec2::register_class(vm, linalg);
    Vec3::register_class(vm, linalg);
    Vec4::register_class(vm, linalg);
    Mat3x3::register_class(vm, linalg);

    PyObject* float_p = vm->_modules["c"]->attr("float_p");
    linalg->attr().set("vec2_p", float_p);
    linalg->attr().set("vec3_p", float_p);
    linalg->attr().set("vec4_p", float_p);
    linalg->attr().set("mat3x3_p", float_p);
}


    /////////////// mat3x3 ///////////////
    Mat3x3::Mat3x3() {}
    Mat3x3::Mat3x3(float _11, float _12, float _13,
           float _21, float _22, float _23,
           float _31, float _32, float _33)
        : _11(_11), _12(_12), _13(_13)
        , _21(_21), _22(_22), _23(_23)
        , _31(_31), _32(_32), _33(_33) {}

    Mat3x3 Mat3x3::zeros(){
        return Mat3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    Mat3x3 Mat3x3::ones(){
        return Mat3x3(1, 1, 1, 1, 1, 1, 1, 1, 1);
    }

    Mat3x3 Mat3x3::identity(){
        return Mat3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    }

    Mat3x3 Mat3x3::operator+(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] + other.v[i];
        return ret;
    }

    Mat3x3 Mat3x3::operator-(const Mat3x3& other) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] - other.v[i];
        return ret;
    }

    Mat3x3 Mat3x3::operator*(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] * scalar;
        return ret;
    }

    Mat3x3 Mat3x3::operator/(float scalar) const{ 
        Mat3x3 ret;
        for (int i=0; i<9; ++i) ret.v[i] = v[i] / scalar;
        return ret;
    }

    bool Mat3x3::operator==(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return false;
        }
        return true;
    }

    bool Mat3x3::operator!=(const Mat3x3& other) const{
        for (int i=0; i<9; ++i){
            if (!isclose(v[i], other.v[i])) return true;
        }
        return false;
    }

    Mat3x3 Mat3x3::matmul(const Mat3x3& other) const{
        Mat3x3 out;
        out._11 = _11 * other._11 + _12 * other._21 + _13 * other._31;
        out._12 = _11 * other._12 + _12 * other._22 + _13 * other._32;
        out._13 = _11 * other._13 + _12 * other._23 + _13 * other._33;
        out._21 = _21 * other._11 + _22 * other._21 + _23 * other._31;
        out._22 = _21 * other._12 + _22 * other._22 + _23 * other._32;
        out._23 = _21 * other._13 + _22 * other._23 + _23 * other._33;
        out._31 = _31 * other._11 + _32 * other._21 + _33 * other._31;
        out._32 = _31 * other._12 + _32 * other._22 + _33 * other._32;
        out._33 = _31 * other._13 + _32 * other._23 + _33 * other._33;
        return out;
    }

    Vec3 Mat3x3::matmul(const Vec3& other) const{
        Vec3 out;
        out.x = _11 * other.x + _12 * other.y + _13 * other.z;
        out.y = _21 * other.x + _22 * other.y + _23 * other.z;
        out.z = _31 * other.x + _32 * other.y + _33 * other.z;
        return out;
    }

    float Mat3x3::determinant() const{
        return _11 * _22 * _33 + _12 * _23 * _31 + _13 * _21 * _32
             - _11 * _23 * _32 - _12 * _21 * _33 - _13 * _22 * _31;
    }

    Mat3x3 Mat3x3::transpose() const{
        Mat3x3 ret;
        ret._11 = _11;  ret._12 = _21;  ret._13 = _31;
        ret._21 = _12;  ret._22 = _22;  ret._23 = _32;
        ret._31 = _13;  ret._32 = _23;  ret._33 = _33;
        return ret;
    }

    bool Mat3x3::inverse(Mat3x3& out) const{
        float det = determinant();
        if (isclose(det, 0)) return false;
        float inv_det = 1.0f / det;
        out._11 = (_22 * _33 - _23 * _32) * inv_det;
        out._12 = (_13 * _32 - _12 * _33) * inv_det;
        out._13 = (_12 * _23 - _13 * _22) * inv_det;
        out._21 = (_23 * _31 - _21 * _33) * inv_det;
        out._22 = (_11 * _33 - _13 * _31) * inv_det;
        out._23 = (_13 * _21 - _11 * _23) * inv_det;
        out._31 = (_21 * _32 - _22 * _31) * inv_det;
        out._32 = (_12 * _31 - _11 * _32) * inv_det;
        out._33 = (_11 * _22 - _12 * _21) * inv_det;
        return true;
    }

    Mat3x3 Mat3x3::trs(Vec2 t, float radian, Vec2 s){
        float cr = cosf(radian);
        float sr = sinf(radian);
        return Mat3x3(s.x * cr,   -s.y * sr,  t.x,
                      s.x * sr,   s.y * cr,   t.y,
                      0.0f,       0.0f,       1.0f);
    }

    bool Mat3x3::is_affine() const{
        float det = _11 * _22 - _12 * _21;
        if(isclose(det, 0)) return false;
        return _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
    }

    Vec2 Mat3x3::_t() const { return Vec2(_13, _23); }
    float Mat3x3::_r() const { return atan2f(_21, _11); }
    Vec2 Mat3x3::_s() const {
        return Vec2(
            sqrtf(_11 * _11 + _21 * _21),
            sqrtf(_12 * _12 + _22 * _22)
        );
    }

}   // namespace pkpy