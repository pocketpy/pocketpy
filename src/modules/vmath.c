#include "pocketpy/vmath.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/vm.h"
#include <math.h>

static bool isclose(float a, float b) { return fabs(a - b) < 1e-4; }

#define DEFINE_VEC_FIELD(name, T, Tc, field)                                                       \
    static bool name##__##field(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(1);                                                                          \
        py_new##T(py_retval(), py_to##name(argv).field);                                           \
        return true;                                                                               \
    }                                                                                              \
    static bool name##__with_##field(int argc, py_Ref argv) {                                      \
        PY_CHECK_ARGC(2);                                                                          \
        Tc val;                                                                                    \
        if(!py_cast##T(&argv[1], &val)) return false;                                              \
        c11_##name v = py_to##name(argv);                                                          \
        v.field = val;                                                                             \
        py_new##name(py_retval(), v);                                                              \
        return true;                                                                               \
    }

#define DEFINE_BOOL_NE(name, f_eq)                                                                 \
    static bool name##__ne__(int argc, py_Ref argv) {                                              \
        f_eq(argc, argv);                                                                          \
        py_Ref ret = py_retval();                                                                  \
        if(ret->type == tp_NotImplementedType) return true;                                        \
        ret->_bool = !ret->_bool;                                                                  \
        return true;                                                                               \
    }

void py_newvec2(py_OutRef out, c11_vec2 v) {
    out->type = tp_vec2;
    out->is_ptr = false;
    out->_vec2 = v;
}

c11_vec2 py_tovec2(py_Ref self) {
    assert(self->type == tp_vec2);
    return self->_vec2;
}

void py_newvec2i(py_OutRef out, c11_vec2i v) {
    out->type = tp_vec2i;
    out->is_ptr = false;
    out->_vec2i = v;
}

c11_vec2i py_tovec2i(py_Ref self) {
    assert(self->type == tp_vec2i);
    return self->_vec2i;
}

void py_newvec3(py_OutRef out, c11_vec3 v) {
    out->type = tp_vec3;
    out->is_ptr = false;
    out->_vec3 = v;
}

c11_vec3 py_tovec3(py_Ref self) {
    assert(self->type == tp_vec3);
    return self->_vec3;
}

void py_newvec3i(py_OutRef out, c11_vec3i v) {
    out->type = tp_vec3i;
    out->is_ptr = false;
    out->_vec3i = v;
}

c11_vec3i py_tovec3i(py_Ref self) {
    assert(self->type == tp_vec3i);
    return self->_vec3i;
}

c11_mat3x3* py_newmat3x3(py_OutRef out) {
    return py_newobject(out, tp_mat3x3, 0, sizeof(c11_mat3x3));
}

c11_mat3x3* py_tomat3x3(py_Ref self) {
    assert(self->type == tp_mat3x3);
    return py_touserdata(self);
}

static py_Ref _const(py_Type type, const char* name) {
    return py_emplacedict(py_tpobject(type), py_name(name));
}

#define DEF_VECTOR_ELEMENT_WISE(D, T, name, op)                                                    \
    static bool T##name(int argc, py_Ref argv) {                                                   \
        PY_CHECK_ARGC(2);                                                                          \
        if(argv[1].type != tp_##T) {                                                               \
            py_newnotimplemented(py_retval());                                                     \
            return true;                                                                           \
        }                                                                                          \
        c11_##T a = py_to##T(&argv[0]);                                                            \
        c11_##T b = py_to##T(&argv[1]);                                                            \
        c11_##T res;                                                                               \
        for(int i = 0; i < D; i++)                                                                 \
            res.data[i] = a.data[i] op b.data[i];                                                  \
        py_new##T(py_retval(), res);                                                               \
        return true;                                                                               \
    }

