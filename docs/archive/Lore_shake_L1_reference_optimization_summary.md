# Lore-shake Reference L1 优化总结

本文总结当前对 `Lore-shake` reference 实现的 L1 等级优化点，目的是帮助在原版本上复现同类性能优化。重点是保持算法语义不变，先删除冗余路径，再优化采样、多项式乘法、NTT 和构建方式。

## 目标

优化对象：

- `Lore-shake/Implementations/Reference_Implementation/Lore-L1`
- 只关注 L1 参数
- 对比对象为 liboqs 中的 ML-KEM-512 reference 实现

优化后的当前测试结果，`10000` 次运行、`1000` 次 warmup：

| 操作 | Lore-shake Reference L1 |
|---|---:|
| keypair | 13.69 us |
| encaps | 25.33 us |
| decaps | 32.82 us |

与 liboqs ML-KEM-512 reference 对比：

| 操作 | Lore / ML-KEM-512 |
|---|---:|
| keypair | 1.16x slower |
| encaps | 2.11x slower |
| decaps | 2.30x slower |

## 优化优先级

建议按照下面顺序实施：

1. 先修正 SHAKE/KDF 路径，避免实际走到 SM3 或伪 XOF。
2. 再优化 `q=257` 的采样和模约化。
3. 然后优化 L1 特有的 `t=2` 路径。
4. 最后处理多项式乘法、delta 路径和 NTT 小优化。

## 1. 替换 SHAKE/KDF 路径

原版本中 `Lore-shake` 虽然名字是 SHAKE 版本，但部分路径仍可能经过较慢的伪 XOF 或 SM3 风格实现，导致 SHAKE 的优势没有体现出来。

优化方式：

- 使用真正的 FIPS202 Keccak/SHAKE 实现。
- 不依赖 OpenSSL EVP。
- `hash_h` 使用 `shake256(out, 32, in, inlen)`。
- `hash_g` 使用 `shake256(out, 64, in, inlen)`。
- KDF 统一使用 `shake256(out, outlen, in, inlen)`。
- benchmark 构建时移除无关的 `sm3.c`、`auxfunc.c` 等文件，避免误链接和误调用。

这是收益最大的第一步。

## 2. q=257 快速模约化

L1 中 `q=257`，不需要频繁使用通用 `% q`。

优化方式：

- 增加针对 `q=257` 的快速 Barrett reduction。
- 在 `poly_add`、`rej_uniform_q`、`q_split pack` 等路径中使用快速 reduce。
- 尽量避免热路径中的除法和取模。

## 3. q 采样改为 12-bit packed rejection

原版本如果用 16-bit candidate 采样 `q=257`，会浪费 SHAKE 输出。

优化方式：

- 每 3 字节解析出两个 12-bit candidate。
- 接受范围设为 `< 3855`，因为 `3855 = 15 * 257`。
- 接受后再对 `257` 做 reduce。

这样平均每个有效 `q` 系数大约消耗 `12.75` bit，比 16-bit candidate 更合理。对 keypair 和 encaps 都有可见收益。

伪代码：

```c
while (ctr < len && pos + 3 <= buflen) {
    uint32_t t = buf[pos] | (buf[pos + 1] << 8) | (buf[pos + 2] << 16);
    uint16_t val0 = t & 0xFFF;
    uint16_t val1 = (t >> 12) & 0xFFF;
    pos += 3;

    if (val0 < 3855) r[ctr++] = reduce_mod_257(val0);
    if (ctr < len && val1 < 3855) r[ctr++] = reduce_mod_257(val1);
}
```

## 4. t=2 路径使用 bitset

L1 中 `t=2`，t 环上的系数只需要 1 bit 表示，不需要用 `int16_t[512]` 存 dense 多项式。

优化方式：

- 在 L1 的 `poly_crt` 中增加 `uint64_t t_bits[N / 64]`。
- matrix 生成时直接从 SHAKE buffer 中提取 t 的 bitset。
- `poly_sparse_mul_modt` 对 `mod 2` 使用 bit-level XOR/rotate 累加。
- 避免把 t 多项式展开成 dense `int16_t[512]` 再做普通乘法。

