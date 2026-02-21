#pragma once

#define DMATH_INFINITY ((float)((1e+300 * 1e+300)))
#define DMATH_NAN ((float)(DMATH_INFINITY * 0.0F))
#define DMATH_PI 3.1415926535897932384
#define DMATH_E 2.7182818284590452354
#define DMATH_DEG2RAD 0.017453292519943295
#define DMATH_RAD2DEG 57.29577951308232
#define DMATH_EPSILON 1e-10
#define DMATH_LOG2_E 1.4426950408889634

double dmath_exp2(double x);
double dmath_log2(double x);

double dmath_exp(double x);
double dmath_exp10(double x);
double dmath_log(double x);
double dmath_log10(double x);
double dmath_pow(double base, double exp);
double dmath_sqrt(double x);
double dmath_cbrt(double x);

void dmath_sincos(double x, double* sin, double* cos);
double dmath_sin(double x);
double dmath_cos(double x);
double dmath_tan(double x);
double dmath_asin(double x);
double dmath_acos(double x);
double dmath_atan(double x);
double dmath_atan2(double y, double x);

int dmath_isinf(double x);
int dmath_isnan(double x);
int dmath_isnormal(double x);
int dmath_isfinite(double x);

double dmath_fmod(double x, double y);
double dmath_copysign(double x, double y);

double dmath_fabs(double x);
double dmath_ceil(double x);
double dmath_floor(double x);
double dmath_trunc(double x);
double dmath_modf(double x, double* intpart);

double dmath_fmin(double x, double y);
double dmath_fmax(double x, double y);
