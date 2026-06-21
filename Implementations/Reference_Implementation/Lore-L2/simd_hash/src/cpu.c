#include "shake.h"
#include "sm3.h"

int shake_cpu_supports_avx2(void) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(__GNUC__) || defined(__clang__)
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2") != 0;
#else
    return 0;
#endif
#else
    return 0;
#endif
}

int shake_cpu_supports_neon(void) {
#if defined(__aarch64__) || defined(__ARM_NEON) || defined(__ARM_NEON__)
    return 1;
#else
    return 0;
#endif
}

int sm3_cpu_supports_avx2(void) {
    return shake_cpu_supports_avx2();
}

int sm3_cpu_supports_neon(void) {
    return shake_cpu_supports_neon();
}
