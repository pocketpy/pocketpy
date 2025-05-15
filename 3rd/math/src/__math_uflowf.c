#include "math.h"

float __math_uflowf(unsigned int sign)
{
	return __math_xflowf(sign, 0x1p-95f);
}
