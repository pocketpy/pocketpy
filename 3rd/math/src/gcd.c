#include <math.h>

int gcd(int a, int b)
{
	if (a < 0) a = -a;
	if (b < 0) b = -b;
	while (b != 0)
	{
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}