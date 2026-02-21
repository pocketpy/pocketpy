#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/dmath.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

// https://bottosson.github.io/posts/gamutclipping/#oklab-to-linear-srgb-conversion

// clang-format off
static c11_vec3 linear_srgb_to_oklab(c11_vec3 c)
{
	double l = 0.4122214708 * c.x + 0.5363325363 * c.y + 0.0514459929 * c.z;
	double m = 0.2119034982 * c.x + 0.6806995451 * c.y + 0.1073969566 * c.z;
	double s = 0.0883024619 * c.x + 0.2817188376 * c.y + 0.6299787005 * c.z;

	double l_ = dmath_cbrt(l);
	double m_ = dmath_cbrt(m);
	double s_ = dmath_cbrt(s);

	return (c11_vec3){{
		0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
		1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
		0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_,
	}};
}

static c11_vec3 oklab_to_linear_srgb(c11_vec3 c)
{
    double l_ = c.x + 0.3963377774 * c.y + 0.2158037573 * c.z;
    double m_ = c.x - 0.1055613458 * c.y - 0.0638541728 * c.z;
    double s_ = c.x - 0.0894841775 * c.y - 1.2914855480 * c.z;

    double l = l_ * l_ * l_;
    double m = m_ * m_ * m_;
    double s = s_ * s_ * s_;

    return (c11_vec3){{
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    }};
}

// clang-format on

static double _gamma_correct_inv(double x) {
    return (x <= 0.04045) ? (x / 12.92) : dmath_pow((x + 0.055) / 1.055, 2.4);
}

static double _gamma_correct(double x) {
    return (x <= 0.0031308) ? (12.92 * x) : (1.055 * dmath_pow(x, 1.0 / 2.4) - 0.055);
}

static c11_vec3 srgb_to_linear_srgb(c11_vec3 c) {
    c.x = _gamma_correct_inv(c.x);
    c.y = _gamma_correct_inv(c.y);
    c.z = _gamma_correct_inv(c.z);
    return c;
}

static c11_vec3 linear_srgb_to_srgb(c11_vec3 c) {
    c.x = _gamma_correct(c.x);
    c.y = _gamma_correct(c.y);
    c.z = _gamma_correct(c.z);
    return c;
}

static c11_vec3 _oklab_to_oklch(c11_vec3 c) {
    c11_vec3 res;
    res.x = c.x;
    res.y = dmath_sqrt(c.y * c.y + c.z * c.z);
    res.z = dmath_fmod(dmath_atan2(c.z, c.y), 2 * DMATH_PI);
    res.z = res.z * DMATH_RAD2DEG;
    return res;
}

static c11_vec3 _oklch_to_oklab(c11_vec3 c) {
    c11_vec3 res;
    res.x = c.x;
    res.y = c.y * dmath_cos(c.z * DMATH_DEG2RAD);
    res.z = c.y * dmath_sin(c.z * DMATH_DEG2RAD);
    return res;
}

static c11_vec3 linear_srgb_to_oklch(c11_vec3 c) {
    return _oklab_to_oklch(linear_srgb_to_oklab(c));
}

static bool _is_valid_srgb(c11_vec3 c) {
    return c.x >= 0.0f && c.x <= 1.0f && c.y >= 0.0f && c.y <= 1.0f && c.z >= 0.0f && c.z <= 1.0f;
}

static c11_vec3 oklch_to_linear_srgb(c11_vec3 c) {
    c11_vec3 candidate = oklab_to_linear_srgb(_oklch_to_oklab(c));
    if(_is_valid_srgb(candidate)) return candidate;

    // try with chroma = 0
    c11_vec3 clamped = {
        {c.x, 0.0f, c.z}
    };

    // if not even chroma = 0 is displayable
    // fall back to RGB clamping
    candidate = oklab_to_linear_srgb(_oklch_to_oklab(clamped));
    if(!_is_valid_srgb(candidate)) {
        candidate.x = dmath_fmax(0.0, dmath_fmin(1.0, candidate.x));
        candidate.y = dmath_fmax(0.0, dmath_fmin(1.0, candidate.y));
        candidate.z = dmath_fmax(0.0, dmath_fmin(1.0, candidate.z));
        return candidate;
    }

    // By this time we know chroma = 0 is displayable and our current chroma is not.
    // Find the displayable chroma through the bisection method.
    float start = 0.0f;
    float end = c.y;
    float range[2] = {0.0f, 0.4f};
    float resolution = (range[1] - range[0]) / dmath_pow(2, 13);
    float _last_good_c = clamped.y;

    while(end - start > resolution) {
        clamped.y = start + (end - start) * 0.5f;
        candidate = oklab_to_linear_srgb(_oklch_to_oklab(clamped));
        if(_is_valid_srgb(candidate)) {
            _last_good_c = clamped.y;
            start = clamped.y;
        } else {
            end = clamped.y;
        }
    }

    candidate = oklab_to_linear_srgb(_oklch_to_oklab(clamped));
    if(_is_valid_srgb(candidate)) return candidate;
    clamped.y = _last_good_c;
    return oklab_to_linear_srgb(_oklch_to_oklab(clamped));
}

