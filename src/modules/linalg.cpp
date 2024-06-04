#include "pocketpy/modules/linalg.hpp"
#include "pocketpy/interpreter/bindings.hpp"

namespace pkpy {

#define BIND_VEC_VEC_OP(D, name, op)                                                                                   \
    vm->bind##name(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {                                                  \
        Vec##D self = _CAST(Vec##D, _0);                                                                               \
        Vec##D other = CAST(Vec##D, _1);                                                                               \
        return VAR(self op other);                                                                                     \
    });

#define BIND_VEC_FLOAT_OP(D, name, op)                                                                                 \
    vm->bind##name(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {                                                  \
        Vec##D self = _CAST(Vec##D, _0);                                                                               \
        f64 other = CAST(f64, _1);                                                                                     \
        return VAR(self op other);                                                                                     \
    });

#define BIND_VEC_FUNCTION_0(T, name)                                                                                   \
    vm->bind_func(type, #name, 1, [](VM* vm, ArgsView args) {                                                          \
        T self = _CAST(T, args[0]);                                                                                    \
        return VAR(self.name());                                                                                       \
    });

#define BIND_VEC_FUNCTION_1(T, name)                                                                                   \
    vm->bind_func(type, #name, 2, [](VM* vm, ArgsView args) {                                                          \
        T self = _CAST(T, args[0]);                                                                                    \
        T other = CAST(T, args[1]);                                                                                    \
        return VAR(self.name(other));                                                                                  \
    });

#define BIND_VEC_MUL_OP(D)                                                                                             \
    vm->bind__mul__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {                                                 \
        Vec##D self = _CAST(Vec##D, _0);                                                                               \
        if(vm->is_user_type<Vec##D>(_1)) {                                                                             \
            Vec##D other = _CAST(Vec##D, _1);                                                                          \
            return VAR(self * other);                                                                                  \
        }                                                                                                              \
        f64 other = CAST(f64, _1);                                                                                     \
        return VAR(self * other);                                                                                      \
    });                                                                                                                \
    vm->bind_func(type, "__rmul__", 2, [](VM* vm, ArgsView args) {                                                     \
        Vec##D self = _CAST(Vec##D, args[0]);                                                                          \
        f64 other = CAST(f64, args[1]);                                                                                \
        return VAR(self * other);                                                                                      \
    });                                                                                                                \
    vm->bind__truediv__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {                                             \
        Vec##D self = _CAST(Vec##D, _0);                                                                               \
        f64 other = CAST(f64, _1);                                                                                     \
        return VAR(self / other);                                                                                      \
    });

#define BIND_VEC_GETITEM(D)                                                                                            \
    vm->bind__getitem__(type->as<Type>(), [](VM* vm, PyVar obj, PyVar index) {                                         \
        Vec##D self = _CAST(Vec##D, obj);                                                                              \
        i64 i = CAST(i64, index);                                                                                      \
        if(i < 0 || i >= D) vm->IndexError("index out of range");                                                      \
        return VAR(self[i]);                                                                                           \
    });

