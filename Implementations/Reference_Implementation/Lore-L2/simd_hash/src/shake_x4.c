#include "shake.h"

#if SHAKE_ENABLE_AVX2 || SHAKE_ENABLE_NEON
#include "shake_internal.h"
#endif

int shake_x4_is_accelerated(void) {
#if SHAKE_ENABLE_AVX2
    if (shake_cpu_supports_avx2()) {
        return 1;
    }
#endif
#if SHAKE_ENABLE_NEON
    if (shake_cpu_supports_neon()) {
        return 1;
    }
#endif
    return 0;
}

void shake128x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3,
                size_t outlen,
                const uint8_t *in0, const uint8_t *in1,
                const uint8_t *in2, const uint8_t *in3,
                size_t inlen) {
#if SHAKE_ENABLE_AVX2
    if (shake_cpu_supports_avx2()) {
        shake128x4_avx2(out0, out1, out2, out3, outlen,
                        in0, in1, in2, in3, inlen);
        return;
    }
#endif
#if SHAKE_ENABLE_NEON
    if (shake_cpu_supports_neon()) {
        shake128x2_neon(out0, out1, outlen, in0, in1, inlen);
        shake128x2_neon(out2, out3, outlen, in2, in3, inlen);
        return;
    }
#endif

    shake128(out0, outlen, in0, inlen);
    shake128(out1, outlen, in1, inlen);
    shake128(out2, outlen, in2, inlen);
    shake128(out3, outlen, in3, inlen);
}

void shake256x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3,
                size_t outlen,
                const uint8_t *in0, const uint8_t *in1,
                const uint8_t *in2, const uint8_t *in3,
                size_t inlen) {
#if SHAKE_ENABLE_AVX2
    if (shake_cpu_supports_avx2()) {
        shake256x4_avx2(out0, out1, out2, out3, outlen,
                        in0, in1, in2, in3, inlen);
        return;
    }
#endif
#if SHAKE_ENABLE_NEON
    if (shake_cpu_supports_neon()) {
        shake256x2_neon(out0, out1, outlen, in0, in1, inlen);
        shake256x2_neon(out2, out3, outlen, in2, in3, inlen);
        return;
    }
#endif

    shake256(out0, outlen, in0, inlen);
    shake256(out1, outlen, in1, inlen);
    shake256(out2, outlen, in2, inlen);
    shake256(out3, outlen, in3, inlen);
}
