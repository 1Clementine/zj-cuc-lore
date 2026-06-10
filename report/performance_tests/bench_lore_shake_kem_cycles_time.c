/* Lore-SHAKE KEM API cycle+time benchmark */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "api.h"
#include "kem.h"
#include "drng.h"

DRNG_ctx drng_algorithm;

static void seed_rng(void) {
    unsigned char n[64]; memset(n, 0, 64);
    memcpy(n, "kem_bench_seed_", 14);
    init_random_number(&drng_algorithm, n, 64);
}

static inline unsigned long long rdtsc(void) {
    unsigned int lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)hi << 32) | lo;
}

static long long ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

#define ITERS 10000
#define WARMUP 1000

static int u64cmp(const void *a, const void *b) {
    return (*(unsigned long long*)a > *(unsigned long long*)b) ? 1 : -1;
}
static int s64cmp(const void *a, const void *b) {
    return (*(long long*)a > *(long long*)b) ? 1 : -1;
}

int main(void) {
    seed_rng();

    unsigned char pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES], ss[CRYPTO_BYTES], ss2[CRYPTO_BYTES];
    int fail = 0;

    /* Warmup */
    for (int i = 0; i < WARMUP; i++) {
        crypto_kem_keypair(pk, sk);
        crypto_kem_enc(ct, ss, pk);
        crypto_kem_dec(ss2, ct, sk);
        if (memcmp(ss, ss2, CRYPTO_BYTES)) fail++;
    }

    unsigned long long *cbuf = malloc(ITERS * sizeof(unsigned long long));
    long long *tbuf = malloc(ITERS * sizeof(long long));

    /* ===== KEM keygen ===== */
    for (int i = 0; i < ITERS; i++) {
        long long t0 = ns_now();
        unsigned long long c0 = rdtsc();
        crypto_kem_keypair(pk, sk);
        unsigned long long c1 = rdtsc();
        long long t1 = ns_now();
        cbuf[i] = c1 - c0; tbuf[i] = t1 - t0;
    }
    qsort(cbuf, ITERS, sizeof(unsigned long long), u64cmp);
    qsort(tbuf, ITERS, sizeof(long long), s64cmp);
    unsigned long long kg_avg_c = 0, kg_med_c = cbuf[ITERS/2];
    long long kg_avg_ns = 0, kg_med_ns = tbuf[ITERS/2];
    for (int i = 0; i < ITERS; i++) { kg_avg_c += cbuf[i]; kg_avg_ns += tbuf[i]; }
    kg_avg_c /= ITERS; kg_avg_ns /= ITERS;

    /* ===== KEM encaps ===== */
    crypto_kem_keypair(pk, sk);
    for (int i = 0; i < ITERS; i++) {
        long long t0 = ns_now();
        unsigned long long c0 = rdtsc();
        crypto_kem_enc(ct, ss, pk);
        unsigned long long c1 = rdtsc();
        long long t1 = ns_now();
        cbuf[i] = c1 - c0; tbuf[i] = t1 - t0;
    }
    qsort(cbuf, ITERS, sizeof(unsigned long long), u64cmp);
    qsort(tbuf, ITERS, sizeof(long long), s64cmp);
    unsigned long long en_avg_c = 0, en_med_c = cbuf[ITERS/2];
    long long en_avg_ns = 0, en_med_ns = tbuf[ITERS/2];
    for (int i = 0; i < ITERS; i++) { en_avg_c += cbuf[i]; en_avg_ns += tbuf[i]; }
    en_avg_c /= ITERS; en_avg_ns /= ITERS;

    /* ===== KEM decaps ===== */
    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(ct, ss, pk);
    for (int i = 0; i < ITERS; i++) {
        long long t0 = ns_now();
        unsigned long long c0 = rdtsc();
        crypto_kem_dec(ss2, ct, sk);
        unsigned long long c1 = rdtsc();
        long long t1 = ns_now();
        cbuf[i] = c1 - c0; tbuf[i] = t1 - t0;
        if (memcmp(ss, ss2, CRYPTO_BYTES)) fail++;
    }
    qsort(cbuf, ITERS, sizeof(unsigned long long), u64cmp);
    qsort(tbuf, ITERS, sizeof(long long), s64cmp);
    unsigned long long de_avg_c = 0, de_med_c = cbuf[ITERS/2];
    long long de_avg_ns = 0, de_med_ns = tbuf[ITERS/2];
    for (int i = 0; i < ITERS; i++) { de_avg_c += cbuf[i]; de_avg_ns += tbuf[i]; }
    de_avg_c /= ITERS; de_avg_ns /= ITERS;

    printf("scheme,level,paper_name,classical_security,operation,iterations,warmup,"
           "avg_cycles,median_cycles,avg_us,median_us,"
           "pk_bytes,ct_bytes,sk_bytes,kappa,n,k,t,pure_shake,opt_status,failures\n");
    printf("Lore-SHAKE,L%d,Lore-%d,%d-bit,kem_keygen,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",
           LORE_LEVEL, LORE_KAPPA, LORE_KAPPA, ITERS, WARMUP,
           kg_avg_c, kg_med_c, kg_avg_ns/1000.0, kg_med_ns/1000.0,
           (size_t)CRYPTO_PUBLICKEYBYTES, (size_t)CRYPTO_CIPHERTEXTBYTES, (size_t)CRYPTO_SECRETKEYBYTES,
           LORE_KAPPA, LORE_N, LORE_K, LORE_T, fail);
    printf("Lore-SHAKE,L%d,Lore-%d,%d-bit,kem_encaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",
           LORE_LEVEL, LORE_KAPPA, LORE_KAPPA, ITERS, WARMUP,
           en_avg_c, en_med_c, en_avg_ns/1000.0, en_med_ns/1000.0,
           (size_t)CRYPTO_PUBLICKEYBYTES, (size_t)CRYPTO_CIPHERTEXTBYTES, (size_t)CRYPTO_SECRETKEYBYTES,
           LORE_KAPPA, LORE_N, LORE_K, LORE_T, fail);
    printf("Lore-SHAKE,L%d,Lore-%d,%d-bit,kem_decaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",
           LORE_LEVEL, LORE_KAPPA, LORE_KAPPA, ITERS, WARMUP,
           de_avg_c, de_med_c, de_avg_ns/1000.0, de_med_ns/1000.0,
           (size_t)CRYPTO_PUBLICKEYBYTES, (size_t)CRYPTO_CIPHERTEXTBYTES, (size_t)CRYPTO_SECRETKEYBYTES,
           LORE_KAPPA, LORE_N, LORE_K, LORE_T, fail);

    free(cbuf); free(tbuf);
    return fail ? 1 : 0;
}
