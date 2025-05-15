#include <math.h>

double fabs(double x)
{
	union {double f; unsigned long long i;} u = {x};
	u.i &= ULLONG_NSHIFT/2;
	return u.f;
}
