#include "math.h"

float __math_oflowf(unsigned int sign)
{
	return __math_xflowf(sign, 0x1p97f);
}
