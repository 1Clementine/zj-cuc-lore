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
