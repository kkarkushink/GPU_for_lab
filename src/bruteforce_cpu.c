#include "bruteforce_cpu.h"
#include "md5_cpu.h"
#include <string.h>

void index_to_candidate(uint64_t idx, const char *charset, int charset_len, int pw_len, char *out)
{
    int i;
    for (i = pw_len - 1; i >= 0; i--) {
        out[i] = charset[idx % (uint64_t)charset_len];
        idx /= (uint64_t)charset_len;
    }
}

uint64_t bruteforce_cpu(const unsigned char target[16], const char *charset, int charset_len,
                         int pw_len, uint64_t total_count)
{
    char candidate[16];
    unsigned char digest[16];
    uint64_t idx;

    for (idx = 0; idx < total_count; idx++) {
        index_to_candidate(idx, charset, charset_len, pw_len, candidate);
        md5((const unsigned char *)candidate, (size_t)pw_len, digest);
        if (memcmp(digest, target, 16) == 0)
            return idx;
    }
    return total_count;
}