#define DEF_VECTOR_OPS(D)                                                                          \
    static bool vec##D##__new__(int argc, py_Ref argv) {                                           \
        c11_vec##D res;                                                                            \
        if(argc == 2) {                                                                            \
            PY_CHECK_ARG_TYPE(1, tp_vec##D##i);                                                    \
            c11_vec##D##i v = py_tovec##D##i(&argv[1]);                                            \
            for(int i = 0; i < D; i++)                                                             \
                res.data[i] = v.data[i];                                                           \
        } else {                                                                                   \
            PY_CHECK_ARGC(D + 1);                                                                  \
            for(int i = 0; i < D; i++) {                                                           \
                if(!py_castfloat32(&argv[i + 1], &res.data[i])) return false;                      \
            }                                                                                      \
        }                                                                                          \
        py_newvec##D(py_retval(), res);                                                            \
        return true;                                                                               \
    }                                                                                              \
    DEF_VECTOR_ELEMENT_WISE(D, vec##D, __add__, +)                                                 \
    DEF_VECTOR_ELEMENT_WISE(D, vec##D, __sub__, -)                                                 \
    static bool vec##D##__mul__(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(2);                                                                          \
        c11_vec##D res;                                                                            \
        switch(argv[1].type) {                                                                     \
            case tp_vec##D: {                                                                      \
                c11_vec##D a = py_tovec##D(&argv[0]);                                              \
                c11_vec##D b = py_tovec##D(&argv[1]);                                              \
                for(int i = 0; i < D; i++)                                                         \
                    res.data[i] = a.data[i] * b.data[i];                                           \
                py_newvec##D(py_retval(), res);                                                    \
                return true;                                                                       \
            }                                                                                      \
            case tp_int: {                                                                         \
                c11_vec##D a = py_tovec##D(&argv[0]);                                              \
                py_i64 b = argv[1]._i64;                                                           \
                for(int i = 0; i < D; i++)                                                         \
                    res.data[i] = a.data[i] * b;                                                   \
                py_newvec##D(py_retval(), res);                                                    \
                return true;                                                                       \
            }                                                                                      \
            case tp_float: {                                                                       \
                c11_vec##D a = py_tovec##D(&argv[0]);                                              \
                py_f64 b = argv[1]._f64;                                                           \
                for(int i = 0; i < D; i++)                                                         \
                    res.data[i] = a.data[i] * b;                                                   \
                py_newvec##D(py_retval(), res);                                                    \
                return true;                                                                       \
            }                                                                                      \
            default: py_newnotimplemented(py_retval()); return true;                               \
        }                                                                                          \
    }                                                                                              \
    static bool vec##D##__truediv__(int argc, py_Ref argv) {                                       \
        PY_CHECK_ARGC(2);                                                                          \
        float divisor;                                                                             \
        if(!py_castfloat32(&argv[1], &divisor)) {                                                  \
            py_clearexc(NULL);                                                                     \
            py_newnotimplemented(py_retval());                                                     \
            return true;                                                                           \
        }                                                                                          \
        c11_vec##D res;                                                                            \
        c11_vec##D a = py_tovec##D(&argv[0]);                                                      \
        for(int i = 0; i < D; i++)                                                                 \
            res.data[i] = a.data[i] / divisor;                                                     \
        py_newvec##D(py_retval(), res);                                                            \
        return true;                                                                               \
    }                                                                                              \
    static bool vec##D##__eq__(int argc, py_Ref argv) {                                            \
        PY_CHECK_ARGC(2);                                                                          \
        if(argv[1].type != tp_vec##D) {                                                            \
            py_newnotimplemented(py_retval());                                                     \
            return true;                                                                           \
        }                                                                                          \
        c11_vec##D lhs = py_tovec##D(&argv[0]);                                                    \
        c11_vec##D rhs = py_tovec##D(&argv[1]);                                                    \
        bool ok = true;                                                                            \
        for(int i = 0; i < D; i++) {                                                               \
            if(!isclose(lhs.data[i], rhs.data[i])) ok = false;                                     \
        }                                                                                          \
        py_newbool(py_retval(), ok);                                                               \
        return true;                                                                               \
    }                                                                                              \
    DEFINE_BOOL_NE(vec##D, vec##D##__eq__)                                                         \
    static bool vec##D##_length(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(1);                                                                          \
        c11_vec##D v = py_tovec##D(argv);                                                          \
        float sum = 0;                                                                             \
        for(int i = 0; i < D; i++)                                                                 \
            sum += v.data[i] * v.data[i];                                                          \
        py_newfloat(py_retval(), sqrtf(sum));                                                      \
        return true;                                                                               \
    }                                                                                              \
    static bool vec##D##_length_squared(int argc, py_Ref argv) {                                   \
        PY_CHECK_ARGC(1);                                                                          \
        c11_vec##D v = py_tovec##D(argv);                                                          \
        float sum = 0;                                                                             \
        for(int i = 0; i < D; i++)                                                                 \
            sum += v.data[i] * v.data[i];                                                          \
        py_newfloat(py_retval(), sum);                                                             \
        return true;                                                                               \
    }                                                                                              \
    static bool vec##D##_dot(int argc, py_Ref argv) {                                              \
        PY_CHECK_ARGC(2);                                                                          \
        PY_CHECK_ARG_TYPE(1, tp_vec##D);                                                           \
        c11_vec##D a = py_tovec##D(&argv[0]);                                                      \
        c11_vec##D b = py_tovec##D(&argv[1]);                                                      \
        float sum = 0;                                                                             \
        for(int i = 0; i < D; i++)                                                                 \
            sum += a.data[i] * b.data[i];                                                          \
        py_newfloat(py_retval(), sum);                                                             \
        return true;                                                                               \
    }                                                                                              \
    static bool vec##D##_normalize(int argc, py_Ref argv) {                                        \
        PY_CHECK_ARGC(1);                                                                          \
        c11_vec##D self = py_tovec##D(argv);                                                       \
        float len = 0;                                                                             \
        for(int i = 0; i < D; i++)                                                                 \
            len += self.data[i] * self.data[i];                                                    \
        if(isclose(len, 0)) return ZeroDivisionError("cannot normalize zero vector");              \
        len = sqrtf(len);                                                                          \
        c11_vec##D res;                                                                            \
        for(int i = 0; i < D; i++)                                                                 \
            res.data[i] = self.data[i] / len;                                                      \
        py_newvec##D(py_retval(), res);                                                            \
        return true;                                                                               \
    }

DEF_VECTOR_OPS(2)
DEF_VECTOR_OPS(3)

#define DEF_VECTOR_INT_OPS(D)                                                                      \
    static bool vec##D##i__new__(int argc, py_Ref argv) {                                          \
        PY_CHECK_ARGC(D + 1);                                                                      \
        c11_vec##D##i res;                                                                         \
        for(int i = 0; i < D; i++) {                                                               \
            if(!py_checkint(&argv[i + 1])) return false;                                           \
            res.data[i] = py_toint(&argv[i + 1]);                                                  \
        }                                                                                          \
        py_newvec##D##i(py_retval(), res);                                                         \
        return true;                                                                               \
    }                                                                                              \
    DEF_VECTOR_ELEMENT_WISE(D, vec##D##i, __add__, +)                                              \
    DEF_VECTOR_ELEMENT_WISE(D, vec##D##i, __sub__, -)                                              \
    static bool vec##D##i__mul__(int argc, py_Ref argv) {                                          \
        PY_CHECK_ARGC(2);                                                                          \
        c11_vec##D##i res;                                                                         \
        switch(argv[1].type) {                                                                     \
            case tp_vec##D##i: {                                                                   \
                c11_vec##D##i a = py_tovec##D##i(&argv[0]);                                        \
                c11_vec##D##i b = py_tovec##D##i(&argv[1]);                                        \
                for(int i = 0; i < D; i++)                                                         \
                    res.data[i] = a.data[i] * b.data[i];                                           \
                py_newvec##D##i(py_retval(), res);                                                 \
                return true;                                                                       \
            }                                                                                      \
            case tp_int: {                                                                         \
                c11_vec##D##i a = py_tovec##D##i(&argv[0]);                                        \
                py_i64 b = argv[1]._i64;                                                           \
                for(int i = 0; i < D; i++)                                                         \
                    res.data[i] = a.data[i] * b;                                                   \
                py_newvec##D##i(py_retval(), res);                                                 \
                return true;                                                                       \
            }                                                                                      \
            default: py_newnotimplemented(py_retval()); return true;                               \
        }                                                                                          \
    }                                                                                              \
    static bool vec##D##i__eq__(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(2);                                                                          \
        if(argv[1].type != tp_vec##D##i) {                                                         \
            py_newnotimplemented(py_retval());                                                     \
            return true;                                                                           \
        }                                                                                          \
        c11_vec##D##i lhs = py_tovec##D##i(&argv[0]);                                              \
        c11_vec##D##i rhs = py_tovec##D##i(&argv[1]);                                              \
        bool ok = true;                                                                            \
        for(int i = 0; i < D; i++) {                                                               \
            if(lhs.data[i] != rhs.data[i]) ok = false;                                             \
        }                                                                                          \
        py_newbool(py_retval(), ok);                                                               \
        return true;                                                                               \
    }                                                                                              \
    DEFINE_BOOL_NE(vec##D##i, vec##D##i__eq__)                                                     \
    static bool vec##D##i##_dot(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(2);                                                                          \
        PY_CHECK_ARG_TYPE(1, tp_vec##D##i);                                                        \
        c11_vec##D##i a = py_tovec##D##i(&argv[0]);                                                \
        c11_vec##D##i b = py_tovec##D##i(&argv[1]);                                                \
        py_i64 sum = 0;                                                                            \
        for(int i = 0; i < D; i++)                                                                 \
            sum += a.data[i] * b.data[i];                                                          \
        py_newint(py_retval(), sum);                                                               \
        return true;                                                                               \
    }                                                                                              \
    static bool vec##D##i##__floordiv__(int argc, py_Ref argv) {                                   \
        PY_CHECK_ARGC(2);                                                                          \
        PY_CHECK_ARG_TYPE(1, tp_int);                                                              \
        c11_vec##D##i a = py_tovec##D##i(&argv[0]);                                                \
        py_i64 b = py_toint(&argv[1]);                                                             \
        for(int i = 0; i < D; i++)                                                                 \
            a.data[i] /= b;                                                                        \
        py_newvec##D##i(py_retval(), a);                                                           \
        return true;                                                                               \
    }

