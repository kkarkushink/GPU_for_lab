// один work-item = один пароль-кандидат, idx -> строка как в bruteforce_cpu.c
// пароли короткие, поэтому MD5 всегда в один блок

__constant uint K[64] = {
    0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
    0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
    0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
    0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
    0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
    0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
    0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
    0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
    0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
    0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
    0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
    0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
    0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
    0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
    0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
    0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u
};

__constant uint S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

static uint rotl32(uint x, uint c)
{
    return (x << c) | (x >> (32 - c));
}

// msg_len <= 55, укладывается в один блок
static void md5_one_block(const uchar *msg, uint msg_len, uint h_out[4])
{
    uchar block[64];
    uint m[16];
    uint a, b, c, d;
    ulong bit_len;
    int i;

    for (i = 0; i < 64; i++)
        block[i] = 0;
    for (i = 0; i < (int)msg_len; i++)
        block[i] = msg[i];
    block[msg_len] = 0x80;

    bit_len = (ulong)msg_len * 8;
    for (i = 0; i < 8; i++)
        block[56 + i] = (uchar)(bit_len >> (8 * i));

    for (i = 0; i < 16; i++) {
        m[i] = (uint)block[i * 4 + 0]
             | ((uint)block[i * 4 + 1] << 8)
             | ((uint)block[i * 4 + 2] << 16)
             | ((uint)block[i * 4 + 3] << 24);
    }

    a = 0x67452301u; b = 0xefcdab89u; c = 0x98badcfeu; d = 0x10325476u;

    for (i = 0; i < 64; i++) {
        uint f;
        int g;

        if (i < 16) {
            f = (b & c) | (~b & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | (~d & c);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        } else {
            f = c ^ (b | ~d);
            g = (7 * i) % 16;
        }

        f = f + a + K[i] + m[g];
        a = d;
        d = c;
        c = b;
        b = b + rotl32(f, S[i]);
    }

    h_out[0] = 0x67452301u + a;
    h_out[1] = 0xefcdab89u + b;
    h_out[2] = 0x98badcfeu + c;
    h_out[3] = 0x10325476u + d;
}

__kernel void bruteforce_md5(
    __global uint *found_index,
    uint total_count,
    uint charset_len,
    uint pw_len,
    __constant uchar *charset,
    uint target0, uint target1, uint target2, uint target3)
{
    uint idx = get_global_id(0);
    if (idx >= total_count)
        return;

    uchar candidate[16];
    uint tmp = idx;
    int i;

    for (i = (int)pw_len - 1; i >= 0; i--) {
        candidate[i] = charset[tmp % charset_len];
        tmp /= charset_len;
    }

    uint h[4];
    md5_one_block(candidate, pw_len, h);

    if (h[0] == target0 && h[1] == target1 && h[2] == target2 && h[3] == target3) {
        atomic_min(found_index, idx);
    }
}
