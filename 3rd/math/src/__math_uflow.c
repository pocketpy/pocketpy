#include "math.h"

double __math_uflow(unsigned int sign)
{
	return __math_xflow(sign, 0x1p-767);
}
