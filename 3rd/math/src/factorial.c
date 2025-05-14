#include <math.h>

int factorial(int n)
{
	if (n < 0) return (int)__math_invalid(-1.0f);
	int32_t r = 1;
	for (int32_t i = 2; i <= n; i++)
	{
		r = r * i;
	}
	return r;
}