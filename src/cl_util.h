#ifndef CL_UTIL_H
#define CL_UTIL_H

#include <CL/cl.h>

typedef struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
} CLEnv;

// находит GPU (или любое устройство) и создаёт контекст + очередь
int cl_env_init(CLEnv *env);
void cl_env_destroy(CLEnv *env);

cl_program cl_build_program(const CLEnv *env, const char *kernel_src);

char *read_text_file(const char *path);

#endif /* CL_UTIL_H */
