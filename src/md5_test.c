#include <stdio.h>
#include <string.h>
#include "md5_cpu.h"

static int check(const char *input, const char *expected_hex)
{
    unsigned char digest[16];
    char hex[33];
    md5((const unsigned char *)input, strlen(input), digest);
    md5_to_hex(digest, hex);
    printf("md5(\"%s\") = %s  (ожидали %s)  %s\n",
           input, hex, expected_hex, strcmp(hex, expected_hex) == 0 ? "OK" : "FAIL");
    return strcmp(hex, expected_hex) == 0;
}

int main(void)
{
    int ok = 1;
    ok &= check("", "d41d8cd98f00b204e9800998ecf8427e");
    ok &= check("abc", "900150983cd24fb0d6963f7d28e17f72");
    ok &= check("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
    ok &= check("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
    printf(ok ? "\nВСЕ ПРОВЕРКИ ПРОЙДЕНЫ\n" : "\nЕСТЬ ОШИБКИ\n");
    return ok ? 0 : 1;
}