// https://github.com/python/cpython/blob/3.13/Lib/colorsys.py
static c11_vec3 srgb_to_hsv(c11_vec3 c) {
    float r = c.x;
    float g = c.y;
    float b = c.z;

    float maxc = dmath_fmax(r, dmath_fmax(g, b));
    float minc = dmath_fmin(r, dmath_fmin(g, b));
    float v = maxc;
    if(minc == maxc) {
        return (c11_vec3){
            {0.0f, 0.0f, v}
        };
    }

    float s = (maxc - minc) / maxc;
    float rc = (maxc - r) / (maxc - minc);
    float gc = (maxc - g) / (maxc - minc);
    float bc = (maxc - b) / (maxc - minc);
    float h;
    if(r == maxc) {
        h = bc - gc;
    } else if(g == maxc) {
        h = 2.0f + rc - bc;
    } else {
        h = 4.0f + gc - rc;
    }
    h = dmath_fmod(h / 6.0, 1.0);
    return (c11_vec3){
        {h, s, v}
    };
}

static c11_vec3 hsv_to_srgb(c11_vec3 c) {
    float h = c.x;
    float s = c.y;
    float v = c.z;

    if(s == 0.0f) {
        return (c11_vec3){
            {v, v, v}
        };
    }

    int i = (int)(h * 6.0f);
    float f = (h * 6.0f) - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));
    i = i % 6;
    switch(i) {
        // clang-format off
        case 0: return (c11_vec3){{v, t, p}};
        case 1: return (c11_vec3){{q, v, p}};
        case 2: return (c11_vec3){{p, v, t}};
        case 3: return (c11_vec3){{p, q, v}};
        case 4: return (c11_vec3){{t, p, v}};
        case 5: return (c11_vec3){{v, p, q}};
        // clang-format on
        default: c11__unreachable();
    }
}

#define DEF_VEC3_WRAPPER(F)                                                                        \
    static bool colorcvt_##F(int argc, py_Ref argv);                                               \
    static bool colorcvt_##F(int argc, py_Ref argv) {                                              \
        PY_CHECK_ARGC(1);                                                                          \
        PY_CHECK_ARG_TYPE(0, tp_vec3);                                                             \
        c11_vec3 c = py_tovec3(argv);                                                              \
        py_newvec3(py_retval(), F(c));                                                             \
        return true;                                                                               \
    }

DEF_VEC3_WRAPPER(linear_srgb_to_srgb)
DEF_VEC3_WRAPPER(srgb_to_linear_srgb)
DEF_VEC3_WRAPPER(srgb_to_hsv)
DEF_VEC3_WRAPPER(hsv_to_srgb)
DEF_VEC3_WRAPPER(oklch_to_linear_srgb)
DEF_VEC3_WRAPPER(linear_srgb_to_oklch)

void pk__add_module_colorcvt() {
    py_Ref mod = py_newmodule("colorcvt");

    py_bindfunc(mod, "linear_srgb_to_srgb", colorcvt_linear_srgb_to_srgb);
    py_bindfunc(mod, "srgb_to_linear_srgb", colorcvt_srgb_to_linear_srgb);
    py_bindfunc(mod, "srgb_to_hsv", colorcvt_srgb_to_hsv);
    py_bindfunc(mod, "hsv_to_srgb", colorcvt_hsv_to_srgb);
    py_bindfunc(mod, "oklch_to_linear_srgb", colorcvt_oklch_to_linear_srgb);
    py_bindfunc(mod, "linear_srgb_to_oklch", colorcvt_linear_srgb_to_oklch);
}

#undef DEF_VEC3_WRAPPER