#include <math.h>

double radians(double x)
{
	if (isinf(x) || isnan(x)) return x;
	double r = x * M_DEG2RAD;
	return r;
}