DEF_VECTOR_INT_OPS(2)
DEF_VECTOR_INT_OPS(3)

static bool vec2i__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_vec2i v = py_tovec2i(argv);
    uint64_t x_part = (uint32_t)v.x & 0xFFFFFFFF;
    uint64_t y_part = (uint32_t)v.y & 0xFFFFFFFF;
    uint64_t hash = (x_part << 32) | y_part;
    py_newint(py_retval(), (py_i64)hash);
    return true;
}

static bool vec3i__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_vec3i v = py_tovec3i(argv);
    uint64_t x_part = (uint32_t)v.x & 0xFFFFFF;
    uint64_t y_part = (uint32_t)v.y & 0xFFFFFF;
    uint64_t z_part = (uint32_t)v.z & 0xFFFF;
    uint64_t hash = (x_part << 40) | (y_part << 16) | z_part;
    py_newint(py_retval(), (py_i64)hash);
    return true;
}

static bool vec2__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    char buf[64];
    int size = snprintf(buf, 64, "vec2(%.4f, %.4f)", argv[0]._vec2.x, argv[0]._vec2.y);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

static bool vec2_rotate(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 radians;
    if(!py_castfloat(&argv[1], &radians)) return false;
    float cr = cosf(radians);
    float sr = sinf(radians);
    c11_vec2 res;
    res.x = argv[0]._vec2.x * cr - argv[0]._vec2.y * sr;
    res.y = argv[0]._vec2.x * sr + argv[0]._vec2.y * cr;
    py_newvec2(py_retval(), res);
    return true;
}

static bool vec2_angle_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_vec2);
    PY_CHECK_ARG_TYPE(1, tp_vec2);
    float val = atan2f(argv[1]._vec2.y, argv[1]._vec2.x) - atan2f(argv[0]._vec2.y, argv[0]._vec2.x);
    if(val > PK_M_PI) val -= 2 * (float)PK_M_PI;
    if(val < -PK_M_PI) val += 2 * (float)PK_M_PI;
    py_newfloat(py_retval(), val);
    return true;
}

static bool vec2_smoothdamp_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(6);
    PY_CHECK_ARG_TYPE(0, tp_vec2);  // current: vec2
    PY_CHECK_ARG_TYPE(1, tp_vec2);  // target: vec2
    PY_CHECK_ARG_TYPE(2, tp_vec2);  // current_velocity: vec2

    float smoothTime;
    if(!py_castfloat32(&argv[3], &smoothTime)) return false;
    float maxSpeed;
    if(!py_castfloat32(&argv[4], &maxSpeed)) return false;
    float deltaTime;
    if(!py_castfloat32(&argv[5], &deltaTime)) return false;
    c11_vec2 current = argv[0]._vec2;
    c11_vec2 target = argv[1]._vec2;
    c11_vec2 currentVelocity = argv[2]._vec2;

    // https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector2.cs#L289
    // Based on Game Programming Gems 4 Chapter 1.10
    smoothTime = c11__max(0.0001F, smoothTime);
    float omega = 2.0F / smoothTime;

    float x = omega * deltaTime;
    float exp = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);

    float change_x = current.x - target.x;
    float change_y = current.y - target.y;
    c11_vec2 originalTo = target;

    // Clamp maximum speed
    float maxChange = maxSpeed * smoothTime;

    float maxChangeSq = maxChange * maxChange;
    float sqDist = change_x * change_x + change_y * change_y;
    if(sqDist > maxChangeSq) {
        float mag = sqrtf(sqDist);
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

    py_Ref ret = py_retval();
    py_Ref p = py_newtuple(ret, 2);
    py_newvec2(&p[0],
               (c11_vec2){
                   {output_x, output_y}
    });
    py_newvec2(&p[1], currentVelocity);
    return true;
}

DEFINE_VEC_FIELD(vec2, float, py_f64, x)
DEFINE_VEC_FIELD(vec2, float, py_f64, y)

static bool vec2__with_z(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    float z;
    if(!py_castfloat32(&argv[1], &z)) return false;
    c11_vec3 v = {
        {argv->_vec2.x, argv->_vec2.y, z}
    };
    py_newvec3(py_retval(), v);
    return true;
}

/* mat3x3 */
static bool mat3x3__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(10);
    c11_mat3x3* m = py_newmat3x3(py_retval());
    for(int i = 0; i < 9; i++) {
        py_f64 val;
        if(!py_castfloat(&argv[i + 1], &val)) return false;
        m->data[i] = val;
    }
    return true;
}

static bool mat3x3__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* m = py_tomat3x3(argv);
    char buf[256];
    const char* fmt =
        "mat3x3(%.4f, %.4f, %.4f,\n       %.4f, %.4f, %.4f,\n      "
        " %.4f, %.4f, %.4f)";
    int size = snprintf(buf,
                        256,
                        fmt,
                        m->data[0],
                        m->data[1],
                        m->data[2],
                        m->data[3],
                        m->data[4],
                        m->data[5],
                        m->data[6],
                        m->data[7],
                        m->data[8]);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