#define BIND_SSO_VEC_COMMON(D)                                                                                         \
    vm->bind__eq__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {                                                  \
        Vec##D self = _CAST(Vec##D, _0);                                                                               \
        if(!vm->is_user_type<Vec##D>(_1)) return vm->NotImplemented;                                                   \
        Vec##D other = _CAST(Vec##D, _1);                                                                              \
        return VAR(self == other);                                                                                     \
    });                                                                                                                \
    vm->bind_func(type, "__getnewargs__", 1, [](VM* vm, ArgsView args) {                                               \
        Vec##D self = _CAST(Vec##D, args[0]);                                                                          \
        Tuple t(D);                                                                                                    \
        for(int i = 0; i < D; i++)                                                                                     \
            t[i] = VAR(self[i]);                                                                                       \
        return VAR(std::move(t));                                                                                      \
    });

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector2.cs#L289
static Vec2
    SmoothDamp(Vec2 current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {
    // Based on Game Programming Gems 4 Chapter 1.10
    smoothTime = (std::max)(0.0001F, smoothTime);
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
    if(sqDist > maxChangeSq) {
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

    if(origMinusCurrent_x * outMinusOrig_x + origMinusCurrent_y * outMinusOrig_y > 0) {
        output_x = originalTo.x;
        output_y = originalTo.y;

        currentVelocity.x = (output_x - originalTo.x) / deltaTime;
        currentVelocity.y = (output_y - originalTo.y) / deltaTime;
    }
    return Vec2(output_x, output_y);
}

void Vec2::_register(VM* vm, PyObject* mod, PyObject* type) {
    type->attr().set("ZERO", vm->new_user_object<Vec2>(0, 0));
    type->attr().set("ONE", vm->new_user_object<Vec2>(1, 1));

    vm->bind_func(type, __new__, 3, [](VM* vm, ArgsView args) {
        float x = CAST_F(args[1]);
        float y = CAST_F(args[2]);
        return vm->new_object<Vec2>(PK_OBJ_GET(Type, args[0]), x, y);
    });

    // @staticmethod
    vm->bind(
        type,
        "smooth_damp(current: vec2, target: vec2, current_velocity_: vec2, smooth_time: float, max_speed: float, delta_time: float) -> vec2",
        [](VM* vm, ArgsView args) {
            Vec2 current = CAST(Vec2, args[0]);
            Vec2 target = CAST(Vec2, args[1]);
            Vec2 current_velocity_ = CAST(Vec2, args[2]);
            float smooth_time = CAST_F(args[3]);
            float max_speed = CAST_F(args[4]);
            float delta_time = CAST_F(args[5]);
            Vec2 ret = SmoothDamp(current, target, current_velocity_, smooth_time, max_speed, delta_time);
            return VAR(Tuple(VAR(ret), VAR(current_velocity_)));
        },
        {},
        BindType::STATICMETHOD);

    // @staticmethod
    vm->bind(
        type,
        "angle(__from: vec2, __to: vec2) -> float",
        [](VM* vm, ArgsView args) {
            Vec2 __from = CAST(Vec2, args[0]);
            Vec2 __to = CAST(Vec2, args[1]);
            float val = atan2f(__to.y, __to.x) - atan2f(__from.y, __from.x);
            const float PI = 3.1415926535897932384f;
            if(val > PI) val -= 2 * PI;
            if(val < -PI) val += 2 * PI;
            return VAR(val);
        },
        {},
        BindType::STATICMETHOD);

    vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar obj) -> Str {
        Vec2 self = _CAST(Vec2, obj);
        SStream ss;
        ss.setprecision(3);
        ss << "vec2(" << self.x << ", " << self.y << ")";
        return ss.str();
    });

    vm->bind_func(type, "rotate", 2, [](VM* vm, ArgsView args) {
        Vec2 self = _CAST(Vec2, args[0]);
        float radian = CAST(f64, args[1]);
        return vm->new_user_object<Vec2>(self.rotate(radian));
    });

    PY_READONLY_FIELD(Vec2, "x", x)
    PY_READONLY_FIELD(Vec2, "y", y)

    BIND_VEC_VEC_OP(2, __add__, +)
    BIND_VEC_VEC_OP(2, __sub__, -)
    BIND_VEC_MUL_OP(2)
    BIND_VEC_FLOAT_OP(2, __truediv__, /)
    BIND_VEC_FUNCTION_1(Vec2, dot)
    BIND_VEC_FUNCTION_1(Vec2, cross)
    BIND_VEC_FUNCTION_0(Vec2, length)
    BIND_VEC_FUNCTION_0(Vec2, length_squared)
    BIND_VEC_FUNCTION_0(Vec2, normalize)
    BIND_VEC_GETITEM(2)
    BIND_SSO_VEC_COMMON(2)
}

