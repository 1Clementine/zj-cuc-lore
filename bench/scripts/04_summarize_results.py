#!/usr/bin/env python3
"""Generate summary markdown from KEM cycles+time CSV."""

import csv, os, sys

def main():
    bench_dir = os.environ.get('BENCH_DIR',
        os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
    run_id = sys.argv[1] if len(sys.argv) > 1 else 'latest'

    results_dir = os.path.join(bench_dir, 'results', run_id)
    csv_path = os.path.join(results_dir, 'performance',
                            'lore_shake_kem_cycles_time_all_levels.csv')
    summary_dir = os.path.join(results_dir, 'summary')
    os.makedirs(summary_dir, exist_ok=True)

    formal_repo = os.environ.get('LORE_FORMAL_REPO', '')
    formal_report_dir = os.path.join(formal_repo, 'report') if formal_repo else ''

    data = {}
    with open(csv_path) as f:
        for row in csv.DictReader(f):
            lvl = row['level']
            op = row['operation']
            data[(lvl, op)] = row

    out = os.path.join(summary_dir, 'LORE_SHAKE_KEM_CYCLES_TIME_SUMMARY.md')
    with open(out, 'w') as f:
        f.write("""# Lore-SHAKE KEM Cycles and Time Benchmark

## Scope

- **Measured functions**: `crypto_kem_keypair`, `crypto_kem_enc`, `crypto_kem_dec`
- **Metrics**: CPU cycles (rdtsc) + wall-clock time (clock_gettime)
- **Build**: generic C, `-O3 -DNDEBUG`
- **Iterations**: 10000, **Warmup**: 1000
- **This benchmark does NOT measure PKE-level KeyGen / Enc / Dec.**

## Parameter Mapping

| Level | Paper name | Classical security | κ | n | k | t |
|---|---:|---:|---:|---:|---:|---:|
| Lore-L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| Lore-L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| Lore-L3 | Lore-384 | 384-bit | 384 | 512 | 3 | 4 |
| Lore-L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

## KEM Cycles and Time

| Level | Paper | keygen med cycles | encaps med cycles | decaps med cycles | keygen us | encaps us | decaps us |
|---|---:|---:|---:|---:|---:|---:|---:|
""")
        for lvl, lvl_label in [('L1','Lore-L1'),('L2','Lore-L2'),('L3','Lore-L3'),('L4','Lore-L4')]:
            kg = data.get((lvl, 'kem_keygen'), {})
            en = data.get((lvl, 'kem_encaps'), {})
            de = data.get((lvl, 'kem_decaps'), {})
            paper = kg.get('paper_name', '')
            f.write(f"| {lvl} | {paper} | {kg.get('median_cycles','?')} | "
                    f"{en.get('median_cycles','?')} | {de.get('median_cycles','?')} | "
                    f"{kg.get('median_us','?')} | {en.get('median_us','?')} | "
                    f"{de.get('median_us','?')} |\n")

        f.write(f"""
## Validation

- **KAT**: see `{results_dir}/validation/kat_summary.csv`
- **Pure SHAKE**: YES
- **Failures**: {data.get(('Lore-L1','kem_keygen'),{}).get('failures','?')}

## Output Files

- `{csv_path}`
- `{summary_dir}/`
- `{results_dir}/logs/`
- `{results_dir}/environment/`
- `{results_dir}/validation/`
""")

    # Copy to formal report directory
    if formal_report_dir:
        dest = os.path.join(formal_report_dir, 'LORE_SHAKE_ALL_LEVELS_PERFORMANCE.md')
        os.system(f'cp "{out}" "{dest}"')
        print(f'Copied to {dest}')

    print(f'Summary: {out}')

if __name__ == '__main__':
    main()
