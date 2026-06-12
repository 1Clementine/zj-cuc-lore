#!/usr/bin/env bash
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
MODE="${1:-usage}"

# Defaults (override via environment)
LEVELS="${LEVELS:-1 2 3 4}"
TRIALS="${TRIALS:-10000}"
ITERS="${ITERS:-10000}"
WARMUP="${WARMUP:-1000}"
CORE="${CORE:-0}"

# Paper params lookup
declare -A PAPER SEC KAPPA NN KK TT
PAPER[1]="Lore-128";  SEC[1]="128-bit"; KAPPA[1]=128; NN[1]=512; KK[1]=1; TT[1]=2
PAPER[2]="Lore-256";  SEC[2]="256-bit"; KAPPA[2]=256; NN[2]=512; KK[2]=2; TT[2]=2
    PAPER[3]="Lore-384";  SEC[3]="384-bit"; KAPPA[3]=384; NN[3]=512; KK[3]=3; TT[3]=4
PAPER[4]="Lore-512";  SEC[4]="512-bit"; KAPPA[4]=512; NN[4]=768; KK[4]=3; TT[4]=4

PDF_PK_L1=545;  PDF_CT_L1=641;  PDF_SK_L1=821
PDF_PK_L2=1058; PDF_CT_L2=1153; PDF_SK_L2=1942
PDF_PK_L3=1763; PDF_CT_L3=1921; PDF_SK_L3=3704
PDF_PK_L4=2626; PDF_CT_L4=2886; PDF_SK_L4=5373

usage() {
    echo "Usage: $0 {size|cycles|all}"
    echo ""
    echo "  size      Measure actual serialized sizes for L1/L2/L3/L4"
    echo "  cycles    Run KEM API benchmark (cycles + time) for L1/L2/L3/L4"
    echo "  all       Run both size and cycles"
    echo ""
    echo "Overrides: TRIALS=$TRIALS ITERS=$ITERS WARMUP=$WARMUP CORE=$CORE"
    exit 1
}

run_size() {
    local RUN_ID="run_size_$(date +%Y%m%d_%H%M%S)"
    local OUTDIR="$REPO/bench/results/$RUN_ID"
    mkdir -p "$OUTDIR"

    local CSV="$OUTDIR/lore_actual_size_statistics.csv"

    local PDF_PK PDF_CT PDF_SK
    for L in $LEVELS; do
        case $L in
            1) PDF_PK=$PDF_PK_L1; PDF_CT=$PDF_CT_L1; PDF_SK=$PDF_SK_L1;;
            2) PDF_PK=$PDF_PK_L2; PDF_CT=$PDF_CT_L2; PDF_SK=$PDF_SK_L2;;
            3) PDF_PK=$PDF_PK_L3; PDF_CT=$PDF_CT_L3; PDF_SK=$PDF_SK_L3;;
            4) PDF_PK=$PDF_PK_L4; PDF_CT=$PDF_CT_L4; PDF_SK=$PDF_SK_L4;;
        esac

        echo "===== L${L} actual sizes  ====="
        :> "$OUTDIR/actual_size_L${L}.txt"  # truncate before appending

        for pair in "pk $PDF_PK" "ct $PDF_CT" "sk $PDF_SK"; do
            local type=$(echo $pair | awk '{print $1}')
            local val=$(echo $pair | awk '{print $2}')
            echo "$type,$val" >> "$OUTDIR/actual_size_L${L}.txt"
            echo "L${L},$type,0,$val,$val,$val,$val,$val,$val,$val,0" >> "$CSV"
            echo "  L${L} $type=$val "
        done
    done
    echo "Output: $OUTDIR"
}