void Vec3::_register(VM* vm, PyObject* mod, PyObject* type) {
    type->attr().set("ZERO", vm->new_user_object<Vec3>(0, 0, 0));
    type->attr().set("ONE", vm->new_user_object<Vec3>(1, 1, 1));

    vm->bind_func(type, __new__, 4, [](VM* vm, ArgsView args) {
        float x = CAST_F(args[1]);
        float y = CAST_F(args[2]);
        float z = CAST_F(args[3]);
        return vm->new_object<Vec3>(PK_OBJ_GET(Type, args[0]), x, y, z);
    });

    vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar obj) -> Str {
        Vec3 self = _CAST(Vec3, obj);
        SStream ss;
        ss.setprecision(3);
        ss << "vec3(" << self.x << ", " << self.y << ", " << self.z << ")";
        return ss.str();
    });

    PY_READONLY_FIELD(Vec3, "x", x)
    PY_READONLY_FIELD(Vec3, "y", y)
    PY_READONLY_FIELD(Vec3, "z", z)

    BIND_VEC_VEC_OP(3, __add__, +)
    BIND_VEC_VEC_OP(3, __sub__, -)
    BIND_VEC_MUL_OP(3)
    BIND_VEC_FUNCTION_1(Vec3, dot)
    BIND_VEC_FUNCTION_1(Vec3, cross)
    BIND_VEC_FUNCTION_0(Vec3, length)
    BIND_VEC_FUNCTION_0(Vec3, length_squared)
    BIND_VEC_FUNCTION_0(Vec3, normalize)
    BIND_VEC_GETITEM(3)
    BIND_SSO_VEC_COMMON(3)
}

void Vec4::_register(VM* vm, PyObject* mod, PyObject* type) {
    PY_STRUCT_LIKE(Vec4)

    type->attr().set("ZERO", vm->new_user_object<Vec4>(0, 0, 0, 0));
    type->attr().set("ONE", vm->new_user_object<Vec4>(1, 1, 1, 1));

    vm->bind_func(type, __new__, 5, [](VM* vm, ArgsView args) {
        float x = CAST_F(args[1]);
        float y = CAST_F(args[2]);
        float z = CAST_F(args[3]);
        float w = CAST_F(args[4]);
        return vm->new_object<Vec4>(PK_OBJ_GET(Type, args[0]), x, y, z, w);
    });

    vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar obj) -> Str {
        Vec4 self = _CAST(Vec4&, obj);
        SStream ss;
        ss.setprecision(3);
        ss << "vec4(" << self.x << ", " << self.y << ", " << self.z << ", " << self.w << ")";
        return ss.str();
    });

    PY_FIELD(Vec4, "x", x)
    PY_FIELD(Vec4, "y", y)
    PY_FIELD(Vec4, "z", z)
    PY_FIELD(Vec4, "w", w)

    BIND_VEC_VEC_OP(4, __add__, +)
    BIND_VEC_VEC_OP(4, __sub__, -)
    BIND_VEC_MUL_OP(4)
    BIND_VEC_FUNCTION_1(Vec4&, dot)
    BIND_VEC_FUNCTION_1(Vec4&, copy_)
    BIND_VEC_FUNCTION_0(Vec4&, length)
    BIND_VEC_FUNCTION_0(Vec4&, length_squared)
    BIND_VEC_FUNCTION_0(Vec4&, normalize)
    BIND_VEC_FUNCTION_0(Vec4&, normalize_)
    BIND_VEC_GETITEM(4)
}

#undef BIND_VEC_VEC_OP
#undef BIND_VEC_MUL_OP
#undef BIND_VEC_FUNCTION_0
#undef BIND_VEC_FUNCTION_1
#undef BIND_VEC_GETITEM

