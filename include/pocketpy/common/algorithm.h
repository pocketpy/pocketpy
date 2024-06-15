#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void *c11__lower_bound(const void *key, const void *ptr, int count, int size,
                       bool (*less)(const void *, const void *));

int *c11__lower_bound_int(int key, const int *ptr, int count);
double *c11__lower_bound_double(double key, const double *ptr, int count);

#ifdef __cplusplus
}

#endif