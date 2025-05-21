#include "libm.h"

double degrees(double x)
{
	if (isinf(x) || isnan(x)) return x;
	double r = x * M_RAD2DEG;
	return r;
}