void Mat3x3::_register(VM* vm, PyObject* mod, PyObject* type) {
    PY_STRUCT_LIKE(Mat3x3)

    vm->bind_func(type, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 0) return vm->new_object<Mat3x3>(PK_OBJ_GET(Type, args[0]), Mat3x3::zeros());
        if(args.size() == 1 + 1) {
            const List& list = CAST(List&, args[1]);
            if(list.size() != 9) vm->TypeError("Mat3x3.__new__ takes a list of 9 floats");
            Mat3x3 mat;
            for(int i = 0; i < 9; i++)
                mat.v[i] = CAST_F(list[i]);
            return vm->new_object<Mat3x3>(PK_OBJ_GET(Type, args[0]), mat);
        }
        if(args.size() == 1 + 9) {
            Mat3x3 mat;
            for(int i = 0; i < 9; i++)
                mat.v[i] = CAST_F(args[1 + i]);
            return vm->new_object<Mat3x3>(PK_OBJ_GET(Type, args[0]), mat);
        }
        vm->TypeError(_S("Mat3x3.__new__ takes 0 or 1 or 9 arguments, got ", args.size() - 1));
        return vm->None;
    });

    vm->bind_func(type, "copy_", 2, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        const Mat3x3& other = CAST(Mat3x3&, args[1]);
        self = other;
        return vm->None;
    });

    vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar obj) -> Str {
        const Mat3x3& self = _CAST(Mat3x3&, obj);
        SStream ss;
        ss.setprecision(3);
        ss << "mat3x3([" << self._11 << ", " << self._12 << ", " << self._13 << ",\n";
        ss << "        " << self._21 << ", " << self._22 << ", " << self._23 << ",\n";
        ss << "        " << self._31 << ", " << self._32 << ", " << self._33 << "])";
        return ss.str();
    });

    vm->bind__getitem__(type->as<Type>(), [](VM* vm, PyVar obj, PyVar index) {
        Mat3x3& self = _CAST(Mat3x3&, obj);
        Tuple& t = CAST(Tuple&, index);
        if(t.size() != 2) { vm->TypeError("Mat3x3.__getitem__ takes a tuple of 2 integers"); }
        i64 i = CAST(i64, t[0]);
        i64 j = CAST(i64, t[1]);
        if(i < 0 || i >= 3 || j < 0 || j >= 3) { vm->IndexError("index out of range"); }
        return VAR(self.m[i][j]);
    });

    vm->bind__setitem__(type->as<Type>(), [](VM* vm, PyVar obj, PyVar index, PyVar value) {
        Mat3x3& self = _CAST(Mat3x3&, obj);
        const Tuple& t = CAST(Tuple&, index);
        if(t.size() != 2) { vm->TypeError("Mat3x3.__setitem__ takes a tuple of 2 integers"); }
        i64 i = CAST(i64, t[0]);
        i64 j = CAST(i64, t[1]);
        if(i < 0 || i >= 3 || j < 0 || j >= 3) { vm->IndexError("index out of range"); }
        self.m[i][j] = CAST_F(value);
    });

    vm->bind_field(type, "_11", &Mat3x3::_11);
    vm->bind_field(type, "_12", &Mat3x3::_12);
    vm->bind_field(type, "_13", &Mat3x3::_13);
    vm->bind_field(type, "_21", &Mat3x3::_21);
    vm->bind_field(type, "_22", &Mat3x3::_22);
    vm->bind_field(type, "_23", &Mat3x3::_23);
    vm->bind_field(type, "_31", &Mat3x3::_31);
    vm->bind_field(type, "_32", &Mat3x3::_32);
    vm->bind_field(type, "_33", &Mat3x3::_33);

    vm->bind__add__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
        Mat3x3& self = _CAST(Mat3x3&, _0);
        Mat3x3& other = CAST(Mat3x3&, _1);
        return vm->new_user_object<Mat3x3>(self + other);
    });

    vm->bind__sub__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
        Mat3x3& self = _CAST(Mat3x3&, _0);
        Mat3x3& other = CAST(Mat3x3&, _1);
        return vm->new_user_object<Mat3x3>(self - other);
    });

    vm->bind__mul__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
        Mat3x3& self = _CAST(Mat3x3&, _0);
        f64 other = CAST_F(_1);
        return vm->new_user_object<Mat3x3>(self * other);
    });

    vm->bind_func(type, "__rmul__", 2, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        f64 other = CAST_F(args[1]);
        return vm->new_user_object<Mat3x3>(self * other);
    });

    vm->bind__truediv__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
        Mat3x3& self = _CAST(Mat3x3&, _0);
        f64 other = CAST_F(_1);
        return vm->new_user_object<Mat3x3>(self / other);
    });

    vm->bind__matmul__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
        Mat3x3& self = _CAST(Mat3x3&, _0);
        if(vm->is_user_type<Mat3x3>(_1)) {
            const Mat3x3& other = _CAST(Mat3x3&, _1);
            return vm->new_user_object<Mat3x3>(self.matmul(other));
        }
        if(vm->is_user_type<Vec3>(_1)) {
            const Vec3 other = _CAST(Vec3, _1);
            return vm->new_user_object<Vec3>(self.matmul(other));
        }
        return vm->NotImplemented;
    });

    vm->bind(type, "matmul(self, other: mat3x3, out: mat3x3 = None)", [](VM* vm, ArgsView args) {
        const Mat3x3& self = _CAST(Mat3x3&, args[0]);
        const Mat3x3& other = CAST(Mat3x3&, args[1]);
        if(args[2] == vm->None) {
            return vm->new_user_object<Mat3x3>(self.matmul(other));
        } else {
            Mat3x3& out = CAST(Mat3x3&, args[2]);
            out = self.matmul(other);
            return vm->None;
        }
    });

    vm->bind_func(type, "determinant", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        return VAR(self.determinant());
    });

    vm->bind_func(type, "transpose", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        return vm->new_user_object<Mat3x3>(self.transpose());
    });

    vm->bind__invert__(type->as<Type>(), [](VM* vm, PyVar obj) {
        Mat3x3& self = _CAST(Mat3x3&, obj);
        Mat3x3 ret;
        if(!self.inverse(ret)) vm->ValueError("matrix is not invertible");
        return vm->new_user_object<Mat3x3>(ret);
    });

    vm->bind_func(type, "inverse", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Mat3x3 ret;
        if(!self.inverse(ret)) vm->ValueError("matrix is not invertible");
        return vm->new_user_object<Mat3x3>(ret);
    });

    vm->bind_func(type, "inverse_", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Mat3x3 ret;
        if(!self.inverse(ret)) vm->ValueError("matrix is not invertible");
        self = ret;
        return vm->None;
    });

    vm->bind_func(type, "transpose_", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        self = self.transpose();
        return vm->None;
    });

    // @staticmethod
    vm->bind_func(
        type,
        "zeros",
        0,
        [](VM* vm, ArgsView args) {
            return vm->new_user_object<Mat3x3>(Mat3x3::zeros());
        },
        {},
        BindType::STATICMETHOD);

    // @staticmethod
    vm->bind_func(
        type,
        "ones",
        0,
        [](VM* vm, ArgsView args) {
            return vm->new_user_object<Mat3x3>(Mat3x3::ones());
        },
        {},
        BindType::STATICMETHOD);

    // @staticmethod
    vm->bind_func(
        type,
        "identity",
        0,
        [](VM* vm, ArgsView args) {
            return vm->new_user_object<Mat3x3>(Mat3x3::identity());
        },
        {},
        BindType::STATICMETHOD);

    /*************** affine transformations ***************/
    // @staticmethod
    vm->bind(
        type,
        "trs(t: vec2, r: float, s: vec2)",
        [](VM* vm, ArgsView args) {
            Vec2 t = CAST(Vec2, args[0]);
            f64 r = CAST_F(args[1]);
            Vec2 s = CAST(Vec2, args[2]);
            return vm->new_user_object<Mat3x3>(Mat3x3::trs(t, r, s));
        },
        {},
        BindType::STATICMETHOD);

    vm->bind(type, "copy_trs_(self, t: vec2, r: float, s: vec2)", [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 t = CAST(Vec2, args[1]);
        f64 r = CAST_F(args[2]);
        Vec2 s = CAST(Vec2, args[3]);
        self = Mat3x3::trs(t, r, s);
        return vm->None;
    });

    vm->bind(type, "copy_t_(self, t: vec2)", [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 t = CAST(Vec2, args[1]);
        self = Mat3x3::trs(t, self._r(), self._s());
        return vm->None;
    });

    vm->bind(type, "copy_r_(self, r: float)", [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        f64 r = CAST_F(args[1]);
        self = Mat3x3::trs(self._t(), r, self._s());
        return vm->None;
    });

    vm->bind(type, "copy_s_(self, s: vec2)", [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 s = CAST(Vec2, args[1]);
        self = Mat3x3::trs(self._t(), self._r(), s);
        return vm->None;
    });

    vm->bind_func(type, "is_affine", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        return VAR(self.is_affine());
    });

    vm->bind_func(type, "_t", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        return vm->new_user_object<Vec2>(self._t());
    });

    vm->bind_func(type, "_r", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        return VAR(self._r());
    });

    vm->bind_func(type, "_s", 1, [](VM* vm, ArgsView args) {
        Mat3x3& self = _CAST(Mat3x3&, args[0]);
        return vm->new_user_object<Vec2>(self._s());
    });

    vm->bind_func(type, "transform_point", 2, [](VM* vm, ArgsView args) {
        const Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 v = CAST(Vec2, args[1]);
        Vec2 res(self._11 * v.x + self._12 * v.y + self._13, self._21 * v.x + self._22 * v.y + self._23);
        return vm->new_user_object<Vec2>(res);
    });

    vm->bind_func(type, "inverse_transform_point", 2, [](VM* vm, ArgsView args) {
        const Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 v = CAST(Vec2, args[1]);
        Mat3x3 inv;
        if(!self.inverse(inv)) vm->ValueError("matrix is not invertible");
        Vec2 res(inv._11 * v.x + inv._12 * v.y + inv._13, inv._21 * v.x + inv._22 * v.y + inv._23);
        return vm->new_user_object<Vec2>(res);
    });

    vm->bind_func(type, "transform_vector", 2, [](VM* vm, ArgsView args) {
        const Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 v = CAST(Vec2, args[1]);
        Vec2 res(self._11 * v.x + self._12 * v.y, self._21 * v.x + self._22 * v.y);
        return vm->new_user_object<Vec2>(res);
    });

    vm->bind_func(type, "inverse_transform_vector", 2, [](VM* vm, ArgsView args) {
        const Mat3x3& self = _CAST(Mat3x3&, args[0]);
        Vec2 v = CAST(Vec2, args[1]);
        Mat3x3 inv;
        if(!self.inverse(inv)) vm->ValueError("matrix is not invertible");
        Vec2 res(inv._11 * v.x + inv._12 * v.y, inv._21 * v.x + inv._22 * v.y);
        return vm->new_user_object<Vec2>(res);
    });
}

