#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "api.h"

/* ================================================================
 * Architecture-aware CPU cycle counter.
 * x86_64: rdtsc/rdtscp  (no perf_event_open, no root needed)
 * aarch64: perf_event_open  (requires perf_event_paranoid <= 2 or root)
 * ================================================================ */

static const char *cycle_method = "unknown";

#if defined(__x86_64__) || defined(__i386__)
/* ---- x86_64 / i386: rdtsc (inline asm, no intrinsics needed) ---- */
static void cyc_init(void) { cycle_method = "rdtsc_x86"; }
static inline unsigned long long cyc_rd(void) {
    unsigned int lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)hi << 32) | lo;
}
static void cyc_close(void) {}
#elif defined(__aarch64__)
/* ---- aarch64: perf_event_open ---- */
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
static int perf_fd = -1;
static void cyc_init(void) {
    cycle_method = "perf_event_open_aarch64";
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_CPU_CYCLES;
    attr.size = sizeof(attr);
    attr.disabled = 1;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    perf_fd = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
    if (perf_fd < 0) {
        fprintf(stderr, "[ERROR] perf_event_open failed on aarch64 (errno=%d). "
                "Check perf_event_paranoid or run as root.\n", errno);
        exit(1);
    }
    ioctl(perf_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(perf_fd, PERF_EVENT_IOC_ENABLE, 0);
}
static inline unsigned long long cyc_rd(void) {
    unsigned long long v = 0;
    read(perf_fd, &v, sizeof(v));
    return v;
}
static void cyc_close(void) { if (perf_fd >= 0) close(perf_fd); }
#else
#error "Unsupported architecture for cycle counter. Only x86_64 and aarch64 are supported."
#endif

static long long ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static int u64cmp(const void *a, const void *b) {
    unsigned long long x = *(const unsigned long long*)a, y = *(const unsigned long long*)b;
    return (x > y) ? 1 : (x < y) ? -1 : 0;
}
static int s64cmp(const void *a, const void *b) {
    long long x = *(const long long*)a, y = *(const long long*)b;
    return (x > y) ? 1 : (x < y) ? -1 : 0;
}

int main(void) {
    cyc_init();
    fprintf(stderr, "arch=%s cycle_counter=%s\n",
#if defined(__x86_64__)
        "x86_64",
#elif defined(__i386__)
        "i386",
#elif defined(__aarch64__)
        "aarch64",
#else
        "unknown",
#endif
        cycle_method);

    unsigned char pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES], ss[CRYPTO_BYTES], ss2[CRYPTO_BYTES];
    int fail = 0, W = WARMUP_VAL, N = ITERS_VAL;

    unsigned long long *cb = malloc(N * sizeof(unsigned long long));
    long long *tb = malloc(N * sizeof(long long));
    if (!cb || !tb) { fprintf(stderr, "malloc failed\n"); exit(1); }

    for (int i = 0; i < W; i++) {
        crypto_kem_keypair(pk, sk);
        crypto_kem_enc(ct, ss, pk);
        crypto_kem_dec(ss2, ct, sk);
        if (memcmp(ss, ss2, CRYPTO_BYTES)) fail++;
    }

    /* keygen */
    {
        unsigned long long ac = 0; long long an = 0;
        for (int i = 0; i < N; i++) {
            long long t0 = ns(); unsigned long long c0 = cyc_rd();
            crypto_kem_keypair(pk, sk);
            unsigned long long c1 = cyc_rd(); long long t1 = ns();
            cb[i] = c1 - c0; tb[i] = t1 - t0;
            ac += cb[i]; an += tb[i];
        }
        qsort(cb, N, sizeof(unsigned long long), u64cmp);
        qsort(tb, N, sizeof(long long), s64cmp);
        ac /= N; an /= N;
        printf("Lore-SM3,ref,L%d,Lore-%d,%d-bit,ref,kem_keygen,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%zu,%d,%d,%d,%d,SM3,AB_inline,%d\n",
            LORE_LEVEL, LORE_KAPPA, LORE_KAPPA, N, W, ac, cb[N/2],
            an / 1000.0, tb[N/2] / 1000.0,
            (size_t)CRYPTO_PUBLICKEYBYTES, (size_t)CRYPTO_CIPHERTEXTBYTES,
            (size_t)CRYPTO_SECRETKEYBYTES, (size_t)CRYPTO_BYTES,
            LORE_KAPPA, LORE_N, LORE_K, LORE_T, fail);
    }

    /* encaps */
    {
        crypto_kem_keypair(pk, sk);
        unsigned long long ac = 0; long long an = 0;
        for (int i = 0; i < N; i++) {
            long long t0 = ns(); unsigned long long c0 = cyc_rd();
            crypto_kem_enc(ct, ss, pk);
            unsigned long long c1 = cyc_rd(); long long t1 = ns();
            cb[i] = c1 - c0; tb[i] = t1 - t0;
            ac += cb[i]; an += tb[i];
        }
        qsort(cb, N, sizeof(unsigned long long), u64cmp);
        qsort(tb, N, sizeof(long long), s64cmp);
        ac /= N; an /= N;
        printf("Lore-SM3,ref,L%d,Lore-%d,%d-bit,ref,kem_encaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%zu,%d,%d,%d,%d,SM3,AB_inline,%d\n",
            LORE_LEVEL, LORE_KAPPA, LORE_KAPPA, N, W, ac, cb[N/2],
            an / 1000.0, tb[N/2] / 1000.0,
            (size_t)CRYPTO_PUBLICKEYBYTES, (size_t)CRYPTO_CIPHERTEXTBYTES,
            (size_t)CRYPTO_SECRETKEYBYTES, (size_t)CRYPTO_BYTES,
            LORE_KAPPA, LORE_N, LORE_K, LORE_T, fail);
    }

    /* decaps */
    {
        crypto_kem_keypair(pk, sk);
        crypto_kem_enc(ct, ss, pk);
        unsigned long long ac = 0; long long an = 0;
        for (int i = 0; i < N; i++) {
            long long t0 = ns(); unsigned long long c0 = cyc_rd();
            crypto_kem_dec(ss2, ct, sk);
            unsigned long long c1 = cyc_rd(); long long t1 = ns();
            cb[i] = c1 - c0; tb[i] = t1 - t0;
            if (memcmp(ss, ss2, CRYPTO_BYTES)) fail++;
            ac += cb[i]; an += tb[i];
        }
        qsort(cb, N, sizeof(unsigned long long), u64cmp);
        qsort(tb, N, sizeof(long long), s64cmp);
        ac /= N; an /= N;
        printf("Lore-SM3,ref,L%d,Lore-%d,%d-bit,ref,kem_decaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%zu,%d,%d,%d,%d,SM3,AB_inline,%d\n",
            LORE_LEVEL, LORE_KAPPA, LORE_KAPPA, N, W, ac, cb[N/2],
            an / 1000.0, tb[N/2] / 1000.0,
            (size_t)CRYPTO_PUBLICKEYBYTES, (size_t)CRYPTO_CIPHERTEXTBYTES,
            (size_t)CRYPTO_SECRETKEYBYTES, (size_t)CRYPTO_BYTES,
            LORE_KAPPA, LORE_N, LORE_K, LORE_T, fail);
    }

    cyc_close();
    free(cb); free(tb);
    return fail ? 1 : 0;
}
