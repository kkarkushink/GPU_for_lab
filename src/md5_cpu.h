#ifndef MD5_CPU_H
#define MD5_CPU_H

#include <stddef.h>
#include <stdint.h>

/* Стандартный MD5 (RFC 1321). digest получает 16 байт результата. */
void md5(const unsigned char *msg, size_t len, unsigned char digest[16]);

/* Печатает 16 байт хэша в виде 32 шестнадцатеричных символов + '\0'. */
void md5_to_hex(const unsigned char digest[16], char out[33]);

/* Разбирает строку из 32 шестнадцатеричных символов обратно в 16 байт.
 * Возвращает 1 при успехе, 0 при неверном формате. */
int md5_from_hex(const char *hex, unsigned char digest[16]);

#endif /* MD5_CPU_H */
