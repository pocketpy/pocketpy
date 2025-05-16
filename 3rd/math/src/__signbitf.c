#include "math.h"

// FIXME: macro in math.h
int __signbitf(float x)
{
	union {
		float f;
		unsigned int i;
	} y = { x };
	return y.i>>31;
}