static bool mat3x3__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_tuple);
    c11_mat3x3* ud = py_tomat3x3(argv);
    if(py_tuple_len(&argv[1]) != 2) return IndexError("expected a tuple of length 2");
    py_Ref i = py_tuple_getitem(&argv[1], 0);
    py_Ref j = py_tuple_getitem(&argv[1], 1);
    if(!py_checktype(i, tp_int) || !py_checktype(j, tp_int)) return false;
    if(i->_i64 < 0 || i->_i64 >= 3 || j->_i64 < 0 || j->_i64 >= 3) {
        return IndexError("index out of range");
    }
    py_newfloat(py_retval(), ud->m[i->_i64][j->_i64]);
    return true;
}

static bool mat3x3__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_tuple);
    c11_mat3x3* ud = py_tomat3x3(argv);
    if(py_tuple_len(&argv[1]) != 2) return IndexError("expected a tuple of length 2");
    py_Ref i = py_tuple_getitem(&argv[1], 0);
    py_Ref j = py_tuple_getitem(&argv[1], 1);
    if(!py_checktype(i, tp_int) || !py_checktype(j, tp_int)) return false;
    py_f64 val;
    if(!py_castfloat(&argv[2], &val)) return false;
    if(i->_i64 < 0 || i->_i64 >= 3 || j->_i64 < 0 || j->_i64 >= 3) {
        return IndexError("index out of range");
    }
    ud->m[i->_i64][j->_i64] = val;
    py_newnone(py_retval());
    return true;
}

static bool mat3x3__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(argv[1].type != tp_mat3x3) {
        py_newnotimplemented(py_retval());
        return true;
    }
    c11_mat3x3* lhs = py_tomat3x3(argv);
    c11_mat3x3* rhs = py_tomat3x3(&argv[1]);
    for(int i = 0; i < 9; i++) {
        if(!isclose(lhs->data[i], rhs->data[i])) {
            py_newbool(py_retval(), false);
            return true;
        }
    }
    py_newbool(py_retval(), true);
    return true;
}

DEFINE_BOOL_NE(mat3x3, mat3x3__eq__)

static void matmul(const c11_mat3x3* lhs, const c11_mat3x3* rhs, c11_mat3x3* restrict out) {
    out->_11 = lhs->_11 * rhs->_11 + lhs->_12 * rhs->_21 + lhs->_13 * rhs->_31;
    out->_12 = lhs->_11 * rhs->_12 + lhs->_12 * rhs->_22 + lhs->_13 * rhs->_32;
    out->_13 = lhs->_11 * rhs->_13 + lhs->_12 * rhs->_23 + lhs->_13 * rhs->_33;
    out->_21 = lhs->_21 * rhs->_11 + lhs->_22 * rhs->_21 + lhs->_23 * rhs->_31;
    out->_22 = lhs->_21 * rhs->_12 + lhs->_22 * rhs->_22 + lhs->_23 * rhs->_32;
    out->_23 = lhs->_21 * rhs->_13 + lhs->_22 * rhs->_23 + lhs->_23 * rhs->_33;
    out->_31 = lhs->_31 * rhs->_11 + lhs->_32 * rhs->_21 + lhs->_33 * rhs->_31;
    out->_32 = lhs->_31 * rhs->_12 + lhs->_32 * rhs->_22 + lhs->_33 * rhs->_32;
    out->_33 = lhs->_31 * rhs->_13 + lhs->_32 * rhs->_23 + lhs->_33 * rhs->_33;
}

static float determinant(const c11_mat3x3* m) {
    return m->_11 * (m->_22 * m->_33 - m->_23 * m->_32) -
           m->_12 * (m->_21 * m->_33 - m->_23 * m->_31) +
           m->_13 * (m->_21 * m->_32 - m->_22 * m->_31);
}

static bool inverse(const c11_mat3x3* m, c11_mat3x3* restrict out) {
    float det = determinant(m);
    if(isclose(det, 0)) return false;
    float invdet = 1.0f / det;
    out->_11 = (m->_22 * m->_33 - m->_23 * m->_32) * invdet;
    out->_12 = (m->_13 * m->_32 - m->_12 * m->_33) * invdet;
    out->_13 = (m->_12 * m->_23 - m->_13 * m->_22) * invdet;
    out->_21 = (m->_23 * m->_31 - m->_21 * m->_33) * invdet;
    out->_22 = (m->_11 * m->_33 - m->_13 * m->_31) * invdet;
    out->_23 = (m->_13 * m->_21 - m->_11 * m->_23) * invdet;
    out->_31 = (m->_21 * m->_32 - m->_22 * m->_31) * invdet;
    out->_32 = (m->_12 * m->_31 - m->_11 * m->_32) * invdet;
    out->_33 = (m->_11 * m->_22 - m->_12 * m->_21) * invdet;
    return true;
}

static void trs(c11_vec2 t, float r, c11_vec2 s, c11_mat3x3* restrict out) {
    float cr = cosf(r);
    float sr = sinf(r);
    // clang-format off
    *out = (c11_mat3x3){
        ._11 = s.x * cr, ._12 = -s.y * sr, ._13 = t.x,
        ._21 = s.x * sr, ._22 = s.y * cr, ._23 = t.y,
        ._31 = 0, ._32 = 0, ._33 = 1,
    };
    // clang-format on
}

static bool mat3x3__matmul__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_mat3x3* lhs = py_tomat3x3(argv);
    if(argv[1].type == tp_mat3x3) {
        c11_mat3x3* rhs = py_tomat3x3(&argv[1]);
        c11_mat3x3* out = py_newmat3x3(py_retval());
        matmul(lhs, rhs, out);
    } else if(argv[1].type == tp_vec3) {
        c11_vec3 rhs = py_tovec3(&argv[1]);
        c11_vec3 res;
        res.x = lhs->_11 * rhs.x + lhs->_12 * rhs.y + lhs->_13 * rhs.z;
        res.y = lhs->_21 * rhs.x + lhs->_22 * rhs.y + lhs->_23 * rhs.z;
        res.z = lhs->_31 * rhs.x + lhs->_32 * rhs.y + lhs->_33 * rhs.z;
        py_newvec3(py_retval(), res);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool mat3x3__invert__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    c11_mat3x3* out = py_newmat3x3(py_retval());
    if(inverse(ud, out)) return true;
    return ZeroDivisionError("matrix is not invertible");
}

