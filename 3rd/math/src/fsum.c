#include <math.h>

double fsum(double* aptr, int n)
{

	/*Kahan sum for float addition*/
	double sum = 0, C = 0, Y, T;
	for (int i = 0; i < n; i++)
	{
		Y = aptr[i] - C;
		T = sum + Y;
		C = T - sum - Y;
		sum = Y;
	}
	return sum;
}