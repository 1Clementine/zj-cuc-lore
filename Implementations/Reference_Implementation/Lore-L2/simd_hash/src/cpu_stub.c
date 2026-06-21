#include <stdint.h>
#include <stddef.h>

/* CPU feature stubs — AVX2 always available in this prototype */
int shake_cpu_supports_avx2(void) { return 1; }
int shake_cpu_supports_neon(void) { return 0; }

/* NEON stubs — not compiled in SHAKE-only prototype */
void shake128x4_neon(uint8_t *o0, uint8_t *o1, uint8_t *o2, uint8_t *o3,
                     size_t ol, const uint8_t *i0, const uint8_t *i1,
                     const uint8_t *i2, const uint8_t *i3, size_t il) {}
void shake256x4_neon(uint8_t *o0, uint8_t *o1, uint8_t *o2, uint8_t *o3,
                     size_t ol, const uint8_t *i0, const uint8_t *i1,
                     const uint8_t *i2, const uint8_t *i3, size_t il) {}
void shake128x2_neon(uint8_t *o0, uint8_t *o1, size_t ol,
                     const uint8_t *i0, const uint8_t *i1, size_t il) {}
void shake256x2_neon(uint8_t *o0, uint8_t *o1, size_t ol,
                     const uint8_t *i0, const uint8_t *i1, size_t il) {}