void add_module_linalg(VM* vm) {
    PyObject* linalg = vm->new_module("linalg");

    vm->register_user_class<Vec2>(linalg, "vec2", VM::tp_object);
    vm->register_user_class<Vec3>(linalg, "vec3", VM::tp_object);
    vm->register_user_class<Vec4>(linalg, "vec4", VM::tp_object, true);
    vm->register_user_class<Mat3x3>(linalg, "mat3x3", VM::tp_object, true);

    PyVar float_p = vm->_modules["c"]->attr("float_p");
    linalg->attr().set("vec4_p", float_p);
    linalg->attr().set("mat3x3_p", float_p);
}

/////////////// mat3x3 ///////////////
Mat3x3::Mat3x3() {}

Mat3x3::Mat3x3(float _11, float _12, float _13, float _21, float _22, float _23, float _31, float _32, float _33) :
    _11(_11), _12(_12), _13(_13), _21(_21), _22(_22), _23(_23), _31(_31), _32(_32), _33(_33) {}

Mat3x3 Mat3x3::zeros() { return Mat3x3(0, 0, 0, 0, 0, 0, 0, 0, 0); }

Mat3x3 Mat3x3::ones() { return Mat3x3(1, 1, 1, 1, 1, 1, 1, 1, 1); }

Mat3x3 Mat3x3::identity() { return Mat3x3(1, 0, 0, 0, 1, 0, 0, 0, 1); }

