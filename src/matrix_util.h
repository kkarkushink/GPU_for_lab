#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <stddef.h>

void matrix_fill_random(double *a, int n, unsigned seed);
void matrix_copy(double *dst, const double *src, int n);

#endif /* MATRIX_UTIL_H */