注意：matrix 生成中要先保存 t 的 bitset，再进行 q 采样 refill，否则 q 采样 refill 可能覆盖原始 buffer，导致 t 部分来源不稳定。

## 5. `poly_getnoise_uniform(t=2)` 的等价优化

这一点需要谨慎表述。

当前 reference 代码中，`poly_getnoise_uniform` 的定义是从：

```text
U[-(t - 1) / 2, (t - 1) / 2]
```

采样。按照 C 的整数除法，当 `t=2` 时：

```text
max_val = (2 - 1) / 2 = 0
min_val = 0
range = 1
t_coeff = val % 1 + 0 = 0
```

所以原实现虽然调用了 PRF 生成随机字节，但最终每个系数仍然恒为 `0`。因此可以把这条路径提前返回全 0，用来删除冗余 PRF 和循环。

重要注意事项：

- 这不是“为了加速而删除安全噪音”。
- 这是对当前 reference 语义的等价优化。
- 如果论文或规格要求 `t=2` 时使用非平凡 binary noise，例如 `{0,1}` 或 Bernoulli noise，那么当前 reference 本身就没有实现该语义，不能直接应用这个优化。

建议转述为：

> 对 `poly_getnoise_uniform(t=2)` 的优化不是删除安全噪音，而是发现当前 reference 的整数采样公式在 `t=2` 下支持集退化为 `{0}`，因此原实现生成随机字节后结果仍恒为 0。优化仅去掉冗余 PRF 和循环。若规范要求 `t=2` 使用非平凡二元噪音，则需要重新检查参数设计和安全证明。

## 6. q-only pointwise multiplication

有些 CRT 点乘路径中，t 部分实际为 0 或后续不会使用。

优化方式：

- 增加 L1 专用 `pointwise_acc_montgomery_qonly()`。
- encaps 中 `b_ntt * sp_crt` 只计算 q 部分。
- decaps 中 `cu_ntt * s_crt` 只计算 q 部分。
- 避免无意义的 t 部分乘法。

## 7. delta 路径直接计算

原版本 delta 路径通常会先做完整 CRT 乘法，再从 t 部分计算补偿值。

优化方式：

- L1 中 q 部分照常 NTT 点乘。
- t 部分用 bitset + sparse positions 直接计算 `mod 2` 结果。
- 直接生成 `delta_q_std`。
- 避免构造完整 dense t polynomial 后再扫描。

这一点对 keypair 和 encaps 有帮助，因为二者都会生成 `b` 或 `cu` 并处理 delta。

## 8. NTT 和小函数内联

优化方式：

- 将 `fqmul`、Montgomery reduce、Barrett reduce 等热路径函数改为 `static inline`。
- 展开 `basemul4`。
- 不在头文件暴露不必要的内部函数。

这类优化单点收益不一定很大，但可以降低 reference C 中函数调用和重复 reduce 的开销。

## 9. 构建与测试注意事项

构建 benchmark 时建议：

- 不定义会绕到 API PKC DRNG 的宏，除非测试目标就是该路径。
- 源文件列表只包含 SHAKE reference 所需文件。
- 不链接 SM3 相关文件。
- 使用固定迭代次数，例如 `10000` 次运行、`1000` 次 warmup。
- 每次优化后至少跑 KEM 正确性测试，确保 failures 为 `0`。

当前测试中 KEM failures 为 `0`。

## 结论

这些优化后，Lore-shake L1 reference 的 keypair 已经接近 ML-KEM-512 reference，但 encaps 和 decaps 仍明显更慢。剩余差距主要来自 Lore 自身结构：

- `N=512` 多项式规模；
- CRT 路径；
- delta 补偿路径；
- q/t 双环结构；
- `q_split` 打包与解包；
- 多项式乘法结构和数据布局。

因此，后续如果还要继续追 ML-KEM-512 reference 的性能，重点不应只放在随机比特数量，而应继续做结构性优化，例如减少 CRT 中间态、合并 pack/decompress 流程、进一步压缩 delta 路径，以及考虑 SIMD/AVX2 的 SHAKE 和 NTT 实现。
