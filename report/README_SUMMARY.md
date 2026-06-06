# Lore SM3 Branch Report

## Version

- Version directory: /home/syh/Work/zj-cuc-lore-sm3
- Intended branch: SM3

## KEM FO Layer

- H = SM3
- G = SM3-KDF
- KDF = SM3-KDF
- SM3-KDF = SM3(input || counter_be), counter from 0x00000001

## Encaps / Decaps Logic

- Encaps: G(H(m) || H(pk))
- Decaps: G(H(m') || H(pk))
- Success: KDF(Kbar' || H(c'))
- Failure: KDF(z || H(c'))

## KAT

- report/KAT_SM3/
- L1: actual PK=545, CT=641, SK=821, SS=32 (buffer PK=610, CT=706, SK=2108)
- L2: actual PK~1057, CT~1153, SK~1942, SS=32 (buffer PK=1186, CT=1282, SK=4518)
- L3: actual PK~1762, CT~1922, SK~3704, SS=32 (buffer PK=1954, CT=2114, SK=7976)
- L4: actual PK~2627, CT~2883, SK~5373, SS=32 (buffer PK=2914, CT=3170, SK=11672). L4 CT is ~3 bytes below PDF 2886 (see FINAL_SIZE_TABLE.md)

## Performance

- report/performance/
- report/performance_tests/

Note: KAT sizes are implementation fixed API sizes, not expected transmission sizes.
