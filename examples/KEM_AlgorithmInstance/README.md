# KEM AlgorithmInstance Integration Example

This directory shows the expected integration points for a KEM algorithm instance.

Required files in a real algorithm instance directory:

- `KAT_KEM.c`
- `KEM_AlgorithmInstance.c`
- `KEM_AlgorithmInstance.h`
- `auxfunc.c`
- `auxfunc.h`
- `drng.c`
- `drng.h`
- algorithm implementation source files
- `Makefile`

The official KAT driver calls:

- `kem_get_pk_len_bytes()`
- `kem_get_sk_len_bytes()`
- `kem_get_ct_len_bytes()`
- `kem_get_ss_len_bytes()`
- `kem_keygen(...)`
- `kem_enc(...)`
- `kem_dec(...)`

`KAT_KEM.c` generates 10 KAT cases using official fixed seeds and writes:

- `Count`
- `Seed_Len`
- `Seed`
- `PK_Len`
- `PK`
- `SK_Len`
- `SK`
- `CT_Len`
- `CT`
- `SS_Len`
- `SS`

Reference-generated KAT files should be treated as golden baselines for later optimized implementations.
