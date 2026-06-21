#include "fips202.h"

#include <string.h>

#define NROUNDS 24

static uint64_t rol64(uint64_t x, unsigned int offset)
{
    return (x << offset) ^ (x >> (64 - offset));
}

static uint64_t load64(const uint8_t x[8])
{
    unsigned int i;
    uint64_t r = 0;

    for (i = 0; i < 8; i++) {
        r |= (uint64_t)x[i] << (8 * i);
    }

    return r;
}

static void store64(uint8_t x[8], uint64_t u)
{
    unsigned int i;

    for (i = 0; i < 8; i++) {
        x[i] = (uint8_t)(u >> (8 * i));
    }
}

static void keccakf1600(uint64_t s[25])
{
    static const uint64_t rc[NROUNDS] = {
        0x0000000000000001ULL, 0x0000000000008082ULL,
        0x800000000000808aULL, 0x8000000080008000ULL,
        0x000000000000808bULL, 0x0000000080000001ULL,
        0x8000000080008081ULL, 0x8000000000008009ULL,
        0x000000000000008aULL, 0x0000000000000088ULL,
        0x0000000080008009ULL, 0x000000008000000aULL,
        0x000000008000808bULL, 0x800000000000008bULL,
        0x8000000000008089ULL, 0x8000000000008003ULL,
        0x8000000000008002ULL, 0x8000000000000080ULL,
        0x000000000000800aULL, 0x800000008000000aULL,
        0x8000000080008081ULL, 0x8000000000008080ULL,
        0x0000000080000001ULL, 0x8000000080008008ULL
    };
    unsigned int round;

    for (round = 0; round < NROUNDS; round++) {
        uint64_t b[25];
        uint64_t c[5];
        uint64_t d[5];

        c[0] = s[0] ^ s[5] ^ s[10] ^ s[15] ^ s[20];
        c[1] = s[1] ^ s[6] ^ s[11] ^ s[16] ^ s[21];
        c[2] = s[2] ^ s[7] ^ s[12] ^ s[17] ^ s[22];
        c[3] = s[3] ^ s[8] ^ s[13] ^ s[18] ^ s[23];
        c[4] = s[4] ^ s[9] ^ s[14] ^ s[19] ^ s[24];

        d[0] = c[4] ^ rol64(c[1], 1);
        d[1] = c[0] ^ rol64(c[2], 1);
        d[2] = c[1] ^ rol64(c[3], 1);
        d[3] = c[2] ^ rol64(c[4], 1);
        d[4] = c[3] ^ rol64(c[0], 1);

        s[0] ^= d[0];  s[5] ^= d[0];  s[10] ^= d[0]; s[15] ^= d[0]; s[20] ^= d[0];
        s[1] ^= d[1];  s[6] ^= d[1];  s[11] ^= d[1]; s[16] ^= d[1]; s[21] ^= d[1];
        s[2] ^= d[2];  s[7] ^= d[2];  s[12] ^= d[2]; s[17] ^= d[2]; s[22] ^= d[2];
        s[3] ^= d[3];  s[8] ^= d[3];  s[13] ^= d[3]; s[18] ^= d[3]; s[23] ^= d[3];
        s[4] ^= d[4];  s[9] ^= d[4];  s[14] ^= d[4]; s[19] ^= d[4]; s[24] ^= d[4];

        b[0] = s[0];
        b[1] = rol64(s[6], 44);
        b[2] = rol64(s[12], 43);
        b[3] = rol64(s[18], 21);
        b[4] = rol64(s[24], 14);
        b[5] = rol64(s[3], 28);
        b[6] = rol64(s[9], 20);
        b[7] = rol64(s[10], 3);
        b[8] = rol64(s[16], 45);
        b[9] = rol64(s[22], 61);
        b[10] = rol64(s[1], 1);
        b[11] = rol64(s[7], 6);
        b[12] = rol64(s[13], 25);
        b[13] = rol64(s[19], 8);
        b[14] = rol64(s[20], 18);
        b[15] = rol64(s[4], 27);
        b[16] = rol64(s[5], 36);
        b[17] = rol64(s[11], 10);
        b[18] = rol64(s[17], 15);
        b[19] = rol64(s[23], 56);
        b[20] = rol64(s[2], 62);
        b[21] = rol64(s[8], 55);
        b[22] = rol64(s[14], 39);
        b[23] = rol64(s[15], 41);
        b[24] = rol64(s[21], 2);

        s[0] = b[0] ^ ((~b[1]) & b[2]);
        s[1] = b[1] ^ ((~b[2]) & b[3]);
        s[2] = b[2] ^ ((~b[3]) & b[4]);
        s[3] = b[3] ^ ((~b[4]) & b[0]);
        s[4] = b[4] ^ ((~b[0]) & b[1]);
        s[5] = b[5] ^ ((~b[6]) & b[7]);
        s[6] = b[6] ^ ((~b[7]) & b[8]);
        s[7] = b[7] ^ ((~b[8]) & b[9]);
        s[8] = b[8] ^ ((~b[9]) & b[5]);
        s[9] = b[9] ^ ((~b[5]) & b[6]);
        s[10] = b[10] ^ ((~b[11]) & b[12]);
        s[11] = b[11] ^ ((~b[12]) & b[13]);
        s[12] = b[12] ^ ((~b[13]) & b[14]);
        s[13] = b[13] ^ ((~b[14]) & b[10]);
        s[14] = b[14] ^ ((~b[10]) & b[11]);
        s[15] = b[15] ^ ((~b[16]) & b[17]);
        s[16] = b[16] ^ ((~b[17]) & b[18]);
        s[17] = b[17] ^ ((~b[18]) & b[19]);
        s[18] = b[18] ^ ((~b[19]) & b[15]);
        s[19] = b[19] ^ ((~b[15]) & b[16]);
        s[20] = b[20] ^ ((~b[21]) & b[22]);
        s[21] = b[21] ^ ((~b[22]) & b[23]);
        s[22] = b[22] ^ ((~b[23]) & b[24]);
        s[23] = b[23] ^ ((~b[24]) & b[20]);
        s[24] = b[24] ^ ((~b[20]) & b[21]);

        s[0] ^= rc[round];
    }
}

