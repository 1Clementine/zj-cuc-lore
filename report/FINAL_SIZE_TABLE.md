# Lore Final Size Table

## Actual Size Definition

Actual serialized size is measured by:

- parsing serialized RSP/KAT hex strings and computing `hex_length / 2`;
- instrumenting the pack functions and recording `bytes_written` before padding.

The two methods agree. Therefore, `CRYPTO_*BYTES` and `LORE_*BYTES` should be interpreted as maximum buffer sizes, not actual serialized sizes.

## 10000-Trial Actual Size Statistics

| Level | Type | Count | Min | Avg | Max | P5 | P50 | P95 | PDF | Avg-PDF | Status |
|--- |--- |---:|---:|---:|---:|---:|---:|---:|---:|---:|--- |
| L1 | PK | 10000 | 544 | 545.00 | 546 | 545 | 545 | 545 | 545 | +0.00 | MATCH |
| L1 | CT | 20000 | 640 | 641.00 | 642 | 641 | 641 | 641 | 641 | +0.00 | MATCH |
| L1 | SK | 10000 | 820 | 821.01 | 822 | 821 | 821 | 821 | 821 | +0.01 | MATCH |
| L2 | PK | 10000 | 1056 | 1057.40 | 1059 | 1057 | 1057 | 1058 | 1058 | -0.60 | close |
| L2 | CT | 20000 | 1152 | 1153.40 | 1155 | 1153 | 1153 | 1154 | 1153 | +0.40 | close |
| L2 | SK | 10000 | 1940 | 1941.41 | 1943 | 1941 | 1941 | 1942 | 1942 | -0.59 | close |
| L3 | PK | 10000 | 1761 | 1761.94 | 1764 | 1761 | 1762 | 1763 | 1763 | -1.06 | close |
| L3 | CT | 20000 | 1921 | 1921.95 | 1924 | 1921 | 1922 | 1923 | 1921 | +0.95 | close |
| L3 | SK | 10000 | 3703 | 3703.94 | 3706 | 3703 | 3704 | 3705 | 3704 | -0.06 | MATCH |
| L4 | PK | 10000 | 2625 | 2626.69 | 2629 | 2626 | 2627 | 2628 | 2626 | +0.69 | close |
| L4 | CT | 20000 | 2881 | 2882.68 | 2885 | 2882 | 2883 | 2884 | 2886 | **-3.32** | **MISMATCH** |
| L4 | SK | 10000 | 5371 | 5372.68 | 5375 | 5372 | 5373 | 5374 | 5373 | -0.32 | MATCH |

## L4 CT Analysis

For L4 ciphertext:

- measured CT range: 2881-2885 bytes;
- measured CT average: 2882.68 bytes;
- PDF Table 2 CT value: 2886 bytes.

The PDF value is outside the observed 10000-trial range. Therefore, the L4 CT discrepancy is not a small-sample effect.

The measured L4 CT follows:

```
CT = 2880 + overflow_bytes
2880 = 3 * 768 + 3 * 96 + 96 + 192
```

where the average measured overflow_bytes is about 2.68.

To reach PDF CT = 2886, the encoding would need overflow_bytes = 6, which is much larger than the observed average and outside the 10000-trial maximum.

Therefore, the current implementation suggests that PDF Table 2 L4 CT = 2886 may have been computed with a different formula, a different parameter assumption, or contains a table inconsistency.

## Buffer Size Comparison

| Level | Actual PK avg | Buffer PK | Actual CT avg | Buffer CT | Actual SK avg | KEM SK Buffer |
|--- |---:|---:|---:|---:|---:|---:|
| L1 | 545.00 | 610 | 641.00 | 706 | 821.01 | 2108 |
| L2 | 1057.40 | 1186 | 1153.40 | 1282 | 1941.41 | 4518 |
| L3 | 1761.94 | 1954 | 1921.95 | 2114 | 3703.94 | 7976 |
| L4 | 2626.69 | 2914 | 2882.68 | 3170 | 5372.68 | 11672 |

## Conclusions

1. `CRYPTO_*BYTES` and `LORE_*BYTES` are maximum buffer sizes, not actual serialized sizes.
2. The true serialized size should be measured by pack-function `bytes_written` before padding, or by RSP/KAT hex length divided by 2.
3. PK and SK logical sizes are consistent with the PDF Table 2 values, up to small variation from variable-length overflow-bit encoding.
4. CT sizes for L1-L3 are consistent with the PDF Table 2 values.
5. **L4 CT is not consistent with PDF Table 2**: measured average is 2882.68 bytes, while the PDF reports 2886 bytes (outside observed range).
6. The L4 CT mismatch should be explicitly reported rather than described as a match.
7. The likely explanation is a PDF table/formula/parameter inconsistency for L4 CT, not a code-size issue.
