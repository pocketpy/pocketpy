#include "math.h"

double __math_oflow(unsigned int sign)
{
	return __math_xflow(sign, 0x1p769);
}
