#ifndef DETERMINANT_GPU_H
#define DETERMINANT_GPU_H

#include "cl_util.h"

// то же самое, но на GPU (determinant.cl)
double determinant_gpu(CLEnv *env, const char *kernel_src, double *a, int n);

#endif /* DETERMINANT_GPU_H */
