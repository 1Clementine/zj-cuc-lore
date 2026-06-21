#include <immintrin.h>
#include <stdint.h>

#include "shake_internal.h"
#include "KeccakP-1600-times4-SnP.h"

static uint64_t load64(const uint8_t *x)
{
  uint64_t r = 0;

  for (size_t i = 0; i < 8; ++i) {
    r |= (uint64_t)x[i] << 8 * i;
  }
  return r;
}

static void store64(uint8_t *x, uint64_t u)
{
  for(size_t i=0; i<8; ++i) {
    x[i] = (uint8_t)u;
    u >>= 8;
  }
}

/* Use implementation from the Keccak Code Package. */
#define KeccakF1600_StatePermute4x(s) \
  KeccakP1600times4_PermuteAll_24rounds((void *)(s))

static void keccak_absorb4x(__m256i *s,
                          size_t r,
                          const uint8_t *m0,
                          const uint8_t *m1,
                          const uint8_t *m2,
                          const uint8_t *m3,
                          size_t mlen,
                          uint8_t p)
{
  size_t i;
  uint8_t t0[200];
  uint8_t t1[200];
  uint8_t t2[200];
  uint8_t t3[200];

  uint64_t *ss = (uint64_t *)s;


  while (mlen >= r)
  {
    for (i = 0; i < r / 8; ++i)
    {
      ss[4*i+0] ^= load64(m0 + 8 * i);
      ss[4*i+1] ^= load64(m1 + 8 * i);
      ss[4*i+2] ^= load64(m2 + 8 * i);
      ss[4*i+3] ^= load64(m3 + 8 * i);
    }

    KeccakF1600_StatePermute4x(s);
    mlen -= r;
    m0 += r;
    m1 += r;
    m2 += r;
    m3 += r;
  }

  for (i = 0; i < r; ++i)
  {
    t0[i] = 0;
    t1[i] = 0;
    t2[i] = 0;
    t3[i] = 0;
  }
  for (i = 0; i < mlen; ++i)
  {
    t0[i] = m0[i];
    t1[i] = m1[i];
    t2[i] = m2[i];
    t3[i] = m3[i];
  }

  t0[i] = p;
  t1[i] = p;
  t2[i] = p;
  t3[i] = p;

  t0[r - 1] |= 128;
  t1[r - 1] |= 128;
  t2[r - 1] |= 128;
  t3[r - 1] |= 128;

  for (i = 0; i < r / 8; ++i)
  {
    ss[4*i+0] ^= load64(t0 + 8 * i);
    ss[4*i+1] ^= load64(t1 + 8 * i);
    ss[4*i+2] ^= load64(t2 + 8 * i);
    ss[4*i+3] ^= load64(t3 + 8 * i);
  }
}


static void keccak_squeezeblocks4x(uint8_t *h0,
                                   uint8_t *h1,
                                   uint8_t *h2,
                                   uint8_t *h3,
                                   size_t nblocks,
                                   __m256i *s,
                                   size_t r)
{
  size_t i;

  uint64_t *ss = (uint64_t *)s;

  while(nblocks > 0)
  {
    KeccakF1600_StatePermute4x(s);
    for(i=0;i<(r>>3);i++)
    {
      store64(h0+8*i, ss[4*i+0]);
      store64(h1+8*i, ss[4*i+1]);
      store64(h2+8*i, ss[4*i+2]);
      store64(h3+8*i, ss[4*i+3]);
    }
    h0 += r;
    h1 += r;
    h2 += r;
    h3 += r;
    nblocks--;
  }
}



void shake128x4_avx2(uint8_t *out0,
                     uint8_t *out1,
                     uint8_t *out2,
                     uint8_t *out3, size_t outlen,
                     const uint8_t *in0,
                     const uint8_t *in1,
                     const uint8_t *in2,
                     const uint8_t *in3, size_t inlen)
{
  __m256i s[25];
  uint8_t t0[SHAKE128_RATE];
  uint8_t t1[SHAKE128_RATE];
  uint8_t t2[SHAKE128_RATE];
  uint8_t t3[SHAKE128_RATE];
  size_t i;

  /* zero state */
  for(i=0;i<25;i++)
    s[i] = _mm256_setzero_si256();

  /* absorb 4 message of identical length in parallel */
  keccak_absorb4x(s, SHAKE128_RATE, in0, in1, in2, in3, inlen, 0x1F);

  /* Squeeze output */
  keccak_squeezeblocks4x(out0, out1, out2, out3, outlen/SHAKE128_RATE, s, SHAKE128_RATE);

  out0 += (outlen/SHAKE128_RATE)*SHAKE128_RATE;
  out1 += (outlen/SHAKE128_RATE)*SHAKE128_RATE;
  out2 += (outlen/SHAKE128_RATE)*SHAKE128_RATE;
  out3 += (outlen/SHAKE128_RATE)*SHAKE128_RATE;

  if(outlen%SHAKE128_RATE)
  {
    keccak_squeezeblocks4x(t0, t1, t2, t3, 1, s, SHAKE128_RATE);
    for(i=0;i<outlen%SHAKE128_RATE;i++)
    {
      out0[i] = t0[i];
      out1[i] = t1[i];
      out2[i] = t2[i];
      out3[i] = t3[i];
    }
  }
}


void shake256x4_avx2(uint8_t *out0,
                     uint8_t *out1,
                     uint8_t *out2,
                     uint8_t *out3, size_t outlen,
                     const uint8_t *in0,
                     const uint8_t *in1,
                     const uint8_t *in2,
                     const uint8_t *in3, size_t inlen)
{
  __m256i s[25];
  uint8_t t0[SHAKE256_RATE];
  uint8_t t1[SHAKE256_RATE];
  uint8_t t2[SHAKE256_RATE];
  uint8_t t3[SHAKE256_RATE];
  size_t i;

  /* zero state */
  for(i=0;i<25;i++)
    s[i] = _mm256_setzero_si256();

  /* absorb 4 message of identical length in parallel */
  keccak_absorb4x(s, SHAKE256_RATE, in0, in1, in2, in3, inlen, 0x1F);

  /* Squeeze output */
  keccak_squeezeblocks4x(out0, out1, out2, out3, outlen/SHAKE256_RATE, s, SHAKE256_RATE);

  out0 += (outlen/SHAKE256_RATE)*SHAKE256_RATE;
  out1 += (outlen/SHAKE256_RATE)*SHAKE256_RATE;
  out2 += (outlen/SHAKE256_RATE)*SHAKE256_RATE;
  out3 += (outlen/SHAKE256_RATE)*SHAKE256_RATE;

  if(outlen%SHAKE256_RATE)
  {
    keccak_squeezeblocks4x(t0, t1, t2, t3, 1, s, SHAKE256_RATE);
    for(i=0;i<outlen%SHAKE256_RATE;i++)
    {
      out0[i] = t0[i];
      out1[i] = t1[i];
      out2[i] = t2[i];
      out3[i] = t3[i];
    }
  }
}
