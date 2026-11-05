#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "determinant_cpu.h"
#include "determinant_gpu.h"
#include "matrix_util.h"
#include "cl_util.h"

static const int SIZES[] = { 32, 64, 128, 256, 384, 512, 768, 1024 };
#define N_SIZES (int)(sizeof(SIZES) / sizeof(SIZES[0]))

int main(void)
{
    CLEnv env;
    char *kernel_src;
    FILE *csv;
    int i;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (!cl_env_init(&env)) {
        fprintf(stderr, "Не удалось инициализировать OpenCL\n");
        return 1;
    }

    kernel_src = read_text_file("src/determinant.cl");
    if (!kernel_src) {
        fprintf(stderr, "Не найден src/determinant.cl (запускать из корня проекта)\n");
        return 1;
    }

    csv = fopen("bin/det_results.csv", "w");
    if (!csv) {
        fprintf(stderr, "Не удалось открыть bin/det_results.csv на запись\n");
        return 1;
    }
    fprintf(csv, "n,cpu_ms,gpu_ms,speedup,max_rel_diff\n");

    printf("%6s %12s %12s %10s %14s\n", "n", "CPU, мс", "GPU, мс", "ускорение", "max |rel.diff|");

    for (i = 0; i < N_SIZES; i++) {
        int n = SIZES[i];
        double *a0 = (double *)malloc(sizeof(double) * (size_t)n * (size_t)n);
        double *a_cpu = (double *)malloc(sizeof(double) * (size_t)n * (size_t)n);
        double *a_gpu = (double *)malloc(sizeof(double) * (size_t)n * (size_t)n);
        clock_t t0, t1;
        double cpu_ms, gpu_ms, det_cpu, det_gpu, rel_diff;

        matrix_fill_random(a0, n, 12345u + (unsigned)i);

        matrix_copy(a_cpu, a0, n);
        t0 = clock();
        det_cpu = determinant_cpu(a_cpu, n);
        t1 = clock();
        cpu_ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;

        matrix_copy(a_gpu, a0, n);
        t0 = clock();
        det_gpu = determinant_gpu(&env, kernel_src, a_gpu, n);
        t1 = clock();
        gpu_ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;

        rel_diff = fabs(det_cpu - det_gpu) / (fabs(det_cpu) > 1e-9 ? fabs(det_cpu) : 1.0);

        printf("%6d %12.3f %12.3f %10.2f %14.2e\n",
               n, cpu_ms, gpu_ms, gpu_ms > 0 ? cpu_ms / gpu_ms : 0.0, rel_diff);
        fprintf(csv, "%d,%.4f,%.4f,%.4f,%.6e\n",
                n, cpu_ms, gpu_ms, gpu_ms > 0 ? cpu_ms / gpu_ms : 0.0, rel_diff);

        free(a0);
        free(a_cpu);
        free(a_gpu);
    }

    fclose(csv);
    free(kernel_src);
    cl_env_destroy(&env);
    return 0;
}
