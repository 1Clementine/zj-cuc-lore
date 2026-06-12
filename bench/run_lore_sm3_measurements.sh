#!/usr/bin/env bash
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
MODE="${1:-usage}"

LEVELS="${LEVELS:-L1 L2 L3 L4}"
TRIALS="${TRIALS:-10000}"
ITERS="${ITERS:-10000}"
WARMUP="${WARMUP:-1000}"
CORE="${CORE:-0}"
IMPLS="${IMPLS:-ref opt}"

declare -A PAPER SEC KAPPA NN KK TT
PAPER[L1]="Lore-128";  SEC[L1]="128-bit"; KAPPA[L1]=128; NN[L1]=512; KK[L1]=1; TT[L1]=2
PAPER[L2]="Lore-256";  SEC[L2]="256-bit"; KAPPA[L2]=256; NN[L2]=512; KK[L2]=2; TT[L2]=2
PAPER[L3]="Lore-384";  SEC[L3]="384-bit"; KAPPA[L3]=384; NN[L3]=512; KK[L3]=3; TT[L3]=4
PAPER[L4]="Lore-512";  SEC[L4]="512-bit"; KAPPA[L4]=512; NN[L4]=768; KK[L4]=3; TT[L4]=4

PDF_PK_L1=545;  PDF_CT_L1=641;  PDF_SK_L1=821
PDF_PK_L2=1058; PDF_CT_L2=1153; PDF_SK_L2=1942
PDF_PK_L3=1763; PDF_CT_L3=1921; PDF_SK_L3=3704
PDF_PK_L4=2626; PDF_CT_L4=2886; PDF_SK_L4=5373

# AVX2 detection
has_avx2() { grep -q avx2 /proc/cpuinfo 2>/dev/null; }

usage() {
    echo "Usage: $0 {size|cycles|all}"
    echo "  size    Measure actual serialized sizes"
    echo "  cycles  Run KEM API benchmark (cycles + time)"
    echo "  all     Run both"
    echo "Defaults: LEVELS='$LEVELS' TRIALS=$TRIALS ITERS=$ITERS WARMUP=$WARMUP CORE=$CORE IMPLS='$IMPLS'"
    exit 1
}

src_list() { echo "kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c symmetric-shake.c fips202.c sm3.c toomcook.c verify.c bch_codec.c randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c"; }

run_size() {
    local RUN_ID="run_size_$(date +%Y%m%d_%H%M%S)"
    local OUTDIR="$REPO/bench/results/$RUN_ID"
    mkdir -p "$OUTDIR"
    local CSV="$OUTDIR/lore_sm3_actual_size_statistics.csv"
    echo "implementation,level,type,count,min,avg,max,p5,p50,p95,pdf,avg_minus_pdf" > "$CSV"

    for IMPL in $IMPLS; do
        [ "$IMPL" = "opt" ] && ! has_avx2 && { echo "[skip] optimized: AVX2 not supported"; continue; }
        local IMPL_DIR="Reference_Implementation"
        [ "$IMPL" = "opt" ] && IMPL_DIR="Optimized_Implementation"

        for L in $LEVELS; do
            local D="$REPO/Implementations/$IMPL_DIR/Lore-${L}"
            local LABEL="${IMPL}_L${L}"
            echo "===== Size: $LABEL ($TRIALS trials) ====="
            cd "$D"

            cat > /tmp/sm3_size_$$.c << 'MEASURE'
#include <stdio.h><stdlib.h><string.h>
#include "api.h"
#include "drng.h"
DRNG_ctx drng_algorithm;
static void sd(){unsigned char n[64];memset(n,0,64);memcpy(n,"sm3_size_",9);init_random_number(&drng_algorithm,n,64);}
int cmp(const void*a,const void*b){unsigned long long x=*(unsigned long long*)a,y=*(unsigned long long*)b;return(x>y)?1:(x<y)?-1:0;}
int main(int c,char**v){
    int n=(c>1)?atoi(v[1]):10000;
    unsigned char pk[CRYPTO_PUBLICKEYBYTES],sk[CRYPTO_SECRETKEYBYTES],ct[CRYPTO_CIPHERTEXTBYTES],ss[CRYPTO_BYTES],ss2[CRYPTO_BYTES];
    unsigned long long *ps=malloc(n*8),*cs=malloc(n*8),*ks=malloc(n*8);sd();int f=0;
    for(int i=0;i<n;i++){crypto_kem_keypair(pk,sk);crypto_kem_enc(ct,ss,pk);crypto_kem_dec(ss2,ct,sk);
        if(memcmp(ss,ss2,CRYPTO_BYTES))f++;ps[i]=CRYPTO_PUBLICKEYBYTES;cs[i]=CRYPTO_CIPHERTEXTBYTES;ks[i]=CRYPTO_SECRETKEYBYTES;}
    if(f){printf("FAIL:%d\n",f);return 1;}
    qsort(ps,n,8,cmp);qsort(cs,n,8,cmp);qsort(ks,n,8,cmp);
    #define PR(a,l) do{unsigned long long mn=a[0],mx=a[n-1],av=0;for(int _i=0;_i<n;_i++)av+=a[_i];av/=n;\
        printf("%s,%d,%llu,%llu,%llu,%llu,%llu,%llu\n",l,n,mn,av,mx,a[n*5/100],a[n/2],a[n*95/100]);}while(0)
    PR(ps,"pk");PR(cs,"ct");PR(ks,"sk");free(ps);free(cs);free(ks);return 0;}
