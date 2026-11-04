#include "determinant_cpu.h"
#include <math.h>

double determinant_cpu(double *a, int n)
{
    double det = 1.0;
    int col, row, k;

    for (col = 0; col < n; col++) {
        double pivot = a[col * n + col];

        if (fabs(pivot) < 1e-12)
            return 0.0;

        det *= pivot;

        // строки ниже диагонали независимы - это и уходит на GPU
        for (row = col + 1; row < n; row++) {
            double factor = a[row * n + col] / pivot;
            for (k = col; k < n; k++)
                a[row * n + k] -= factor * a[col * n + k];
        }
    }

    return det;
}