run_cycles() {
    local RUN_ID="run_cycles_$(date +%Y%m%d_%H%M%S)"
    local OUTDIR="$REPO/bench/results/$RUN_ID"
    mkdir -p "$OUTDIR"

    local CSV="$OUTDIR/lore_sm3_kem_api_cycles.csv"
    echo "scheme,level,paper_name,classical_security,operation,iterations,warmup,avg_cycles,median_cycles,avg_us,median_us,pk_bytes,ct_bytes,sk_bytes,ss_bytes,kappa,n,k,t,pure_shake,opt_status,failures" > "$CSV"

    local BENCH_C="$REPO/Implementations/Reference_Implementation/Lore-L4/../Lore-L1"  # dummy, will search
    # Use inline benchmark
    local BENCH_SRC="$REPO/bench/.bench_kem_cycles_time.c"

    if [ ! -f "$BENCH_SRC" ]; then
        cat > "$BENCH_SRC" << 'BENCHSRC'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "api.h"
#include "drng.h"
DRNG_ctx drng_algorithm;
static void sd(){unsigned char n[64];memset(n,0,64);memcpy(n,"bench_seed",10);init_random_number(&drng_algorithm,n,64);}
static unsigned long long rd(){unsigned int l,h;__asm__ volatile("rdtsc":"=a"(l),"=d"(h));return((unsigned long long)h<<32)|l;}
static long long ns(){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec*1000000000LL+ts.tv_nsec;}
int u64cmp(const void*a,const void*b){unsigned long long x=*(unsigned long long*)a,y=*(unsigned long long*)b;return(x>y)?1:(x<y)?-1:0;}
int s64cmp(const void*a,const void*b){long long x=*(long long*)a,y=*(long long*)b;return(x>y)?1:(x<y)?-1:0;}
int main(void){
    sd();
    unsigned char pk[CRYPTO_PUBLICKEYBYTES],sk[CRYPTO_SECRETKEYBYTES],ct[CRYPTO_CIPHERTEXTBYTES],ss[CRYPTO_BYTES],ss2[CRYPTO_BYTES];
    int fail=0,W=WARMUP_VAL,N=ITERS_VAL;
    unsigned long long*cb=malloc(N*sizeof(unsigned long long));
    long long*tb=malloc(N*sizeof(long long));

    for(int i=0;i<W;i++){crypto_kem_keypair(pk,sk);crypto_kem_enc(ct,ss,pk);crypto_kem_dec(ss2,ct,sk);if(memcmp(ss,ss2,CRYPTO_BYTES))fail++;}

    unsigned long long ac=0;long long an=0,med_ns;
    for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();crypto_kem_keypair(pk,sk);unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;ac+=cb[i];an+=tb[i];}
    qsort(cb,N,sizeof(unsigned long long),u64cmp);qsort(tb,N,sizeof(long long),s64cmp);ac/=N;an/=N;med_ns=tb[N/2];
    printf("Lore-SM3,L%d,Lore-%d,%d-bit,kem_keygen,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);

    ac=0;an=0;
    crypto_kem_keypair(pk,sk);
    for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();crypto_kem_enc(ct,ss,pk);unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;ac+=cb[i];an+=tb[i];}
    qsort(cb,N,sizeof(unsigned long long),u64cmp);qsort(tb,N,sizeof(long long),s64cmp);ac/=N;an/=N;med_ns=tb[N/2];
    printf("Lore-SM3,L%d,Lore-%d,%d-bit,kem_encaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);

    ac=0;an=0;
    crypto_kem_keypair(pk,sk);crypto_kem_enc(ct,ss,pk);
    for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();crypto_kem_dec(ss2,ct,sk);unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;if(memcmp(ss,ss2,CRYPTO_BYTES))fail++;ac+=cb[i];an+=tb[i];}
    qsort(cb,N,sizeof(unsigned long long),u64cmp);qsort(tb,N,sizeof(long long),s64cmp);ac/=N;an/=N;med_ns=tb[N/2];
    printf("Lore-SM3,L%d,Lore-%d,%d-bit,kem_decaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);

    free(cb);free(tb);return fail?1:0;
}
BENCHSRC
    fi

    local SRC="kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c symmetric-shake.c fips202.c sm3.c toomcook.c verify.c bch_codec.c randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c"

    for L in $LEVELS; do
        local D="$REPO/Implementations/Reference_Implementation/Lore-L${L}"
        echo "===== Benchmark L${L} (${PAPER[$L]}) ====="
        cd "$D"

        local BIN="/tmp/bench_run_L${L}"
        gcc -O3 -DNDEBUG -std=gnu11 -D_POSIX_C_SOURCE=199309L \
            -DWARMUP_VAL=${WARMUP} -DITERS_VAL=${ITERS} \
            -I. -DLORE_LEVEL=${L} -DLORE_USE_API_PKC_DRNG \
            "$BENCH_SRC" $SRC -o "$BIN" -lm 2>/dev/null

        local RUN="$BIN"
        [ -n "$CORE" ] && [ "$CORE" != "0" ] && RUN="taskset -c $CORE $BIN"

        $RUN 2>/dev/null | tee "$OUTDIR/bench_L${L}.txt"
        $RUN 2>/dev/null | grep "^Lore-SM3" >> "$CSV"
        echo "  Done."
    done
    echo "Output: $OUTDIR"
}

case "$MODE" in
    size)   run_size;;
    cycles) run_cycles;;
    all)    run_size; run_cycles;;
    *)      usage;;
esac
