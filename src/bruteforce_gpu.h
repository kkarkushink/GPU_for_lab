#ifndef BRUTEFORCE_GPU_H
#define BRUTEFORCE_GPU_H

#include <stdint.h>
#include "cl_util.h"

// то же самое, но на GPU
uint64_t bruteforce_gpu(CLEnv *env, const char *kernel_src, const unsigned char target[16],
                         const char *charset, int charset_len, int pw_len, uint64_t total_count);

#endif /* BRUTEFORCE_GPU_H */
