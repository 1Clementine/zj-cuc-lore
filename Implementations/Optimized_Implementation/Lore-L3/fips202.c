#include "fips202.h"
#include "auxfunc.h"

#include <stdlib.h>
#include <string.h>

enum {
    LORE_MODE_SHAKE128 = 0x11,
    LORE_MODE_SHAKE256 = 0x12,
    LORE_MODE_SHA3_256 = 0x21,
    LORE_MODE_SHA3_512 = 0x22
};

static void state_reset(keccak_state *state, uint8_t mode)
{
    memset(state, 0, sizeof(*state));
    state->mode = mode;
}

static void state_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
    if (state->overflow) {
        return;
    }

    if (inlen == 0) {
        return;
    }

    if (state->inlen + inlen > sizeof(state->input)) {
        state->overflow = 1;
        return;
    }

    memcpy(state->input + state->inlen, in, inlen);
    state->inlen += inlen;
}

static int tagged_xof(uint8_t *out,
                      size_t outlen,
                      uint8_t tag,
                      const uint8_t *in,
                      size_t inlen)
{
    size_t tagged_len = inlen + 1;
    uint8_t *buf;
    int ret;

    buf = (uint8_t *)malloc(tagged_len == 0 ? 1 : tagged_len);
    if (buf == NULL) {
        return -1;
    }

    buf[0] = tag;
    if (inlen > 0) {
        memcpy(buf + 1, in, inlen);
    }

    ret = pseudoXOF((unsigned long long)outlen * 8ULL,
                    buf,
                    (unsigned long long)tagged_len * 8ULL,
                    out);

    free(buf);
    return ret;
}

static int tagged_hash256(uint8_t out[32],
                          uint8_t tag,
                          const uint8_t *in,
                          size_t inlen)
{
    size_t tagged_len = inlen + 1;
    uint8_t *buf;
    int ret;

    buf = (uint8_t *)malloc(tagged_len == 0 ? 1 : tagged_len);
    if (buf == NULL) {
        return -1;
    }

    buf[0] = tag;
    if (inlen > 0) {
        memcpy(buf + 1, in, inlen);
    }

    ret = sm3hash(256,
                  buf,
                  (unsigned long long)tagged_len * 8ULL,
                  out);

    free(buf);
    return ret;
}

static int tagged_hash512(uint8_t out[64],
                          uint8_t tag,
                          const uint8_t *in,
                          size_t inlen)
{
    size_t tagged_len = inlen + 1;
    uint8_t *buf;
    int ret;

    buf = (uint8_t *)malloc(tagged_len == 0 ? 1 : tagged_len);
    if (buf == NULL) {
        return -1;
    }

    buf[0] = tag;
    if (inlen > 0) {
        memcpy(buf + 1, in, inlen);
    }

    ret = pseudohash(512,
                     buf,
                     (unsigned long long)tagged_len * 8ULL,
                     out);

    free(buf);
    return ret;
}

static void state_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
    size_t total;
    uint8_t *tmp;

    if (outlen == 0) {
        return;
    }

    if (state->overflow) {
        memset(out, 0, outlen);
        return;
    }

    if (!state->finalized) {
        state->finalized = 1;
    }

    total = state->outpos + outlen;
    tmp = (uint8_t *)malloc(total == 0 ? 1 : total);
    if (tmp == NULL) {
        memset(out, 0, outlen);
        return;
    }

    if (tagged_xof(tmp, total, state->mode, state->input, state->inlen) != 0) {
        memset(out, 0, outlen);
        free(tmp);
        return;
    }

    memcpy(out, tmp + state->outpos, outlen);
    state->outpos += outlen;

    free(tmp);
}

void shake128_init(keccak_state *state)
{
    state_reset(state, LORE_MODE_SHAKE128);
}

void shake128_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
    state_absorb(state, in, inlen);
}

void shake128_finalize(keccak_state *state)
{
    state->finalized = 1;
}

void shake128_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
    state_squeeze(out, outlen, state);
}

void shake128_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen)
{
    state_reset(state, LORE_MODE_SHAKE128);
    state_absorb(state, in, inlen);
    state->finalized = 1;
}

void shake128_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state)
{
    state_squeeze(out, nblocks * SHAKE128_RATE, state);
}

void shake256_init(keccak_state *state)
{
    state_reset(state, LORE_MODE_SHAKE256);
}

void shake256_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
    state_absorb(state, in, inlen);
}

void shake256_finalize(keccak_state *state)
{
    state->finalized = 1;
}

void shake256_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
    state_squeeze(out, outlen, state);
}

void shake256_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen)
{
    state_reset(state, LORE_MODE_SHAKE256);
    state_absorb(state, in, inlen);
    state->finalized = 1;
}

void shake256_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state)
{
    state_squeeze(out, nblocks * SHAKE256_RATE, state);
}

void shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
    if (tagged_xof(out, outlen, LORE_MODE_SHAKE128, in, inlen) != 0) {
        memset(out, 0, outlen);
    }
}

void shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
    if (tagged_xof(out, outlen, LORE_MODE_SHAKE256, in, inlen) != 0) {
        memset(out, 0, outlen);
    }
}

void sha3_256(uint8_t h[32], const uint8_t *in, size_t inlen)
{
    if (tagged_hash256(h, LORE_MODE_SHA3_256, in, inlen) != 0) {
        memset(h, 0, 32);
    }
}

void sha3_512(uint8_t h[64], const uint8_t *in, size_t inlen)
{
    if (tagged_hash512(h, LORE_MODE_SHA3_512, in, inlen) != 0) {
        memset(h, 0, 64);
    }
}
