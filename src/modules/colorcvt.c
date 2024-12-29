#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include <math.h>

// https://bottosson.github.io/posts/gamutclipping/#oklab-to-linear-srgb-conversion

// clang-format off
static c11_vec3 linear_srgb_to_oklab(c11_vec3 c)
{
	float l = 0.4122214708f * c.x + 0.5363325363f * c.y + 0.0514459929f * c.z;
	float m = 0.2119034982f * c.x + 0.6806995451f * c.y + 0.1073969566f * c.z;
	float s = 0.0883024619f * c.x + 0.2817188376f * c.y + 0.6299787005f * c.z;

	float l_ = cbrtf(l);
	float m_ = cbrtf(m);
	float s_ = cbrtf(s);

	return (c11_vec3){{
		0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
		1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
		0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
	}};
}

static c11_vec3 oklab_to_linear_srgb(c11_vec3 c)
{
    float l_ = c.x + 0.3963377774f * c.y + 0.2158037573f * c.z;
    float m_ = c.x - 0.1055613458f * c.y - 0.0638541728f * c.z;
    float s_ = c.x - 0.0894841775f * c.y - 1.2914855480f * c.z;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    return (c11_vec3){{
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    }};
}

// clang-format on

static float _gamma_correct_inv(float x) {
    return (x <= 0.04045f) ? (x / 12.92f) : powf((x + 0.055f) / 1.055f, 2.4f);
}

static float _gamma_correct(float x) {
    return (x <= 0.0031308f) ? (12.92f * x) : (1.055f * powf(x, 1.0f / 2.4f) - 0.055f);
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
    res.y = sqrtf(c.y * c.y + c.z * c.z);
    res.z = fmodf(atan2f(c.z, c.y), 2 * (float)PK_M_PI);
    res.z = res.z * PK_M_RAD2DEG;
    return res;
}

static c11_vec3 _oklch_to_oklab(c11_vec3 c) {
    c11_vec3 res;
    res.x = c.x;
    res.y = c.y * cosf(c.z * PK_M_DEG2RAD);
    res.z = c.y * sinf(c.z * PK_M_DEG2RAD);
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
        candidate.x = fmaxf(0.0f, fminf(1.0f, candidate.x));
        candidate.y = fmaxf(0.0f, fminf(1.0f, candidate.y));
        candidate.z = fmaxf(0.0f, fminf(1.0f, candidate.z));
        return candidate;
    }

    // By this time we know chroma = 0 is displayable and our current chroma is not.
    // Find the displayable chroma through the bisection method.
    float start = 0.0f;
    float end = c.y;
    float range[2] = {0.0f, 0.4f};
    float resolution = (range[1] - range[0]) / powf(2, 13);
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

    float maxc = fmaxf(r, fmaxf(g, b));
    float minc = fminf(r, fminf(g, b));
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
    h = fmodf(h / 6.0f, 1.0f);
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