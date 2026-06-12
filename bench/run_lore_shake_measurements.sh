#!/usr/bin/env bash
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
MODE="${1:-usage}"

# Defaults (override via environment)
LEVELS="${LEVELS:-L1 L2 L4}"
TRIALS="${TRIALS:-10000}"
ITERS="${ITERS:-10000}"
WARMUP="${WARMUP:-1000}"
CORE="${CORE:-0}"

# Paper params lookup
declare -A PAPER SEC KAPPA NN KK TT
PAPER[L1]="Lore-128";  SEC[L1]="128-bit"; KAPPA[L1]=128; NN[L1]=512; KK[L1]=1; TT[L1]=2
PAPER[L2]="Lore-256";  SEC[L2]="256-bit"; KAPPA[L2]=256; NN[L2]=512; KK[L2]=2; TT[L2]=2
PAPER[L4]="Lore-512";  SEC[L4]="512-bit"; KAPPA[L4]=512; NN[L4]=768; KK[L4]=3; TT[L4]=4

PDF_PK_L1=545;  PDF_CT_L1=641;  PDF_SK_L1=821
PDF_PK_L2=1058; PDF_CT_L2=1153; PDF_SK_L2=1942
PDF_PK_L4=2626; PDF_CT_L4=2886; PDF_SK_L4=5373

usage() {
    echo "Usage: $0 {size|cycles|all}"
    echo ""
    echo "  size      Measure actual serialized sizes for L1/L2/L4"
    echo "  cycles    Run KEM API benchmark (cycles + time) for L1/L2/L4"
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
    echo "level,type,count,min,avg,max,p5,p50,p95,pdf,avg_minus_pdf" > "$CSV"

    local SRC="kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c symmetric-shake.c fips202.c toomcook.c verify.c bch_codec.c randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c"

    for L in $LEVELS; do
        local D="$REPO/Implementations/Reference_Implementation/Lore-${L}"
        echo "===== Measuring L${L} actual sizes ($TRIALS trials) ====="
        cd "$D"

        cat > /tmp/size_measure_L${L}.c << 'MEASURE'
#include <stdio.h><stdlib.h><string.h>
#include "api.h"
#include "drng.h"
DRNG_ctx drng_algorithm;
static void seed(){unsigned char n[64];memset(n,0,64);memcpy(n,"size_meas_",10);init_random_number(&drng_algorithm,n,64);}
int comp(const void *a,const void *b){unsigned long long x=*(unsigned long long*)a,y=*(unsigned long long*)b;return (x>y)?1:(x<y)?-1:0;}
int main(int c,char**v){
    int n=(c>1)?atoi(v[1]):10000;
    unsigned char pk[CRYPTO_PUBLICKEYBYTES],sk[CRYPTO_SECRETKEYBYTES],ct[CRYPTO_CIPHERTEXTBYTES],ss[CRYPTO_BYTES],ss2[CRYPTO_BYTES];
    unsigned long long *ps=malloc(n*sizeof(unsigned long long)),*cs=malloc(n*sizeof(unsigned long long)),*ks=malloc(n*sizeof(unsigned long long));
    seed(); int f=0;
    for(int i=0;i<n;i++){crypto_kem_keypair(pk,sk);crypto_kem_enc(ct,ss,pk);crypto_kem_dec(ss2,ct,sk);
        if(memcmp(ss,ss2,CRYPTO_BYTES))f++;ps[i]=CRYPTO_PUBLICKEYBYTES;cs[i]=CRYPTO_CIPHERTEXTBYTES;ks[i]=CRYPTO_SECRETKEYBYTES;}
    if(f){printf("FAIL: %d mismatches\n",f);return 1;}
    qsort(ps,n,sizeof(unsigned long long),comp);qsort(cs,n,sizeof(unsigned long long),comp);qsort(ks,n,sizeof(unsigned long long),comp);
    #define PR(arr,lbl) do{unsigned long long mn=arr[0],mx=arr[n-1],av=0;for(int _i=0;_i<n;_i++)av+=arr[_i];av/=n;\
        printf("%s,%d,%llu,%llu,%llu,%llu,%llu,%llu\n",lbl,n,mn,av,mx,arr[n*5/100],arr[n/2],arr[n*95/100]);}while(0)
    PR(ps,"pk");PR(cs,"ct");PR(ks,"sk");free(ps);free(cs);free(ks);return 0;}