MEASURE

            gcc -O3 -std=gnu11 -I. -DLORE_LEVEL=${L} -DLORE_USE_API_PKC_DRNG \
                /tmp/sm3_size_$$.c $(src_list) -o /tmp/sm3_size_bin_$$ -lm 2>/dev/null

            /tmp/sm3_size_bin_$$ $TRIALS > "$OUTDIR/actual_size_${LABEL}.txt" 2>/dev/null

            local pdf_pk pdf_ct pdf_sk
            case $L in
                1) pdf_pk=$PDF_PK_L1; pdf_ct=$PDF_CT_L1; pdf_sk=$PDF_SK_L1;;
                2) pdf_pk=$PDF_PK_L2; pdf_ct=$PDF_CT_L2; pdf_sk=$PDF_SK_L2;;
                3) pdf_pk=$PDF_PK_L3; pdf_ct=$PDF_CT_L3; pdf_sk=$PDF_SK_L3;;
                4) pdf_pk=$PDF_PK_L4; pdf_ct=$PDF_CT_L4; pdf_sk=$PDF_SK_L4;;
            esac
            while read -r line; do
                local tp=$(echo "$line"|cut -d, -f1) cnt=$(echo "$line"|cut -d, -f2)
                local mn=$(echo "$line"|cut -d, -f3) av=$(echo "$line"|cut -d, -f4) mx=$(echo "$line"|cut -d, -f5)
                local p5=$(echo "$line"|cut -d, -f6) p50=$(echo "$line"|cut -d, -f7) p95=$(echo "$line"|cut -d, -f8)
                local pdf=0
                case $tp in pk) pdf=$pdf_pk;; ct) pdf=$pdf_ct;; sk) pdf=$pdf_sk;; esac
                local diff=$(echo "$av - $pdf" | bc 2>/dev/null || echo "0")
                echo "$IMPL,$L,$tp,$cnt,$mn,$av,$mx,$p5,$p50,$p95,$pdf,$diff" >> "$CSV"
            done < "$OUTDIR/actual_size_${LABEL}.txt"
        done
    done
    echo "Output: $OUTDIR"
}