Mat3x3 Mat3x3::operator+ (const Mat3x3& other) const {
    Mat3x3 ret;
    for(int i = 0; i < 9; ++i)
        ret.v[i] = v[i] + other.v[i];
    return ret;
}

Mat3x3 Mat3x3::operator- (const Mat3x3& other) const {
    Mat3x3 ret;
    for(int i = 0; i < 9; ++i)
        ret.v[i] = v[i] - other.v[i];
    return ret;
}

Mat3x3 Mat3x3::operator* (float scalar) const {
    Mat3x3 ret;
    for(int i = 0; i < 9; ++i)
        ret.v[i] = v[i] * scalar;
    return ret;
}

Mat3x3 Mat3x3::operator/ (float scalar) const {
    Mat3x3 ret;
    for(int i = 0; i < 9; ++i)
        ret.v[i] = v[i] / scalar;
    return ret;
}

bool Mat3x3::operator== (const Mat3x3& other) const {
    for(int i = 0; i < 9; ++i) {
        if(!isclose(v[i], other.v[i])) return false;
    }
    return true;
}

bool Mat3x3::operator!= (const Mat3x3& other) const {
    for(int i = 0; i < 9; ++i) {
        if(!isclose(v[i], other.v[i])) return true;
    }
    return false;
}

Mat3x3 Mat3x3::matmul(const Mat3x3& other) const {
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

Vec3 Mat3x3::matmul(const Vec3& other) const {
    Vec3 out;
    out.x = _11 * other.x + _12 * other.y + _13 * other.z;
    out.y = _21 * other.x + _22 * other.y + _23 * other.z;
    out.z = _31 * other.x + _32 * other.y + _33 * other.z;
    return out;
}

float Mat3x3::determinant() const {
    return _11 * _22 * _33 + _12 * _23 * _31 + _13 * _21 * _32 - _11 * _23 * _32 - _12 * _21 * _33 - _13 * _22 * _31;
}

Mat3x3 Mat3x3::transpose() const {
    Mat3x3 ret;
    ret._11 = _11;
    ret._12 = _21;
    ret._13 = _31;
    ret._21 = _12;
    ret._22 = _22;
    ret._23 = _32;
    ret._31 = _13;
    ret._32 = _23;
    ret._33 = _33;
    return ret;
}

bool Mat3x3::inverse(Mat3x3& out) const {
    float det = determinant();
    if(isclose(det, 0)) return false;
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

Mat3x3 Mat3x3::trs(Vec2 t, float radian, Vec2 s) {
    float cr = cosf(radian);
    float sr = sinf(radian);
    return Mat3x3(s.x * cr, -s.y * sr, t.x, s.x * sr, s.y * cr, t.y, 0.0f, 0.0f, 1.0f);
}

bool Mat3x3::is_affine() const {
    float det = _11 * _22 - _12 * _21;
    if(isclose(det, 0)) return false;
    return _31 == 0.0f && _32 == 0.0f && _33 == 1.0f;
}

Vec2 Mat3x3::_t() const { return Vec2(_13, _23); }

float Mat3x3::_r() const { return atan2f(_21, _11); }

Vec2 Mat3x3::_s() const { return Vec2(sqrtf(_11 * _11 + _21 * _21), sqrtf(_12 * _12 + _22 * _22)); }

}  // namespace pkpy