static bool mat3x3_matmul(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_mat3x3);
    PY_CHECK_ARG_TYPE(1, tp_mat3x3);
    PY_CHECK_ARG_TYPE(2, tp_mat3x3);
    c11_mat3x3* lhs = py_tomat3x3(&argv[0]);
    c11_mat3x3* rhs = py_tomat3x3(&argv[1]);
    c11_mat3x3* out = py_tomat3x3(&argv[2]);
    matmul(lhs, rhs, out);
    py_newnone(py_retval());
    return true;
}

static bool mat3x3_determinant(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    py_newfloat(py_retval(), determinant(ud));
    return true;
}

static bool mat3x3_copy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    c11_mat3x3* out = py_newmat3x3(py_retval());
    *out = *ud;
    return true;
}

static bool mat3x3_inverse(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    c11_mat3x3* out = py_newmat3x3(py_retval());
    if(inverse(ud, out)) return true;
    return ZeroDivisionError("matrix is not invertible");
}

static bool mat3x3_copy_(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_mat3x3);
    c11_mat3x3* self = py_tomat3x3(argv);
    c11_mat3x3* other = py_tomat3x3(&argv[1]);
    *self = *other;
    py_newnone(py_retval());
    return true;
}

static bool mat3x3_inverse_(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    c11_mat3x3 res;
    if(inverse(ud, &res)) {
        *ud = res;
        py_newnone(py_retval());
        return true;
    }
    return ZeroDivisionError("matrix is not invertible");
}

static bool mat3x3_zeros_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    c11_mat3x3* out = py_newmat3x3(py_retval());
    memset(out, 0, sizeof(c11_mat3x3));
    return true;
}

static bool mat3x3_identity_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    c11_mat3x3* out = py_newmat3x3(py_retval());
    // clang-format off
    *out = (c11_mat3x3){
        ._11 = 1, ._12 = 0, ._13 = 0,
        ._21 = 0, ._22 = 1, ._23 = 0,
        ._31 = 0, ._32 = 0, ._33 = 1,
    };
    // clang-format on
    return true;
}

static bool mat3x3_trs_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    py_f64 r;
    if(!py_checktype(&argv[0], tp_vec2)) return false;
    if(!py_castfloat(&argv[1], &r)) return false;
    if(!py_checktype(&argv[2], tp_vec2)) return false;
    c11_vec2 t = py_tovec2(&argv[0]);
    c11_vec2 s = py_tovec2(&argv[2]);
    c11_mat3x3* out = py_newmat3x3(py_retval());
    trs(t, r, s, out);
    return true;
}

static bool mat3x3_copy_trs_(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    c11_mat3x3* ud = py_tomat3x3(&argv[0]);
    py_f64 r;
    if(!py_checktype(&argv[1], tp_vec2)) return false;
    if(!py_castfloat(&argv[2], &r)) return false;
    if(!py_checktype(&argv[3], tp_vec2)) return false;
    c11_vec2 t = py_tovec2(&argv[1]);
    c11_vec2 s = py_tovec2(&argv[3]);
    trs(t, r, s, ud);
    py_newnone(py_retval());
    return true;
}

static bool mat3x3_t(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    c11_vec2 res;
    res.x = ud->_13;
    res.y = ud->_23;
    py_newvec2(py_retval(), res);
    return true;
}

static bool mat3x3_r(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    float r = atan2f(ud->_21, ud->_11);
    py_newfloat(py_retval(), r);
    return true;
}

static bool mat3x3_s(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_mat3x3* ud = py_tomat3x3(argv);
    c11_vec2 res;
    res.x = sqrtf(ud->_11 * ud->_11 + ud->_21 * ud->_21);
    res.y = sqrtf(ud->_12 * ud->_12 + ud->_22 * ud->_22);
    py_newvec2(py_retval(), res);
    return true;
}

static bool mat3x3_transform_point(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2);
    c11_mat3x3* ud = py_tomat3x3(&argv[0]);
    c11_vec2 p = py_tovec2(&argv[1]);
    c11_vec2 res;
    res.x = ud->_11 * p.x + ud->_12 * p.y + ud->_13;
    res.y = ud->_21 * p.x + ud->_22 * p.y + ud->_23;
    py_newvec2(py_retval(), res);
    return true;
}

static bool mat3x3_transform_vector(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2);
    c11_mat3x3* ud = py_tomat3x3(&argv[0]);
    c11_vec2 p = py_tovec2(&argv[1]);
    c11_vec2 res;
    res.x = ud->_11 * p.x + ud->_12 * p.y;
    res.y = ud->_21 * p.x + ud->_22 * p.y;
    py_newvec2(py_retval(), res);
    return true;
}

/* vec2i */
DEFINE_VEC_FIELD(vec2i, int, py_i64, x)
DEFINE_VEC_FIELD(vec2i, int, py_i64, y)

static bool vec2i__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_vec2i data = py_tovec2i(argv);
    char buf[64];
    int size = snprintf(buf, 64, "vec2i(%d, %d)", data.x, data.y);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

/* vec3i */
static bool vec3i__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_vec3i data = py_tovec3i(argv);
    char buf[64];
    int size = snprintf(buf, 64, "vec3i(%d, %d, %d)", data.x, data.y, data.z);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

DEFINE_VEC_FIELD(vec3i, int, py_i64, x)
DEFINE_VEC_FIELD(vec3i, int, py_i64, y)
DEFINE_VEC_FIELD(vec3i, int, py_i64, z)

/* vec3 */
static bool vec3__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_vec3 data = py_tovec3(argv);
    char buf[64];
    int size = snprintf(buf, 64, "vec3(%.4f, %.4f, %.4f)", data.x, data.y, data.z);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

DEFINE_VEC_FIELD(vec3, float, py_f64, x)
DEFINE_VEC_FIELD(vec3, float, py_f64, y)
DEFINE_VEC_FIELD(vec3, float, py_f64, z)

static bool vec3__xy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_vec3 data = py_tovec3(argv);
    c11_vec2 res = {
        {data.x, data.y}
    };
    py_newvec2(py_retval(), res);
    return true;
}

static bool vec3__with_xy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2);
    c11_vec2 xy = py_tovec2(&argv[1]);
    c11_vec3 res = {
        {xy.x, xy.y, py_tovec3(argv).z}
    };
    py_newvec3(py_retval(), res);
    return true;
}

/* Color32 */
void py_newcolor32(py_OutRef out, c11_color32 color) {
    out->type = tp_color32;
    out->is_ptr = false;
    out->_color32 = color;
}

c11_color32 py_tocolor32(py_Ref obj) {
    assert(obj->type == tp_color32);
    return obj->_color32;
}

