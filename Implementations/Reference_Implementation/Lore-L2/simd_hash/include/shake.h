#ifndef SHAKE_H
#define SHAKE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136
#define SHAKE_INC_CTX_WORDS 26

void shake128(uint8_t *output, size_t outlen,
              const uint8_t *input, size_t inlen);
void shake256(uint8_t *output, size_t outlen,
              const uint8_t *input, size_t inlen);

void shake128_absorb(uint64_t *state, const uint8_t *input, size_t inlen);
void shake128_squeezeblocks(uint8_t *output, size_t nblocks, uint64_t *state);
void shake128_inc_init(uint64_t *state);
void shake128_inc_absorb(uint64_t *state, const uint8_t *input, size_t inlen);
void shake128_inc_finalize(uint64_t *state);
void shake128_inc_squeeze(uint8_t *output, size_t outlen, uint64_t *state);

void shake256_absorb(uint64_t *state, const uint8_t *input, size_t inlen);
void shake256_squeezeblocks(uint8_t *output, size_t nblocks, uint64_t *state);
void shake256_inc_init(uint64_t *state);
void shake256_inc_absorb(uint64_t *state, const uint8_t *input, size_t inlen);
void shake256_inc_finalize(uint64_t *state);
void shake256_inc_squeeze(uint8_t *output, size_t outlen, uint64_t *state);

void shake128x2(uint8_t *out0, uint8_t *out1, size_t outlen,
                const uint8_t *in0, const uint8_t *in1, size_t inlen);
void shake256x2(uint8_t *out0, uint8_t *out1, size_t outlen,
                const uint8_t *in0, const uint8_t *in1, size_t inlen);

void shake128x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3,
                size_t outlen,
                const uint8_t *in0, const uint8_t *in1,
                const uint8_t *in2, const uint8_t *in3,
                size_t inlen);
void shake256x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3,
                size_t outlen,
                const uint8_t *in0, const uint8_t *in1,
                const uint8_t *in2, const uint8_t *in3,
                size_t inlen);

int shake_cpu_supports_avx2(void);
int shake_cpu_supports_neon(void);
int shake_x2_is_accelerated(void);
int shake_x4_is_accelerated(void);

#ifdef __cplusplus
}
#endif

#endif
