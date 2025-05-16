#include "math.h"

float __math_xflowf(unsigned int sign, float y)
{
	return eval_as_float(fp_barrierf(sign ? -y : y) * y);
}
