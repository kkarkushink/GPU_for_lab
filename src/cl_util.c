#include "cl_util.h"
#include <stdio.h>
#include <stdlib.h>

int cl_env_init(CLEnv *env)
{
    cl_platform_id platforms[8];
    cl_uint n_platforms = 0;
    cl_device_id gpu = NULL, fallback = NULL;
    cl_int err;
    cl_uint p, d;

    err = clGetPlatformIDs(8, platforms, &n_platforms);
    if (err != CL_SUCCESS || n_platforms == 0) {
        fprintf(stderr, "OpenCL: платформы не найдены (err=%d)\n", err);
        return 0;
    }

    for (p = 0; p < n_platforms && !gpu; p++) {
        cl_device_id devices[8];
        cl_uint n_devices = 0;

        if (clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 8, devices, &n_devices) != CL_SUCCESS)
            continue;

        for (d = 0; d < n_devices; d++) {
            cl_device_type type;
            clGetDeviceInfo(devices[d], CL_DEVICE_TYPE, sizeof(type), &type, NULL);
            if (!fallback) {
                fallback = devices[d];
                env->platform = platforms[p];
            }
            if (type & CL_DEVICE_TYPE_GPU) {
                gpu = devices[d];
                env->platform = platforms[p];
                break;
            }
        }
    }

    env->device = gpu ? gpu : fallback;
    if (!env->device) {
        fprintf(stderr, "OpenCL: устройства не найдены\n");
        return 0;
    }

    env->context = clCreateContext(NULL, 1, &env->device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clCreateContext failed: %d\n", err);
        return 0;
    }

    env->queue = clCreateCommandQueueWithProperties(env->context, env->device, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clCreateCommandQueueWithProperties failed: %d\n", err);
        clReleaseContext(env->context);
        return 0;
    }

    return 1;
}

void cl_env_destroy(CLEnv *env)
{
    if (env->queue)
        clReleaseCommandQueue(env->queue);
    if (env->context)
        clReleaseContext(env->context);
}

cl_program cl_build_program(const CLEnv *env, const char *kernel_src)
{
    cl_int err;
    cl_program program = clCreateProgramWithSource(env->context, 1, &kernel_src, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clCreateProgramWithSource failed: %d\n", err);
        return NULL;
    }

    err = clBuildProgram(program, 1, &env->device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[8192];
        size_t log_len = 0;
        clGetProgramBuildInfo(program, env->device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, &log_len);
        fprintf(stderr, "clBuildProgram failed: %d\n%.*s\n", err, (int)log_len, log);
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}

char *read_text_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    long size;
    char *buf;

    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}
