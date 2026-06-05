#include <string.h>
#include "sm3.h"

#define L_SHIFT(a, n) ((a) << (n) | ((a) & 0xFFFFFFFF) >> (32 - (n)))
#define P0(x) ((x) ^ L_SHIFT((x), 9) ^ L_SHIFT((x), 17))
#define P1(x) ((x) ^ L_SHIFT((x), 15) ^ L_SHIFT((x), 23))
#define FF1(x, y, z) ((x) ^ (y) ^ (z))
#define FF2(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG1(x, y, z) ((x) ^ (y) ^ (z))
#define GG2(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))
#define PUT32(a, b) ((a)[0] = (unsigned char)((b) >> 24), \
                     (a)[1] = (unsigned char)((b) >> 16), \
                     (a)[2] = (unsigned char)((b) >> 8),  \
                     (a)[3] = (unsigned char)(b))

static void sm3_bit_init(unsigned int *init_digest)
{
    init_digest[0] = 0x7380166F;
    init_digest[1] = 0x4914B2B9;
    init_digest[2] = 0x172442D7;
    init_digest[3] = 0xDA8A0600;
    init_digest[4] = 0xA96F30BC;
    init_digest[5] = 0x163138AA;
    init_digest[6] = 0xE38DEE4D;
    init_digest[7] = 0xB0FB0E4E;
}

static void sm3_bit_compress(unsigned int dgst[8], const unsigned char *msg, unsigned long long blocks)
{
    unsigned int A, B, C, D, E, F, G, H;
    unsigned int W[68];
    unsigned int W_prime[64];
    unsigned int SS1, SS2, TT1, TT2;
    int i;
    while (blocks--)
    {
        for (i = 0; i < 16; i++)
        {
            W[i] = ((unsigned int)(msg + i * 4)[0] << 24 | (unsigned int)(msg + i * 4)[1] << 16 | (unsigned int)(msg + i * 4)[2] << 8 | (unsigned int)(msg + i * 4)[3]);
        }
        for (; i < 68; i++)
        {
            W[i] = P1(W[i - 16] ^ W[i - 9] ^ L_SHIFT(W[i - 3], 15)) ^ L_SHIFT(W[i - 13], 7) ^ W[i - 6];
        }
        for (i = 0; i < 64; i++)
        {
            W_prime[i] = W[i] ^ W[i + 4];
        }
        A = dgst[0]; B = dgst[1]; C = dgst[2]; D = dgst[3];
        E = dgst[4]; F = dgst[5]; G = dgst[6]; H = dgst[7];
        for (i = 0; i < 64; i++)
        {
            if (i < 16)
                SS1 = L_SHIFT(L_SHIFT(A, 12) + E + L_SHIFT(0x79cc4519U, i & 0x1F), 7);
            else
                SS1 = L_SHIFT(L_SHIFT(A, 12) + E + L_SHIFT(0x7a879d8aU, i & 0x1F), 7);
            SS2 = SS1 ^ L_SHIFT(A, 12);
            if (i < 16)
            {
                TT1 = FF1(A, B, C) + D + SS2 + W_prime[i];
                TT2 = GG1(E, F, G) + H + SS1 + W[i];
            }
            else
            {
                TT1 = FF2(A, B, C) + D + SS2 + W_prime[i];
                TT2 = GG2(E, F, G) + H + SS1 + W[i];
            }
            D = C; C = L_SHIFT(B, 9); B = A; A = TT1;
            H = G; G = L_SHIFT(F, 19); F = E; E = P0(TT2);
        }
        dgst[0] ^= A; dgst[1] ^= B; dgst[2] ^= C; dgst[3] ^= D;
        dgst[4] ^= E; dgst[5] ^= F; dgst[6] ^= G; dgst[7] ^= H;
        msg += 64;
    }
}

static void sm3_bit(const unsigned char *msg, unsigned long long msg_bitlen, unsigned char *dgst)
{
    unsigned long long block_num = msg_bitlen / 512;
    unsigned long long remain = msg_bitlen & 0x1FF;
    unsigned int digest[8];
    sm3_bit_init(digest);
    if (block_num != 0)
        sm3_bit_compress(digest, msg, block_num);
    unsigned char block[64];
    memset(block, 0, 64);
    memcpy(block, msg + block_num * 64, (remain + 7) >> 3);
    block[remain >> 3] &= ((0xFF00 >> (remain & 0x7)) & 0xFF);
    block[remain >> 3] |= (1 << (7 - (remain & 0x7)));
    if (remain <= 512 - 65)
    {
        memset(block + (remain >> 3) + 1, 0, (512 - remain - 65) >> 3);
    }
    else
    {
        memset(block + (remain >> 3) + 1, 0, (512 - remain - 1) >> 3);
        sm3_bit_compress(digest, block, 1);
        memset(block, 0, 64 - 8);
    }
    PUT32(block + 56, block_num >> 32 << 9);
    PUT32(block + 60, (block_num << 9) + (remain));
    sm3_bit_compress(digest, block, 1);
    for (int i = 0; i < 8; i++)
        PUT32(dgst + i * 4, digest[i]);
}

void sm3(uint8_t *out, const uint8_t *in, size_t inlen)
{
    sm3_bit(in, (unsigned long long)inlen * 8, out);
}

void sm3_kdf(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
    uint8_t tmp[SM3_OUTLEN];
    uint32_t ctr = 1;
    while (outlen > 0)
    {
        uint8_t ctr_be[4];
        ctr_be[0] = (uint8_t)(ctr >> 24);
        ctr_be[1] = (uint8_t)(ctr >> 16);
        ctr_be[2] = (uint8_t)(ctr >> 8);
        ctr_be[3] = (uint8_t)(ctr);

        uint8_t buf[4 + 512]; /* safe upper bound */
        memcpy(buf, in, inlen);
        memcpy(buf + inlen, ctr_be, 4);

        sm3(tmp, buf, 4 + inlen);

        size_t to_copy = (outlen < SM3_OUTLEN) ? outlen : SM3_OUTLEN;
        memcpy(out, tmp, to_copy);
        out += to_copy;
        outlen -= to_copy;
        ctr++;
    }
}
