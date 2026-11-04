#include "bruteforce_gpu.h"
#include <stdio.h>
#include <limits.h>

#define LOCAL_SIZE 256

uint64_t bruteforce_gpu(CLEnv *env, const char *kernel_src, const unsigned char target[16],
                         const char *charset, int charset_len, int pw_len, uint64_t total_count)
{
    cl_int err;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem buf_found = NULL, buf_charset = NULL;
    uint64_t result = total_count;
    cl_uint not_found = 0xFFFFFFFFu;
    cl_uint target_words[4];
    int i;

    for (i = 0; i < 4; i++) {
        target_words[i] = (cl_uint)target[i * 4 + 0]
                         | ((cl_uint)target[i * 4 + 1] << 8)
                         | ((cl_uint)target[i * 4 + 2] << 16)
                         | ((cl_uint)target[i * 4 + 3] << 24);
    }

    program = cl_build_program(env, kernel_src);
    if (!program)
        return total_count;

    kernel = clCreateKernel(program, "bruteforce_md5", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clCreateKernel failed: %d\n", err);
        goto cleanup;
    }

    buf_found = clCreateBuffer(env->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                sizeof(cl_uint), &not_found, &err);
    buf_charset = clCreateBuffer(env->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  (size_t)charset_len, (void *)charset, &err);
    if (!buf_found || !buf_charset) {
        fprintf(stderr, "clCreateBuffer failed\n");
        goto cleanup;
    }

    {
        cl_uint total_count32 = (cl_uint)total_count;
        cl_uint charset_len32 = (cl_uint)charset_len;
        cl_uint pw_len32 = (cl_uint)pw_len;
        size_t local_size = LOCAL_SIZE;
        size_t global_size = ((total_count + LOCAL_SIZE - 1) / LOCAL_SIZE) * LOCAL_SIZE;

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_found);
        clSetKernelArg(kernel, 1, sizeof(cl_uint), &total_count32);
        clSetKernelArg(kernel, 2, sizeof(cl_uint), &charset_len32);
        clSetKernelArg(kernel, 3, sizeof(cl_uint), &pw_len32);
        clSetKernelArg(kernel, 4, sizeof(cl_mem), &buf_charset);
        clSetKernelArg(kernel, 5, sizeof(cl_uint), &target_words[0]);
        clSetKernelArg(kernel, 6, sizeof(cl_uint), &target_words[1]);
        clSetKernelArg(kernel, 7, sizeof(cl_uint), &target_words[2]);
        clSetKernelArg(kernel, 8, sizeof(cl_uint), &target_words[3]);

        err = clEnqueueNDRangeKernel(env->queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "clEnqueueNDRangeKernel failed: %d\n", err);
            goto cleanup;
        }
        clFinish(env->queue);
    }

    {
        cl_uint found = not_found;
        clEnqueueReadBuffer(env->queue, buf_found, CL_TRUE, 0, sizeof(cl_uint), &found, 0, NULL, NULL);
        if (found != not_found)
            result = (uint64_t)found;
    }

cleanup:
    if (buf_charset) clReleaseMemObject(buf_charset);
    if (buf_found) clReleaseMemObject(buf_found);
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
    return result;
}
