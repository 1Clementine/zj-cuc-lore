#ifndef SIMD_X4_H
#define SIMD_X4_H

#ifdef LORE_USE_AVX2_HASH

#include <stdint.h>
#include <string.h>

/*
 * Forward-declare shake128x4 from the SIMD backend.
 * We do NOT include shake.h here because its absorb/squeezeblocks
 * declarations conflict with Lore's fips202.h namespace-mapped symbols.
 * Only the x4 one-shot interface is needed in polyvec.c.
 */
void shake128x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3,
                size_t outlen,
                const uint8_t *in0, const uint8_t *in1,
                const uint8_t *in2, const uint8_t *in3,
                size_t inlen);

/*
 * Generate 4 XOF output buffers in parallel using SHAKE128 x4.
 *
 * Each lane uses the same seed and length, but unique i,j indices
 * (matching Lore matrix generation: seed || i || j).
 */
static inline int gen_matrix_x4_xof(
    uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3,
    size_t outlen,
    const uint8_t seed[32],
    uint8_t i0, uint8_t j0,
    uint8_t i1, uint8_t j1,
    uint8_t i2, uint8_t j2,
    uint8_t i3, uint8_t j3)
{
    uint8_t in0[34], in1[34], in2[34], in3[34];

    memcpy(in0, seed, 32); in0[32] = i0; in0[33] = j0;
    memcpy(in1, seed, 32); in1[32] = i1; in1[33] = j1;
    memcpy(in2, seed, 32); in2[32] = i2; in2[33] = j2;
    memcpy(in3, seed, 32); in3[32] = i3; in3[33] = j3;

    shake128x4(out0, out1, out2, out3,
               outlen,
               in0, in1, in2, in3,
               34);
    return 0;
}

#endif /* LORE_USE_AVX2_HASH */
#endif /* SIMD_X4_H */