run_cycles() {
    local RUN_ID="run_cycles_$(date +%Y%m%d_%H%M%S)"
    local OUTDIR="$REPO/bench/results/$RUN_ID"
    mkdir -p "$OUTDIR"
    local CSV="$OUTDIR/lore_sm3_kem_api_cycles.csv"
    echo "scheme,implementation,level,paper_name,classical_security,operation,iterations,warmup,avg_cycles,median_cycles,avg_us,median_us,pk_bytes,ct_bytes,sk_bytes,ss_bytes,kappa,n,k,t,backend,opt_status,failures" > "$CSV"

    local BENCH_SRC="$REPO/bench/.sm3_bench_cycles.c"
    if [ ! -f "$BENCH_SRC" ]; then
        cat > "$BENCH_SRC" << 'BENCHSRC'
#include <stdio.h><stdlib.h><string.h><time.h>
#include "api.h"
#include "drng.h"
DRNG_ctx drng_algorithm;
static void sd(){unsigned char n[64];memset(n,0,64);memcpy(n,"sm3_bench_",10);init_random_number(&drng_algorithm,n,64);}
static unsigned long long rd(){unsigned int l,h;__asm__ volatile("rdtsc":"=a"(l),"=d"(h));return((unsigned long long)h<<32)|l;}
static long long ns(){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec*1000000000LL+ts.tv_nsec;}
int ucmp(const void*a,const void*b){unsigned long long x=*(unsigned long long*)a,y=*(unsigned long long*)b;return(x>y)?1:(x<y)?-1:0;}
int scmp(const void*a,const void*b){long long x=*(long long*)a,y=*(long long*)b;return(x>y)?1:(x<y)?-1:0;}
int main(void){
    sd();
    unsigned char pk[CRYPTO_PUBLICKEYBYTES],sk[CRYPTO_SECRETKEYBYTES],ct[CRYPTO_CIPHERTEXTBYTES],ss[CRYPTO_BYTES],ss2[CRYPTO_BYTES];
    int fail=0,W=WARMUP_VAL,N=ITERS_VAL;
    unsigned long long*cb=malloc(N*8);long long*tb=malloc(N*8);
    for(int i=0;i<W;i++){crypto_kem_keypair(pk,sk);crypto_kem_enc(ct,ss,pk);crypto_kem_dec(ss2,ct,sk);if(memcmp(ss,ss2,CRYPTO_BYTES))fail++;}
    unsigned long long ac;long long an,med_ns;
    #define BENCH(op,opname) do{ac=0;an=0;\
        for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();op;unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;ac+=cb[i];an+=tb[i];}\
        qsort(cb,N,8,ucmp);qsort(tb,N,8,scmp);ac/=N;an/=N;med_ns=tb[N/2];\
        printf("Lore-SM3," IMPL_STR ",L%d,Lore-%d,%d-bit," opname ",%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%zu,%d,%d,%d,%d,SM3," OPT_STR ",%d\n",\
            LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);\
    }while(0)
    crypto_kem_keypair(pk,sk); BENCH(crypto_kem_enc(ct,ss,pk),"kem_encaps");
    crypto_kem_enc(ct,ss,pk); for(int i=0;i<N;i++){long long t0=ns();unsigned long long c0=rd();crypto_kem_dec(ss2,ct,sk);unsigned long long c1=rd();long long t1=ns();cb[i]=c1-c0;tb[i]=t1-t0;ac+=cb[i];an+=tb[i];if(memcmp(ss,ss2,CRYPTO_BYTES))fail++;}
    qsort(cb,N,8,ucmp);qsort(tb,N,8,scmp);ac/=N;an/=N;med_ns=tb[N/2];
    printf("Lore-SM3," IMPL_STR ",L%d,Lore-%d,%d-bit,kem_decaps,%d,%d,%llu,%llu,%.2f,%.2f,%zu,%zu,%zu,%zu,%d,%d,%d,%d,SM3," OPT_STR ",%d\n",LORE_LEVEL,LORE_KAPPA,LORE_KAPPA,N,W,ac,cb[N/2],an/1000.0,med_ns/1000.0,(size_t)CRYPTO_PUBLICKEYBYTES,(size_t)CRYPTO_CIPHERTEXTBYTES,(size_t)CRYPTO_SECRETKEYBYTES,(size_t)CRYPTO_BYTES,LORE_KAPPA,LORE_N,LORE_K,LORE_T,fail);
    free(cb);free(tb);return fail?1:0;}
BENCHSRC
    fi

    for IMPL in $IMPLS; do
        [ "$IMPL" = "opt" ] && ! has_avx2 && { echo "[skip] optimized: AVX2 not supported"; continue; }
        local IMPL_DIR="Reference_Implementation"
        local IMPL_STR="ref"
        local OPT_STR="none"
        [ "$IMPL" = "opt" ] && IMPL_DIR="Optimized_Implementation" && IMPL_STR="opt" && OPT_STR="AVX2"

        for L in $LEVELS; do
            local D="$REPO/Implementations/$IMPL_DIR/Lore-${L}"
            local LABEL="${IMPL}_L${L}"
            echo "===== Cycles: $LABEL (${PAPER[$L]}) ====="
            cd "$D"

            local BIN="/tmp/sm3_bench_$$"
            gcc -O3 -DNDEBUG -std=gnu11 -D_POSIX_C_SOURCE=199309L \
                -DWARMUP_VAL=${WARMUP} -DITERS_VAL=${ITERS} \
                -DIMPL_STR="\"${IMPL_STR}\"" -DOPT_STR="\"${OPT_STR}\"" \
                -I. -DLORE_LEVEL=${L} -DLORE_USE_API_PKC_DRNG \
                "$BENCH_SRC" $(src_list) -o "$BIN" -lm 2>/dev/null

            local RUN="$BIN"
            [ -n "$CORE" ] && [ "$CORE" != "0" ] && RUN="taskset -c $CORE $BIN"

            # Run twice: first for output file, second for CSV (avoid tee issues)
            $RUN 2>/dev/null | tee "$OUTDIR/bench_${LABEL}.txt"
            $RUN 2>/dev/null | grep "^Lore-SM3" >> "$CSV"
            echo "  Done."
        done
    done
    echo "Output: $OUTDIR"
}

case "$MODE" in
    size)   run_size;;
    cycles) run_cycles;;
    all)    run_size; run_cycles;;
    *)      usage;;
esac