static bool color32__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(5);
    c11_color32 color;
    for(int i = 1; i < 5; i++) {
        PY_CHECK_ARG_TYPE(i, tp_int);
        py_i64 val = py_toint(&argv[i]);
        if(val < 0 || val > 255) return ValueError("color32 values must be between 0 and 255");
        color.data[i - 1] = (unsigned char)val;
    }
    py_newcolor32(py_retval(), color);
    return true;
}

#define DEFINE_COLOR32_FIELD(name)                                                                 \
    static bool color32__##name(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(1);                                                                          \
        c11_color32 color = py_tocolor32(argv);                                                    \
        py_newint(py_retval(), color.name);                                                        \
        return true;                                                                               \
    }                                                                                              \
    static bool color32_with_##name(int argc, py_Ref argv) {                                       \
        PY_CHECK_ARGC(2);                                                                          \
        c11_color32 color = py_tocolor32(argv);                                                    \
        PY_CHECK_ARG_TYPE(1, tp_int);                                                              \
        py_i64 val = py_toint(&argv[1]);                                                           \
        if(val < 0 || val > 255) {                                                                 \
            return ValueError("color32 values must be between 0 and 255");                         \
        }                                                                                          \
        color.name = (unsigned char)val;                                                           \
        py_newcolor32(py_retval(), color);                                                         \
        return true;                                                                               \
    }

DEFINE_COLOR32_FIELD(r)
DEFINE_COLOR32_FIELD(g)
DEFINE_COLOR32_FIELD(b)
DEFINE_COLOR32_FIELD(a)

#undef DEFINE_COLOR32_FIELD

static bool color32_from_hex_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    c11_sv hex = py_tosv(argv);
    c11_color32 color;
    int res;
    if(hex.size == 7) {
        res = sscanf(hex.data, "#%2hhx%2hhx%2hhx", &color.r, &color.g, &color.b);
        if(res != 3) return ValueError("invalid hex color format");
        color.a = 255;
    } else {
        res = sscanf(hex.data, "#%2hhx%2hhx%2hhx%2hhx", &color.r, &color.g, &color.b, &color.a);
        if(res != 4) return ValueError("invalid hex color format");
    }
    py_newcolor32(py_retval(), color);
    return true;
}

static bool color32_from_vec3_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_vec3);
    c11_vec3 v = py_tovec3(argv);
    c11_color32 color;
    color.r = (unsigned char)(v.x * 255);
    color.g = (unsigned char)(v.y * 255);
    color.b = (unsigned char)(v.z * 255);
    color.a = 255;
    py_newcolor32(py_retval(), color);
    return true;
}

static bool color32_from_vec3i_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_vec3i);
    c11_vec3i v = py_tovec3i(argv);
    c11_color32 color;
    color.r = (unsigned char)v.x;
    color.g = (unsigned char)v.y;
    color.b = (unsigned char)v.z;
    color.a = 255;
    py_newcolor32(py_retval(), color);
    return true;
}

static bool color32_to_hex(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_color32 color = py_tocolor32(argv);
    char buf[16];
    int size;
    if(color.a == 255) {
        size = snprintf(buf, sizeof(buf), "#%02x%02x%02x", color.r, color.g, color.b);
    } else {
        size = snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", color.r, color.g, color.b, color.a);
    }
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

static void c11_color32_premult(c11_color32* color) {
    if(color->a == 255) return;
    float alpha = color->a / 255.0f;
    color->r = (unsigned char)(color->r * alpha);
    color->g = (unsigned char)(color->g * alpha);
    color->b = (unsigned char)(color->b * alpha);
}

static bool color32_to_vec3(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_color32 color = py_tocolor32(argv);
    c11_color32_premult(&color);
    c11_vec3 v;
    v.x = (float)color.r / 255;
    v.y = (float)color.g / 255;
    v.z = (float)color.b / 255;
    py_newvec3(py_retval(), v);
    return true;
}

static bool color32_to_vec3i(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_color32 color = py_tocolor32(argv);
    c11_color32_premult(&color);
    c11_vec3i v;
    v.x = (int)color.r;
    v.y = (int)color.g;
    v.z = (int)color.b;
    py_newvec3i(py_retval(), v);
    return true;
}

static bool color32__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(argv[1].type != tp_color32) {
        py_newnotimplemented(py_retval());
        return true;
    }
    c11_color32 lhs = py_tocolor32(&argv[0]);
    c11_color32 rhs = py_tocolor32(&argv[1]);
    bool eq = memcmp(&lhs, &rhs, sizeof(c11_color32)) == 0;
    py_newbool(py_retval(), eq);
    return true;
}

static bool color32__ne__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(argv[1].type != tp_color32) {
        py_newnotimplemented(py_retval());
        return true;
    }
    c11_color32 lhs = py_tocolor32(&argv[0]);
    c11_color32 rhs = py_tocolor32(&argv[1]);
    bool eq = memcmp(&lhs, &rhs, sizeof(c11_color32)) != 0;
    py_newbool(py_retval(), eq);
    return true;
}

static bool color32__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_color32 color = py_tocolor32(argv);
    char buf[64];
    int size = snprintf(buf, 64, "color32(%d, %d, %d, %d)", color.r, color.g, color.b, color.a);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

static bool color32__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_color32 color = py_tocolor32(argv);
    uint32_t* color_int = (uint32_t*)&color;
    py_newint(py_retval(), *color_int);
    return true;
}

static bool color32_ansi_fg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_color32 color = py_tocolor32(argv);
    c11_color32_premult(&color);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_sv text = py_tosv(&argv[1]);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "\x1b[38;2;%d;%d;%dm%v\x1b[0m", color.r, color.g, color.b, text);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool color32_ansi_bg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_color32 color = py_tocolor32(argv);
    c11_color32_premult(&color);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_sv text = py_tosv(&argv[1]);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "\x1b[48;2;%d;%d;%dm%v\x1b[0m", color.r, color.g, color.b, text);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool vmath_rgb(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    c11_color32 color;
    color.r = (unsigned char)py_toint(&argv[0]);
    color.g = (unsigned char)py_toint(&argv[1]);
    color.b = (unsigned char)py_toint(&argv[2]);
    color.a = 255;
    py_newcolor32(py_retval(), color);
    return true;
}

static bool vmath_rgba(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_float);
    c11_color32 color;
    color.r = (unsigned char)py_toint(&argv[0]);
    color.g = (unsigned char)py_toint(&argv[1]);
    color.b = (unsigned char)py_toint(&argv[2]);
    color.a = (unsigned char)(py_tofloat(&argv[3]) * 255);
    py_newcolor32(py_retval(), color);
    return true;
}

