#include "reduce.h"
#include "params.h"
#include <stdint.h>
#ifdef LORE_USE_AVX2
#include <immintrin.h>
#endif

#define LORE_QINV ((int16_t)-255)

static inline int16_t barrett_reduce_scalar(int16_t a)
{
    int32_t t = (int32_t)a;
    t = (t & 0xFF) - (t >> 8);
    t = (t & 0xFF) - (t >> 8);
    return (int16_t)t;
}

int16_t montgomery_reduce(int64_t a)
{
    int32_t t;
    int16_t v;
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

#ifdef LORE_USE_AVX2
static inline __m256i barrett_reduce_16way(__m256i v)
{
    const __m256i mask = _mm256_set1_epi32(0xFF);
    __m128i v_lo = _mm256_castsi256_si128(v);
    __m256i lo = _mm256_cvtepi16_epi32(v_lo);
    __m128i v_hi = _mm256_extracti128_si256(v, 1);
    __m256i hi = _mm256_cvtepi16_epi32(v_hi);
    lo = _mm256_sub_epi32(_mm256_and_si256(lo, mask), _mm256_srai_epi32(lo, 8));
    hi = _mm256_sub_epi32(_mm256_and_si256(hi, mask), _mm256_srai_epi32(hi, 8));
    lo = _mm256_sub_epi32(_mm256_and_si256(lo, mask), _mm256_srai_epi32(lo, 8));
    hi = _mm256_sub_epi32(_mm256_and_si256(hi, mask), _mm256_srai_epi32(hi, 8));
    return _mm256_packs_epi32(lo, hi);
}
#endif

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
