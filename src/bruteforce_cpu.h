#ifndef BRUTEFORCE_CPU_H
#define BRUTEFORCE_CPU_H

#include <stdint.h>

// idx -> пароль (запись idx в системе счисления с основанием charset_len)
void index_to_candidate(uint64_t idx, const char *charset, int charset_len, int pw_len, char *out);

// возвращает номер найденного кандидата или total_count, если не нашли
uint64_t bruteforce_cpu(const unsigned char target[16], const char *charset, int charset_len,
                         int pw_len, uint64_t total_count);

#endif /* BRUTEFORCE_CPU_H */
