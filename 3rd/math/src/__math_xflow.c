#include "math.h"

double __math_xflow(unsigned int sign, double y)
{
	return eval_as_double(fp_barrier(sign ? -y : y) * y);
}
