#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "md5_cpu.h"
#include "bruteforce_cpu.h"
#include "bruteforce_gpu.h"
#include "cl_util.h"

static const char CHARSET[] = "abcdefghijklmnopqrstuvwxyz";
#define CHARSET_LEN 26

// 26^len кандидатов, len=6 это уже ~309 млн
static const int LENGTHS[] = { 3, 4, 5, 6 };
#define N_LENGTHS (int)(sizeof(LENGTHS) / sizeof(LENGTHS[0]))

static uint64_t pow_u64(uint64_t base, int exp)
{
    uint64_t r = 1;
    int i;
    for (i = 0; i < exp; i++)
        r *= base;
    return r;
}

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

    kernel_src = read_text_file("src/bruteforce.cl");
    if (!kernel_src) {
        fprintf(stderr, "Не найден src/bruteforce.cl (запускать из корня проекта)\n");
        return 1;
    }

    csv = fopen("bin/hash_results.csv", "w");
    fprintf(csv, "pw_len,space_size,cpu_ms,gpu_ms,speedup,cpu_ok,gpu_ok\n");

    printf("%6s %14s %12s %12s %10s %8s %8s\n",
           "len", "простр-во", "CPU, мс", "GPU, мс", "ускор.", "CPU ок", "GPU ок");

    for (i = 0; i < N_LENGTHS; i++) {
        int pw_len = LENGTHS[i];
        uint64_t space = pow_u64(CHARSET_LEN, pw_len);
        uint64_t target_idx = space - 1; /* худший случай: искомое - последний кандидат */
        char target_password[16];
        unsigned char target_digest[16];
        clock_t t0, t1;
        double cpu_ms, gpu_ms;
        uint64_t cpu_found, gpu_found;

        index_to_candidate(target_idx, CHARSET, CHARSET_LEN, pw_len, target_password);
        md5((const unsigned char *)target_password, (size_t)pw_len, target_digest);

        t0 = clock();
        cpu_found = bruteforce_cpu(target_digest, CHARSET, CHARSET_LEN, pw_len, space);
        t1 = clock();
        cpu_ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;

        t0 = clock();
        gpu_found = bruteforce_gpu(&env, kernel_src, target_digest, CHARSET, CHARSET_LEN, pw_len, space);
        t1 = clock();
        gpu_ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;

        printf("%6d %14llu %12.3f %12.3f %10.2f %8s %8s\n",
               pw_len, (unsigned long long)space, cpu_ms, gpu_ms,
               gpu_ms > 0 ? cpu_ms / gpu_ms : 0.0,
               cpu_found == target_idx ? "да" : "НЕТ",
               gpu_found == target_idx ? "да" : "НЕТ");

        fprintf(csv, "%d,%llu,%.4f,%.4f,%.4f,%d,%d\n",
                pw_len, (unsigned long long)space, cpu_ms, gpu_ms,
                gpu_ms > 0 ? cpu_ms / gpu_ms : 0.0,
                cpu_found == target_idx, gpu_found == target_idx);
    }

    fclose(csv);
    free(kernel_src);
    cl_env_destroy(&env);
    return 0;
}
