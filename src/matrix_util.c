#include "matrix_util.h"
#include <stdlib.h>
#include <math.h>

static double rand_uniform(double lo, double hi)
{
    return lo + (hi - lo) * ((double)rand() / (double)RAND_MAX);
}

// диагональ ~1, внедиагональные элементы масштabированы так, чтобы их сумма
// была заведомо меньше диагонали - без этого Гаусс без pivoting плывёт,
// а определитель переполняется на больших n
void matrix_fill_random(double *a, int n, unsigned seed)
{
    int i, j;
    double *off = (double *)malloc(sizeof(double) * (size_t)n);

    srand(seed);

    for (i = 0; i < n; i++) {
        double diag = rand_uniform(0.99, 1.01);
        double raw_sum = 0.0;
        double scale;

        for (j = 0; j < n; j++) {
            off[j] = (j == i) ? 0.0 : rand_uniform(-1.0, 1.0);
            raw_sum += fabs(off[j]);
        }

        scale = (raw_sum > 1e-12) ? (0.3 * diag) / raw_sum : 0.0;

        for (j = 0; j < n; j++)
            a[i * n + j] = (j == i) ? diag : off[j] * scale;
    }

    free(off);
}

void matrix_copy(double *dst, const double *src, int n)
{
    int i;
    for (i = 0; i < n * n; i++)
        dst[i] = src[i];
}
