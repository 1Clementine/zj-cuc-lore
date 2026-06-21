#ifndef SHAKE_INTERNAL_H
#define SHAKE_INTERNAL_H

#include "shake.h"

void shake128x4_avx2(uint8_t *out0, uint8_t *out1, uint8_t *out2,
                     uint8_t *out3, size_t outlen,
                     const uint8_t *in0, const uint8_t *in1,
                     const uint8_t *in2, const uint8_t *in3,
                     size_t inlen);
void shake256x4_avx2(uint8_t *out0, uint8_t *out1, uint8_t *out2,
                     uint8_t *out3, size_t outlen,
                     const uint8_t *in0, const uint8_t *in1,
                     const uint8_t *in2, const uint8_t *in3,
                     size_t inlen);

void shake128x2_neon(uint8_t *out0, uint8_t *out1, size_t outlen,
                     const uint8_t *in0, const uint8_t *in1,
                     size_t inlen);
void shake256x2_neon(uint8_t *out0, uint8_t *out1, size_t outlen,
                     const uint8_t *in0, const uint8_t *in1,
                     size_t inlen);

#endif
