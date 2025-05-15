#include "math.h"

double __math_divzero(unsigned int sign)
{
	return fp_barrier(sign ? -1.0 : 1.0) / 0.0;
}
