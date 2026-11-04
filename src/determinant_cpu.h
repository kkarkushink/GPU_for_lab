#ifndef DETERMINANT_CPU_H
#define DETERMINANT_CPU_H

// определитель n x n методом Гаусса, a портится после вызова (передавать копию)
double determinant_cpu(double *a, int n);

#endif /* DETERMINANT_CPU_H */
