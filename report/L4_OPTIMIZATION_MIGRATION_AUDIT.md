# L4 Optimization Migration Audit

## 1. Context

- **Current SHAKE256 branch**: only Lore-L4 (256-bit), pure Keccak/SHAKE FIPS202.
- **L1 summary**: `docs/archive/Lore_shake_L1_reference_optimization_summary.md` — historical L1 optimization note. Not the current paper baseline.
- **This report**: judges each L1 optimization for L4 applicability.

## 2. L4 Parameter Audit

| Parameter | L1 (archived) | L4 (active) | Compatible? |
|---|---:|---:|---|
| LORE_Q | 257 | 257 | YES |
| LORE_T | 2 | 4 | NO |
| LORE_N | 512 | 768 | partial |
| LORE_K | 1 | 3 | NO |
| LORE_KAPPA | 128 | 512 | NO |
| LORE_R_BITS | 0 | 1 | NO |
| LORE_HWT_TOTAL | 102 | 340 | NO |
| fips202 | pure SHAKE | pure SHAKE | DONE |
| KAT | PASS | PASS | DONE |

## 3. Optimization Transfer Table

| # | Optimization from L1 note | L1 assumption | Applicable to L4? | Action |
|---|---|---|---|---|
| 1 | Pure SHAKE FIPS202 path | Replace SM3/pseudoXOF wrapper | YES | **DONE** — L4 already uses pure Keccak/SHAKE |
| 2 | No SM3/auxfunc in benchmark | Remove sm3.o/auxfunc.o from non-KAT builds | YES | **DOCUMENTED** — build_bench.sh conditionally includes sm3.c (only if present); auxfunc.c still linked for DRNG compat. Not a correctness issue for SHAKE perf. |
| 3 | q=257 fast reduction | Barrett reduction for mod 257 | YES (q=257 in L4) | **COMPATIBLE** — L4 reduce.c already has `barrett_reduce_scalar` and `barrett_reduce`. No new code needed. |
| 4 | 12-bit packed rejection sampling | q=257 sampling efficiency | YES (q=257) | **NOT IMPLEMENTED** — would change SHAKE consumption pattern. KAT would change. Deferred for separate verification. |
| 5 | t=2 bitset | t coefficients need only 1 bit | NO (t=4 in L4) | **NOT APPLICABLE** — L4 uses t=4 (2 bits per coefficient). Bitset path is L1-specific. |
| 6 | poly_getnoise_uniform(t=2) zero shortcut | t=2 => output always 0 | NO (t=4 in L4) | **NOT APPLICABLE** — L4 t=4 has non-trivial noise distribution. |
| 7 | q-only pointwise multiplication | t part is 0 or unused in L1 CRT | NO (t=4, CRT needs t part) | **NOT APPLICABLE** — L4 CRT path uses t=4 which requires t-part multiplication. |
| 8 | delta direct computation | L1 delta path optimization | UNCERTAIN | **NOT IMPLEMENTED** — structural change, needs L4-specific proof. Deferred. |
| 9 | NTT/static inline improvements | fqmul, barrett_reduce, basemul inline | YES | **COMPATIBLE** — general C optimization. No change needed; current code already has `static inline` for hot functions. |

## 4. Implemented Changes

**No semantic source optimization implemented in this round.**

Only documentation/audit cleanup was applied:
- Moved `Lore_shake_L1_reference_optimization_summary.md` to `docs/archive/`
- Verified L4 fips202 purity (pure SHAKE, no pseudoXOF/sm3hash)
- Verified L4 KAT PASS (matches Test_Vectors)
- Verified build_bench.sh correctly handles conditional sm3.c inclusion

## 5. Validation

| Check | Result |
|---|---|
| L4 fips202 purity | PASS (0 pseudoXOF/sm3hash/pseudohash refs) |
| L4 KAT | PASS (120 lines, matches Test_Vectors) |
| L4 perf (pure SHAKE generic) | keypair 388.6us, encaps 516.4us, decaps 671.5us |
| failures | 0 |

## 6. Recommendation

1. **Current paper data**: use the validated stable baseline — pure SHAKE L4 Reference performance (keypair 388.6us, encaps 516.4us, decaps 671.5us). This is not claimed as globally optimal.
2. **q=257 Barrett reduction** already exists in L4 reduce.c.
3. **NTT hot functions** are already `static inline` where applicable.
4. **12-bit packed rejection and delta direct computation** are deferred — they may change KAT or require structural proof.
5. **Do not mix L1 optimized numbers** into the L4-only SHAKE branch. The L1 summary is archived historical reference.
6. **t=2 / bitset / q-only / noise-zero optimizations are L1-specific** and cannot be directly migrated to L4 (t=4).