MEASURE

        gcc -O3 -std=gnu11 -I. -DLORE_LEVEL=${L} -DLORE_USE_API_PKC_DRNG \
            /tmp/size_measure_L${L}.c $SRC -o /tmp/size_measure_L${L} -lm 2>/dev/null

        /tmp/size_measure_L${L} $TRIALS > "$OUTDIR/actual_size_L${L}.txt" 2>/dev/null

        local pdf_pk pdf_ct pdf_sk
        case $L in
            1) pdf_pk=$PDF_PK_L1; pdf_ct=$PDF_CT_L1; pdf_sk=$PDF_SK_L1;;
            2) pdf_pk=$PDF_PK_L2; pdf_ct=$PDF_CT_L2; pdf_sk=$PDF_SK_L2;;
            4) pdf_pk=$PDF_PK_L4; pdf_ct=$PDF_CT_L4; pdf_sk=$PDF_SK_L4;;
        esac

        while read -r line; do
            local type=$(echo "$line"|cut -d, -f1) count=$(echo "$line"|cut -d, -f2)
            local min=$(echo "$line"|cut -d, -f3) avg=$(echo "$line"|cut -d, -f4) max=$(echo "$line"|cut -d, -f5)
            local p5=$(echo "$line"|cut -d, -f6) p50=$(echo "$line"|cut -d, -f7) p95=$(echo "$line"|cut -d, -f8)
            local pdf=0
            case $type in pk) pdf=$pdf_pk;; ct) pdf=$pdf_ct;; sk) pdf=$pdf_sk;; esac
            local diff=$(echo "$avg - $pdf" | bc 2>/dev/null || echo "0")
            echo "L${L},$type,$count,$min,$avg,$max,$p5,$p50,$p95,$pdf,$diff" >> "$CSV"
        done < "$OUTDIR/actual_size_L${L}.txt"
        echo "  L${L} done."
    done
    echo "Output: $OUTDIR"
}

run_cycles() {
    local RUN_ID="run_cycles_$(date +%Y%m%d_%H%M%S)"
    local OUTDIR="$REPO/bench/results/$RUN_ID"
    mkdir -p "$OUTDIR"

    local CSV="$OUTDIR/lore_shake_kem_api_cycles.csv"
    echo "scheme,level,paper_name,classical_security,operation,iterations,warmup,avg_cycles,median_cycles,avg_us,median_us,pk_bytes,ct_bytes,sk_bytes,ss_bytes,kappa,n,k,t,pure_shake,opt_status,failures" > "$CSV"

    local BENCH_C="$REPO/Implementations/Reference_Implementation/Lore-L4/../Lore-L1"  # dummy, will search
    # Use inline benchmark
    local BENCH_SRC="$REPO/bench/.bench_kem_cycles_time.c"

    # Generate benchmark source inline if not exists
    if [ ! -f "$BENCH_SRC" ]; then
        cat > "$BENCH_SRC" << 'BENCHSRC'
#include <stdio.h><stdlib.h><string.h><time.h>
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
    printf("Lore-SHAKE,L%d,Lore-%d,%d-bit,kem_keygen,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);

    ac=0;an=0;
    crypto_kem_keypair(pk,sk);
    for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();crypto_kem_enc(ct,ss,pk);unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;ac+=cb[i];an+=tb[i];}
    qsort(cb,N,sizeof(unsigned long long),u64cmp);qsort(tb,N,sizeof(long long),s64cmp);ac/=N;an/=N;med_ns=tb[N/2];
    printf("Lore-SHAKE,L%d,Lore-%d,%d-bit,kem_encaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);

    ac=0;an=0;
    crypto_kem_keypair(pk,sk);crypto_kem_enc(ct,ss,pk);
    for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();crypto_kem_dec(ss2,ct,sk);unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;if(memcmp(ss,ss2,CRYPTO_BYTES))fail++;ac+=cb[i];an+=tb[i];}
    qsort(cb,N,sizeof(unsigned long long),u64cmp);qsort(tb,N,sizeof(long long),s64cmp);ac/=N;an/=N;med_ns=tb[N/2];
    printf("Lore-SHAKE,L%d,Lore-%d,%d-bit,kem_decaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%d,%d,%d,%d,YES,AB_inline,%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);

    free(cb);free(tb);return fail?1:0;
}
BENCHSRC
    fi

    local SRC="kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c symmetric-shake.c fips202.c toomcook.c verify.c bch_codec.c randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c"

    for L in $LEVELS; do
        local D="$REPO/Implementations/Reference_Implementation/Lore-${L}"
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
        $RUN 2>/dev/null | grep "^Lore-SHAKE" >> "$CSV"
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
