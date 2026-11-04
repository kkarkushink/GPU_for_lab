#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "determinant_cpu.h"
#include "determinant_gpu.h"
#include "matrix_util.h"
#include "cl_util.h"

int main(int argc, char **argv)
{
    int n = (argc > 1) ? atoi(argv[1]) : 8;
    CLEnv env;
    char *kernel_src;
    double *a0, *a_cpu, *a_gpu;
    double det_cpu, det_gpu;
    clock_t t0, t1;

    if (n < 2) {
        printf("Использование: det_main.exe [размер матрицы N]\n");
        return 1;
    }

    if (!cl_env_init(&env)) {
        fprintf(stderr, "Не удалось инициализировать OpenCL\n");
        return 1;
    }
    kernel_src = read_text_file("src/determinant.cl");
    if (!kernel_src) {
        fprintf(stderr, "Не найден src/determinant.cl (запускать из корня проекта)\n");
        return 1;
    }

    a0 = (double *)malloc(sizeof(double) * (size_t)n * (size_t)n);
    a_cpu = (double *)malloc(sizeof(double) * (size_t)n * (size_t)n);
    a_gpu = (double *)malloc(sizeof(double) * (size_t)n * (size_t)n);

    matrix_fill_random(a0, n, (unsigned)time(NULL));
    matrix_copy(a_cpu, a0, n);
    matrix_copy(a_gpu, a0, n);

    t0 = clock();
    det_cpu = determinant_cpu(a_cpu, n);
    t1 = clock();
    printf("CPU (double): det = %.6f, время = %.3f мс\n",
           det_cpu, 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC);

    t0 = clock();
    det_gpu = determinant_gpu(&env, kernel_src, a_gpu, n);
    t1 = clock();
    printf("GPU (float):  det = %.6f, время = %.3f мс\n",
           det_gpu, 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC);

    free(a0);
    free(a_cpu);
    free(a_gpu);
    free(kernel_src);
    cl_env_destroy(&env);
    return 0;
}
