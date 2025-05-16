#include "math.h"

float __math_divzerof(unsigned int sign)
{
	return fp_barrierf(sign ? -1.0f : 1.0f) / 0.0f;
}