static bool color32_alpha_blend_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(!py_checktype(&argv[0], tp_color32)) return false;
    if(py_isnone(&argv[1])) {
        py_assign(py_retval(), &argv[0]);
        return true;
    }
    if(!py_checktype(&argv[1], tp_color32)) return false;
    c11_color32 src = py_tocolor32(&argv[0]);
    c11_color32 dst = py_tocolor32(&argv[1]);
    float alpha = src.a / 255.0f;
    c11_color32 res;
    res.r = (unsigned char)(src.r * alpha + dst.r * (1 - alpha));
    res.g = (unsigned char)(src.g * alpha + dst.g * (1 - alpha));
    res.b = (unsigned char)(src.b * alpha + dst.b * (1 - alpha));
    res.a = (unsigned char)(src.a + dst.a * (1 - alpha));
    py_newcolor32(py_retval(), res);
    return true;
}

void pk__add_module_vmath() {
    py_Ref mod = py_newmodule("vmath");

    py_Type vec2 = pk_newtype("vec2", tp_object, mod, NULL, false, true);
    py_Type vec3 = pk_newtype("vec3", tp_object, mod, NULL, false, true);
    py_Type vec2i = pk_newtype("vec2i", tp_object, mod, NULL, false, true);
    py_Type vec3i = pk_newtype("vec3i", tp_object, mod, NULL, false, true);
    py_Type mat3x3 = pk_newtype("mat3x3", tp_object, mod, NULL, false, true);
    py_Type color32 = pk_newtype("color32", tp_object, mod, NULL, false, true);

    py_setdict(mod, py_name("vec2"), py_tpobject(vec2));
    py_setdict(mod, py_name("vec3"), py_tpobject(vec3));
    py_setdict(mod, py_name("vec2i"), py_tpobject(vec2i));
    py_setdict(mod, py_name("vec3i"), py_tpobject(vec3i));
    py_setdict(mod, py_name("mat3x3"), py_tpobject(mat3x3));
    py_setdict(mod, py_name("color32"), py_tpobject(color32));

    assert(vec2 == tp_vec2);
    assert(vec3 == tp_vec3);
    assert(vec2i == tp_vec2i);
    assert(vec3i == tp_vec3i);
    assert(mat3x3 == tp_mat3x3);
    assert(color32 == tp_color32);

    /* vec2 */
    py_bindmagic(vec2, __new__, vec2__new__);
    py_bindmagic(vec2, __add__, vec2__add__);
    py_bindmagic(vec2, __sub__, vec2__sub__);
    py_bindmagic(vec2, __mul__, vec2__mul__);
    py_bindmagic(vec2, __truediv__, vec2__truediv__);
    py_bindmagic(vec2, __repr__, vec2__repr__);
    py_bindmagic(vec2, __eq__, vec2__eq__);
    py_bindmagic(vec2, __ne__, vec2__ne__);
    py_bindmethod(vec2, "dot", vec2_dot);
    py_bindmethod(vec2, "length", vec2_length);
    py_bindmethod(vec2, "length_squared", vec2_length_squared);
    py_bindmethod(vec2, "normalize", vec2_normalize);
    py_bindmethod(vec2, "rotate", vec2_rotate);

    // clang-format off
    py_newvec2(_const(vec2, "ZERO"), (c11_vec2){{0, 0}});
    py_newvec2(_const(vec2, "ONE"), (c11_vec2){{1, 1}});
    py_newvec2(_const(vec2, "LEFT"), (c11_vec2){{-1, 0}});
    py_newvec2(_const(vec2, "RIGHT"), (c11_vec2){{1, 0}});
    py_newvec2(_const(vec2, "UP"), (c11_vec2){{0, -1}});
    py_newvec2(_const(vec2, "DOWN"), (c11_vec2){{0, 1}});
    // clang-format on

    py_bindstaticmethod(vec2, "angle", vec2_angle_STATIC);
    py_bindstaticmethod(vec2, "smooth_damp", vec2_smoothdamp_STATIC);

    py_bindproperty(vec2, "x", vec2__x, NULL);
    py_bindproperty(vec2, "y", vec2__y, NULL);
    py_bindmethod(vec2, "with_x", vec2__with_x);
    py_bindmethod(vec2, "with_y", vec2__with_y);
    py_bindmethod(vec2, "with_z", vec2__with_z);

    /* mat3x3 */
    py_bindmagic(mat3x3, __new__, mat3x3__new__);
    py_bindmagic(mat3x3, __repr__, mat3x3__repr__);
    py_bindmagic(mat3x3, __getitem__, mat3x3__getitem__);
    py_bindmagic(mat3x3, __setitem__, mat3x3__setitem__);
    py_bindmagic(mat3x3, __matmul__, mat3x3__matmul__);
    py_bindmagic(mat3x3, __invert__, mat3x3__invert__);
    py_bindmagic(mat3x3, __eq__, mat3x3__eq__);
    py_bindmagic(mat3x3, __ne__, mat3x3__ne__);
    py_bindmethod(mat3x3, "matmul", mat3x3_matmul);
    py_bindmethod(mat3x3, "determinant", mat3x3_determinant);
    py_bindmethod(mat3x3, "copy", mat3x3_copy);
    py_bindmethod(mat3x3, "inverse", mat3x3_inverse);
    py_bindmethod(mat3x3, "copy_", mat3x3_copy_);
    py_bindmethod(mat3x3, "inverse_", mat3x3_inverse_);
    py_bindstaticmethod(mat3x3, "zeros", mat3x3_zeros_STATIC);
    py_bindstaticmethod(mat3x3, "identity", mat3x3_identity_STATIC);
    py_bindstaticmethod(mat3x3, "trs", mat3x3_trs_STATIC);
    py_bindmethod(mat3x3, "copy_trs_", mat3x3_copy_trs_);
    py_bindmethod(mat3x3, "t", mat3x3_t);
    py_bindmethod(mat3x3, "r", mat3x3_r);
    py_bindmethod(mat3x3, "s", mat3x3_s);
    py_bindmethod(mat3x3, "transform_point", mat3x3_transform_point);
    py_bindmethod(mat3x3, "transform_vector", mat3x3_transform_vector);

    /* vec2i */
    py_bindmagic(vec2i, __new__, vec2i__new__);
    py_bindmagic(vec2i, __repr__, vec2i__repr__);
    py_bindmagic(vec2i, __add__, vec2i__add__);
    py_bindmagic(vec2i, __sub__, vec2i__sub__);
    py_bindmagic(vec2i, __mul__, vec2i__mul__);
    py_bindmagic(vec2i, __floordiv__, vec2i__floordiv__);
    py_bindmagic(vec2i, __eq__, vec2i__eq__);
    py_bindmagic(vec2i, __ne__, vec2i__ne__);
    py_bindmagic(vec2i, __hash__, vec2i__hash__);
    py_bindproperty(vec2i, "x", vec2i__x, NULL);
    py_bindproperty(vec2i, "y", vec2i__y, NULL);
    py_bindmethod(vec2i, "with_x", vec2i__with_x);
    py_bindmethod(vec2i, "with_y", vec2i__with_y);
    py_bindmethod(vec2i, "dot", vec2i_dot);

    // clang-format off
    py_newvec2i(_const(vec2i, "ZERO"), (c11_vec2i){{0, 0}});
    py_newvec2i(_const(vec2i, "ONE"), (c11_vec2i){{1, 1}});
    py_newvec2i(_const(vec2i, "LEFT"), (c11_vec2i){{-1, 0}});
    py_newvec2i(_const(vec2i, "RIGHT"), (c11_vec2i){{1, 0}});
    py_newvec2i(_const(vec2i, "UP"), (c11_vec2i){{0, -1}});
    py_newvec2i(_const(vec2i, "DOWN"), (c11_vec2i){{0, 1}});
    // clang-format on

    /* vec3i */
    py_bindmagic(vec3i, __new__, vec3i__new__);
    py_bindmagic(vec3i, __repr__, vec3i__repr__);
    py_bindmagic(vec3i, __add__, vec3i__add__);
    py_bindmagic(vec3i, __sub__, vec3i__sub__);
    py_bindmagic(vec3i, __mul__, vec3i__mul__);
    py_bindmagic(vec3i, __floordiv__, vec3i__floordiv__);
    py_bindmagic(vec3i, __eq__, vec3i__eq__);
    py_bindmagic(vec3i, __ne__, vec3i__ne__);
    py_bindmagic(vec3i, __hash__, vec3i__hash__);
    py_bindproperty(vec3i, "x", vec3i__x, NULL);
    py_bindproperty(vec3i, "y", vec3i__y, NULL);
    py_bindproperty(vec3i, "z", vec3i__z, NULL);
    py_bindmethod(vec3i, "with_x", vec3i__with_x);
    py_bindmethod(vec3i, "with_y", vec3i__with_y);
    py_bindmethod(vec3i, "with_z", vec3i__with_z);
    py_bindmethod(vec3i, "dot", vec3i_dot);

    py_newvec3i(_const(vec3i, "ZERO"),
                (c11_vec3i){
                    {0, 0, 0}
    });
    py_newvec3i(_const(vec3i, "ONE"),
                (c11_vec3i){
                    {1, 1, 1}
    });

    /* vec3 */
    py_bindmagic(vec3, __new__, vec3__new__);
    py_bindmagic(vec3, __add__, vec3__add__);
    py_bindmagic(vec3, __sub__, vec3__sub__);
    py_bindmagic(vec3, __mul__, vec3__mul__);
    py_bindmagic(vec3, __truediv__, vec3__truediv__);
    py_bindmagic(vec3, __repr__, vec3__repr__);
    py_bindmagic(vec3, __eq__, vec3__eq__);
    py_bindmagic(vec3, __ne__, vec3__ne__);
    py_bindmethod(vec3, "dot", vec3_dot);
    py_bindmethod(vec3, "length", vec3_length);
    py_bindmethod(vec3, "length_squared", vec3_length_squared);
    py_bindmethod(vec3, "normalize", vec3_normalize);
    py_bindproperty(vec3, "x", vec3__x, NULL);
    py_bindproperty(vec3, "y", vec3__y, NULL);
    py_bindproperty(vec3, "z", vec3__z, NULL);
    py_bindproperty(vec3, "xy", vec3__xy, NULL);
    py_bindmethod(vec3, "with_x", vec3__with_x);
    py_bindmethod(vec3, "with_y", vec3__with_y);
    py_bindmethod(vec3, "with_z", vec3__with_z);
    py_bindmethod(vec3, "with_xy", vec3__with_xy);

    py_newvec3(_const(vec3, "ZERO"),
               (c11_vec3){
                   {0, 0, 0}
    });
    py_newvec3(_const(vec3, "ONE"),
               (c11_vec3){
                   {1, 1, 1}
    });

    /* color32 */
    py_bindmagic(color32, __new__, color32__new__);
    py_bindmagic(color32, __repr__, color32__repr__);
    py_bindmagic(color32, __eq__, color32__eq__);
    py_bindmagic(color32, __ne__, color32__ne__);
    py_bindmagic(color32, __hash__, color32__hash__);
    py_bindproperty(color32, "r", color32__r, NULL);
    py_bindproperty(color32, "g", color32__g, NULL);
    py_bindproperty(color32, "b", color32__b, NULL);
    py_bindproperty(color32, "a", color32__a, NULL);
    py_bindmethod(color32, "with_r", color32_with_r);
    py_bindmethod(color32, "with_g", color32_with_g);
    py_bindmethod(color32, "with_b", color32_with_b);
    py_bindmethod(color32, "with_a", color32_with_a);
    py_bindstaticmethod(color32, "from_hex", color32_from_hex_STATIC);
    py_bindstaticmethod(color32, "from_vec3", color32_from_vec3_STATIC);
    py_bindstaticmethod(color32, "from_vec3i", color32_from_vec3i_STATIC);
    py_bindmethod(color32, "to_hex", color32_to_hex);
    py_bindmethod(color32, "to_vec3", color32_to_vec3);
    py_bindmethod(color32, "to_vec3i", color32_to_vec3i);
    py_bindmethod(color32, "ansi_fg", color32_ansi_fg);
    py_bindmethod(color32, "ansi_bg", color32_ansi_bg);
    py_bindfunc(mod, "rgb", vmath_rgb);
    py_bindfunc(mod, "rgba", vmath_rgba);
    py_bindstaticmethod(color32, "alpha_blend", color32_alpha_blend_STATIC);
}

#undef DEFINE_VEC_FIELD
#undef DEFINE_BOOL_NE
#undef DEF_VECTOR_ELEMENT_WISE
#undef DEF_VECTOR_OPS
#undef DEF_VECTOR_INT_OPS