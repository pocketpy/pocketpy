#include "math.h"

// FIXME: macro in math.h
int __signbit(double x)
{
	union {
		double d;
		unsigned long long i;
	} y = { x };
	return y.i>>63;
}


