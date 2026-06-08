#include "reduce.h"
#include "params.h"
#ifdef LORE_USE_AVX2
#include "avx2_utils.h"
#endif

#include <stdint.h>

#define LORE_QINV ((int16_t)-255)

static inline int16_t barrett_reduce_scalar(int16_t a)
{
    int32_t t = (int32_t)a;

    /*
     * For q = 257, use 2^8 ≡ -1 (mod 257).
     * Two folds are sufficient for the coefficient ranges used here.
     */
    t = (t & 0xFF) - (t >> 8);
    t = (t & 0xFF) - (t >> 8);

    return (int16_t)t;
}

int16_t montgomery_reduce(int64_t a)
{
    int32_t t;
    int16_t v;

    /*
     * Important: multiply first, then truncate to int16_t.
     * This matches the original Lore semantics.
     */
    v = (int16_t)(a * (int64_t)LORE_QINV);
    t = (int32_t)((int64_t)v * LORE_Q);
    t = (int32_t)(a - t);
    t >>= 16;
    return (int16_t)t;
}

int16_t barrett_reduce(int16_t a)
{
    return barrett_reduce_scalar(a);
}

void poly_reduce(poly *r)
{
    unsigned int i = 0;
    int16_t *p = r->coeffs;

#ifdef LORE_USE_AVX2
    for (; i + 16 <= LORE_N; i += 16) {
        __m256i v = _mm256_loadu_si256((const __m256i *)&p[i]);
        v = barrett_reduce_16way(v);
        _mm256_storeu_si256((__m256i *)&p[i], v);
    }
#endif
    for (; i < LORE_N; ++i) {
        p[i] = barrett_reduce_scalar(p[i]);
    }
}