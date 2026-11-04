#include "determinant_gpu.h"
#include <stdio.h>
#include <stdlib.h>

#define LOCAL_SIZE 64

// буфер на GPU в float (double не поддерживается), снаружи всё равно double
double determinant_gpu(CLEnv *env, const char *kernel_src, double *a, int n)
{
    cl_int err;
    cl_program program;
    cl_kernel kernel;
    cl_mem buf;
    double det = 1.0;
    int col, i;
    float *a32 = (float *)malloc(sizeof(float) * (size_t)n * (size_t)n);

    for (i = 0; i < n * n; i++)
        a32[i] = (float)a[i];

    program = cl_build_program(env, kernel_src);
    if (!program) {
        free(a32);
        return 0.0;
    }

    kernel = clCreateKernel(program, "eliminate_rows", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clCreateKernel failed: %d\n", err);
        clReleaseProgram(program);
        free(a32);
        return 0.0;
    }

    buf = clCreateBuffer(env->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                          sizeof(float) * (size_t)n * (size_t)n, a32, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clCreateBuffer failed: %d\n", err);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        free(a32);
        return 0.0;
    }

    for (col = 0; col < n - 1; col++) {
        int count = n - col - 1;
        size_t local_size = LOCAL_SIZE;
        size_t global_size = ((size_t)(count + LOCAL_SIZE - 1) / LOCAL_SIZE) * LOCAL_SIZE;

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
        clSetKernelArg(kernel, 1, sizeof(int), &n);
        clSetKernelArg(kernel, 2, sizeof(int), &col);
        clSetKernelArg(kernel, 3, sizeof(int), &count);

        err = clEnqueueNDRangeKernel(env->queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "clEnqueueNDRangeKernel failed at col=%d: %d\n", col, err);
            clReleaseMemObject(buf);
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            free(a32);
            return 0.0;
        }
    }
    clFinish(env->queue);

    clEnqueueReadBuffer(env->queue, buf, CL_TRUE, 0,
                         sizeof(float) * (size_t)n * (size_t)n, a32, 0, NULL, NULL);

    for (col = 0; col < n; col++)
        det *= (double)a32[col * n + col];

    clReleaseMemObject(buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    free(a32);
    return det;
}