static void xor_byte(uint64_t s[25], unsigned int pos, uint8_t b)
{
    s[pos >> 3] ^= (uint64_t)b << (8 * (pos & 7));
}

static uint8_t get_byte(const uint64_t s[25], unsigned int pos)
{
    return (uint8_t)(s[pos >> 3] >> (8 * (pos & 7)));
}

static void keccak_init(keccak_state *state, unsigned int rate)
{
    memset(state, 0, sizeof(*state));
    state->rate = rate;
}

static void keccak_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
    size_t i;

    if (state->finalized) {
        return;
    }

    while (state->pos == 0 && inlen >= state->rate) {
        for (i = 0; i < state->rate / 8; i++) {
            state->s[i] ^= load64(in + 8 * i);
        }
        keccakf1600(state->s);
        in += state->rate;
        inlen -= state->rate;
    }

    for (i = 0; i < inlen; i++) {
        if (state->pos == state->rate) {
            keccakf1600(state->s);
            state->pos = 0;
        }
        xor_byte(state->s, state->pos++, in[i]);
    }
}

static void keccak_finalize(keccak_state *state, uint8_t domain)
{
    if (state->finalized) {
        return;
    }

    if (state->pos == state->rate) {
        keccakf1600(state->s);
        state->pos = 0;
    }

    xor_byte(state->s, state->pos, domain);
    xor_byte(state->s, state->rate - 1, 0x80);
    keccakf1600(state->s);
    state->pos = 0;
    state->finalized = 1;
}

static void keccak_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
    size_t i;

    while (state->pos == 0 && outlen >= state->rate) {
        for (i = 0; i < state->rate / 8; i++) {
            store64(out + 8 * i, state->s[i]);
        }
        keccakf1600(state->s);
        out += state->rate;
        outlen -= state->rate;
    }

    for (i = 0; i < outlen; i++) {
        if (state->pos == state->rate) {
            keccakf1600(state->s);
            state->pos = 0;
        }
        out[i] = get_byte(state->s, state->pos++);
    }
}

static void xof_once(uint8_t *out, size_t outlen, unsigned int rate,
                     const uint8_t *in, size_t inlen)
{
    keccak_state state;

    keccak_init(&state, rate);
    keccak_absorb(&state, in, inlen);
    keccak_finalize(&state, 0x1f);
    keccak_squeeze(out, outlen, &state);
}

static void hash_once(uint8_t *out, size_t outlen, unsigned int rate,
                      const uint8_t *in, size_t inlen)
{
    keccak_state state;

    keccak_init(&state, rate);
    keccak_absorb(&state, in, inlen);
    keccak_finalize(&state, 0x06);
    keccak_squeeze(out, outlen, &state);
}

void shake128_init(keccak_state *state)
{
    keccak_init(state, SHAKE128_RATE);
}

void shake128_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
    keccak_absorb(state, in, inlen);
}

void shake128_finalize(keccak_state *state)
{
    keccak_finalize(state, 0x1f);
}

void shake128_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
    shake128_finalize(state);
    keccak_squeeze(out, outlen, state);
}

void shake128_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen)
{
    shake128_init(state);
    shake128_absorb(state, in, inlen);
    shake128_finalize(state);
}

void shake128_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state)
{
    keccak_squeeze(out, nblocks * SHAKE128_RATE, state);
}

void shake256_init(keccak_state *state)
{
    keccak_init(state, SHAKE256_RATE);
}

void shake256_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
    keccak_absorb(state, in, inlen);
}

void shake256_finalize(keccak_state *state)
{
    keccak_finalize(state, 0x1f);
}

void shake256_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
    shake256_finalize(state);
    keccak_squeeze(out, outlen, state);
}

void shake256_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen)
{
    shake256_init(state);
    shake256_absorb(state, in, inlen);
    shake256_finalize(state);
}

void shake256_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state)
{
    keccak_squeeze(out, nblocks * SHAKE256_RATE, state);
}

void shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
    xof_once(out, outlen, SHAKE128_RATE, in, inlen);
}

void shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
    xof_once(out, outlen, SHAKE256_RATE, in, inlen);
}

void sha3_256(uint8_t h[32], const uint8_t *in, size_t inlen)
{
    hash_once(h, 32, SHA3_256_RATE, in, inlen);
}

void sha3_512(uint8_t h[64], const uint8_t *in, size_t inlen)
{
    hash_once(h, 64, SHA3_512_RATE, in, inlen);
}
