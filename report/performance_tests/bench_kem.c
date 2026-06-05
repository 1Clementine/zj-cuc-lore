#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "api.h"
#include "drng.h"

DRNG_ctx drng_algorithm;

#ifndef CRYPTO_PUBLICKEYBYTES
#error "CRYPTO_PUBLICKEYBYTES not defined"
#endif

#ifndef CRYPTO_SECRETKEYBYTES
#error "CRYPTO_SECRETKEYBYTES not defined"
#endif

#ifndef CRYPTO_CIPHERTEXTBYTES
#error "CRYPTO_CIPHERTEXTBYTES not defined"
#endif

#ifndef CRYPTO_BYTES
#error "CRYPTO_BYTES not defined"
#endif

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void print_csv(
    const char *version,
    const char *impl,
    int level,
    const char *op,
    int iterations,
    int warmup,
    uint64_t total_ns,
    int failures)
{
    double avg_ns = (iterations > 0) ? ((double)total_ns / (double)iterations) : 0.0;
    double ops_per_sec = (avg_ns > 0.0) ? (1000000000.0 / avg_ns) : 0.0;

    printf("%s,%s,L%d,%s,%d,%d,%llu,%.2f,%.2f,%d,%d,%d,%d,%d\n",
           version,
           impl,
           level,
           op,
           iterations,
           warmup,
           (unsigned long long)total_ns,
           avg_ns,
           ops_per_sec,
           CRYPTO_PUBLICKEYBYTES,
           CRYPTO_CIPHERTEXTBYTES,
           CRYPTO_SECRETKEYBYTES,
           CRYPTO_BYTES,
           failures);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <version> <impl> <level> <iterations> <warmup>\n", argv[0]);
        return 1;
    }

    const char *version = argv[1];
    const char *impl    = argv[2];
    int level           = atoi(argv[3]);
    int iterations      = atoi(argv[4]);
    int warmup          = atoi(argv[5]);

    if (iterations <= 0 || warmup < 0) {
        fprintf(stderr, "Invalid iterations or warmup\n");
        return 1;
    }

    unsigned char *pk  = malloc(CRYPTO_PUBLICKEYBYTES);
    unsigned char *sk  = malloc(CRYPTO_SECRETKEYBYTES);
    unsigned char *ct  = malloc(CRYPTO_CIPHERTEXTBYTES);
    unsigned char *ss1 = malloc(CRYPTO_BYTES);
    unsigned char *ss2 = malloc(CRYPTO_BYTES);

    if (!pk || !sk || !ct || !ss1 || !ss2) {
        fprintf(stderr, "malloc failed\n");
        free(pk); free(sk); free(ct); free(ss1); free(ss2);
        return 1;
    }

    int failures = 0;
    uint64_t start, end, total;

    /* Init DRNG */
    init_random_number(&drng_algorithm, NULL, 0);

    /* ---- Warmup ---- */
    for (int i = 0; i < warmup; i++) {
        if (crypto_kem_keypair(pk, sk) != 0) failures++;
        if (crypto_kem_enc(ct, ss1, pk) != 0) failures++;
        if (crypto_kem_dec(ss2, ct, sk) != 0) failures++;
        if (memcmp(ss1, ss2, CRYPTO_BYTES) != 0) failures++;
    }

    /* ---- keypair ---- */
    total = 0;
    for (int i = 0; i < iterations; i++) {
        start = now_ns();
        if (crypto_kem_keypair(pk, sk) != 0) failures++;
        end = now_ns();
        total += end - start;
    }
    print_csv(version, impl, level, "keypair", iterations, warmup, total, failures);

    if (crypto_kem_keypair(pk, sk) != 0) failures++;

    /* ---- encaps ---- */
    total = 0;
    for (int i = 0; i < iterations; i++) {
        start = now_ns();
        if (crypto_kem_enc(ct, ss1, pk) != 0) failures++;
        end = now_ns();
        total += end - start;
    }
    print_csv(version, impl, level, "encaps", iterations, warmup, total, failures);

    /* ---- decaps + correctness ---- */
    total = 0;
    for (int i = 0; i < iterations; i++) {
        if (crypto_kem_enc(ct, ss1, pk) != 0) failures++;

        start = now_ns();
        if (crypto_kem_dec(ss2, ct, sk) != 0) failures++;
        end = now_ns();

        total += end - start;

        if (memcmp(ss1, ss2, CRYPTO_BYTES) != 0) {
            fprintf(stderr, "DECAPS MISMATCH at iter %d (level=%d impl=%s version=%s)\n",
                    i, level, impl, version);
            failures++;
        }
    }
    print_csv(version, impl, level, "decaps", iterations, warmup, total, failures);

    free(pk); free(sk); free(ct); free(ss1); free(ss2);
    return failures == 0 ? 0 : 2;
}
