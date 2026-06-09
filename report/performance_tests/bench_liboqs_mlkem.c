#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h>

static long long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void warmup(const char *alg, int n) {
    OQS_KEM *kem = OQS_KEM_new(alg);
    if (!kem) return;
    unsigned char pk[4096], sk[8192], ct[4096], ss[64], ss1[64];
    for (int i = 0; i < n; i++) {
        OQS_KEM_keypair(kem, pk, sk);
        OQS_KEM_encaps(kem, ct, ss, pk);
        OQS_KEM_decaps(kem, ss1, ct, sk);
    }
    OQS_KEM_free(kem);
}

static void bench(const char *alg, const char *op, int iters, FILE *out) {
    OQS_KEM *kem = OQS_KEM_new(alg);
    if (!kem) { fprintf(stderr, "OQS_KEM_new(%s) failed\n", alg); return; }

    unsigned char *pk  = malloc(kem->length_public_key);
    unsigned char *sk  = malloc(kem->length_secret_key);
    unsigned char *ct  = malloc(kem->length_ciphertext);
    unsigned char *ss  = malloc(kem->length_shared_secret);
    unsigned char *ss1 = malloc(kem->length_shared_secret);
    int failures = 0;

    long long t0 = now_ns();
    for (int i = 0; i < iters; i++) {
        if (!strcmp(op, "keypair")) {
            OQS_KEM_keypair(kem, pk, sk);
        } else if (!strcmp(op, "encaps")) {
            OQS_KEM_keypair(kem, pk, sk);
            OQS_KEM_encaps(kem, ct, ss, pk);
        } else if (!strcmp(op, "decaps")) {
            OQS_KEM_keypair(kem, pk, sk);
            OQS_KEM_encaps(kem, ct, ss, pk);
            OQS_KEM_decaps(kem, ss1, ct, sk);
            if (memcmp(ss, ss1, kem->length_shared_secret)) failures++;
        }
    }
    long long total_ns = now_ns() - t0;
    double avg_us = (double)total_ns / (double)iters / 1000.0;

    fprintf(out, "%s,%s,%d,1000,%lld,%.2f,%d\n",
            alg, op, iters, total_ns, avg_us, failures);

    free(pk); free(sk); free(ct); free(ss); free(ss1);
    OQS_KEM_free(kem);
}

int main(void) {
    const char *algs[] = {
        OQS_KEM_alg_ml_kem_512,
        OQS_KEM_alg_ml_kem_768,
        OQS_KEM_alg_ml_kem_1024,
        NULL
    };
    const char *ops[] = {"keypair", "encaps", "decaps", NULL};
    int iters = 10000;

    FILE *out = stdout;
    fprintf(out, "scheme,operation,iterations,warmup,total_ns,avg_us,failures\n");

    for (int a = 0; algs[a]; a++) {
        fprintf(stderr, "Warmup %s...\n", algs[a]);
        warmup(algs[a], 1000);
        for (int o = 0; ops[o]; o++) {
            fprintf(stderr, "Bench %s %s...\n", algs[a], ops[o]);
            bench(algs[a], ops[o], iters, out);
        }
    }

    fclose(out);
    return 0;
}
