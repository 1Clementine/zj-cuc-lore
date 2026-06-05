# Lore Final Report

## Repository Paths

| Version | Source Path |
|---------|-------------|
| SHAKE/SHA3 | `/home/syh/Work/zj-cuc-lore` |
| SM3        | `/home/syh/Work/zj-cuc-lore-sm3` |

## Unified Report Path

`/home/syh/Work/zj-cuc-lore/report/`

## KEM FO Hash Functions

### SHAKE/SHA3 Version
- H   = SHA3-256
- G   = SHAKE256
- KDF = SHAKE256

### SM3 Version
- H   = SM3
- G   = SM3-KDF
- KDF = SM3-KDF
- SM3-KDF = SM3(input || counter_be), counter starts from 0x00000001

## Encaps/Decaps Logic (Both Versions)

- Encaps:  G(H(m) || H(pk))
- Decaps:  G(H(m') || H(pk))
- Success: KDF(Kbar' || H(c'))
- Failure: KDF(z || H(c'))

## KAT Status

| Version | L1 | L2 | L3 | L4 |
|---------|----|----|----|-----|
| SHAKE/SHA3 Ref=Opt | PASS | PASS | PASS | PASS |
| SM3 Ref=Opt        | PASS | PASS | PASS | PASS |

## KAT Sizes

| Level | PK | CT | SK | SS |
|-------|-----|------|------|----|
| L1 | 610 | 706 | 2108 | 32 |
| L2 | 1186 | 1282 | 4518 | 32 |
| L3 | 1954 | 2114 | 7976 | 32 |
| L4 | 2914 | 3170 | 11672 | 32 |

Note: These are implementation fixed API sizes (worst-case allocation),
not the expected transmission sizes in Table 2 of the specification.

## Performance Test Coverage

- 2 versions: SHAKE/SHA3 + SM3
- 2 implementations: Reference + Optimized
- 4 levels: L1/L2/L3/L4
- 3 operations: keypair / encaps / decaps
- 48/48 configurations: all passed
- 0 failures, 0 shared-secret mismatches

Performance test scripts: `report/performance_tests/`
Performance CSV data: `report/performance/`

## Known Notes

1. Size: Code uses worst-case fixed-size allocation. PK/CT/SK sizes are larger
   than the PDF Table 2 expected transmission sizes. This exists from the original
   implementation and is not introduced by modifications.

2. SM3-KDF: Uses GM/T 0004-2012 standard counter-mode construction:
   SM3(input || counter_be) with 32-bit big-endian counter from 0x00000001.

3. S_R rounding (t=4): Uses Section 2.6 definition - breaking chooses the
   smaller representative (3 maps to 0).

## Report Directory Structure

```
report/
  README_SUMMARY.md
  KAT_SHAKE/          — SHAKE/SHA3 KAT for L1-L4
  KAT_SM3/            — SM3 KAT for L1-L4
  performance/        — CSV performance results
  performance_tests/  — bench_kem.c + build/run scripts
  code_checks/        — hash calls, md5sum, sizes, Ref/Opt diff records
```
