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

static uint64_t pow_u64(uint64_t base, int exp)
{
    uint64_t r = 1;
    int i;
    for (i = 0; i < exp; i++)
        r *= base;
    return r;
}

int main(int argc, char **argv)
{
    CLEnv env;
    char *kernel_src;
    char password[16];
    unsigned char digest[16];
    char hex[33];
    int pw_len;
    uint64_t space, idx, found;
    clock_t t0, t1;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc > 1) {
        strncpy(password, argv[1], sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
    } else {
        printf("Введите пароль строчными латинскими буквами (до 6 букв, например cat): ");
        if (!fgets(password, sizeof(password), stdin))
            return 1;
        password[strcspn(password, "\r\n")] = '\0';
    }
    pw_len = (int)strlen(password);

    if (pw_len == 0 || pw_len > 6) {
        printf("Длина пароля должна быть от 1 до 6 букв (иначе перебор на CPU займёт слишком долго).\n");
        return 1;
    }
    {
        int i;
        for (i = 0; i < pw_len; i++) {
            if (password[i] < 'a' || password[i] > 'z') {
                printf("Пароль должен состоять только из строчных латинских букв a-z.\n");
                return 1;
            }
        }
    }

    md5((const unsigned char *)password, (size_t)pw_len, digest);
    md5_to_hex(digest, hex);
    printf("Загаданный пароль: \"%s\" (длина %d), его MD5 = %s\n", password, pw_len, hex);

    if (!cl_env_init(&env)) {
        fprintf(stderr, "Не удалось инициализировать OpenCL\n");
        return 1;
    }
    kernel_src = read_text_file("src/bruteforce.cl");
    if (!kernel_src) {
        fprintf(stderr, "Не найден src/bruteforce.cl (запускать из корня проекта)\n");
        return 1;
    }

    space = pow_u64(CHARSET_LEN, pw_len);
    printf("Перебираю все пароли длины %d из букв a-z (%llu вариантов)...\n",
           pw_len, (unsigned long long)space);

    t0 = clock();
    idx = bruteforce_cpu(digest, CHARSET, CHARSET_LEN, pw_len, space);
    t1 = clock();
    if (idx < space) {
        char found_pw[16];
        index_to_candidate(idx, CHARSET, CHARSET_LEN, pw_len, found_pw);
        found_pw[pw_len] = '\0';
        printf("CPU: найден пароль \"%s\" за %.3f мс\n",
               found_pw, 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC);
    } else {
        printf("CPU: пароль не найден\n");
    }

    t0 = clock();
    idx = bruteforce_gpu(&env, kernel_src, digest, CHARSET, CHARSET_LEN, pw_len, space);
    t1 = clock();
    if (idx < space) {
        char found_pw[16];
        index_to_candidate(idx, CHARSET, CHARSET_LEN, pw_len, found_pw);
        found_pw[pw_len] = '\0';
        printf("GPU: найден пароль \"%s\" за %.3f мс\n",
               found_pw, 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC);
    } else {
        printf("GPU: пароль не найден\n");
    }

    free(kernel_src);
    cl_env_destroy(&env);
    return 0;
}
