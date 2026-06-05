# Lore SHAKE256 Branch Report

## Version

- Version directory: /home/syh/Work/zj-cuc-lore
- Intended branch: SHAKE256

## KEM FO Layer

- H = SHA3-256
- G = SHAKE256
- KDF = SHAKE256

## Encaps / Decaps Logic

- Encaps: G(H(m) || H(pk))
- Decaps: G(H(m') || H(pk))
- Success: KDF(Kbar' || H(c'))
- Failure: KDF(z || H(c'))

## KAT

- report/KAT_SHAKE/
- L1: PK=610, CT=706, SK=2108, SS=32
- L2: PK=1186, CT=1282, SK=4518, SS=32
- L3: PK=1954, CT=2114, SK=7976, SS=32
- L4: PK=2914, CT=3170, SK=11672, SS=32

## Performance

- report/performance/
- report/performance_tests/

Note: KAT sizes are implementation fixed API sizes, not expected transmission sizes.
