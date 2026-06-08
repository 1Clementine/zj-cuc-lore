# KAT Template Branch

This branch contains the official KAT/API template files for public-key algorithm submission.

It is a template branch only. It does not contain a concrete Lore implementation.

## Purpose

This branch provides reusable files for integrating a KEM algorithm instance with the official KAT framework:

- official KAT driver
- official KEM API wrapper template
- official auxiliary hash/XOF functions
- official DRNG
- blank KAT template with fixed seeds
- integration script

## Official Requirements

The submission requirements specify that:

- implementation code should use the official programming interface from `API_PKC.zip`;
- hash and XOF functions should use the official auxiliary functions;
- KAT vectors should be generated using the official auxiliary programs and data;
- generated KAT files should be placed under `Test_Vectors/`.

## Directory Structure

```text
official_api/
  auxfunc.c
  auxfunc.h
  drng.c
  drng.h
  KAT_KEM.c
  KAT_SIG.c
  KAT_KEX.c
  KEM_AlgorithmInstance.c
  KEM_AlgorithmInstance.h
  KAT_KEM_AlgorithmInstance.txt

templates/
  KAT_KEM_AlgorithmInstance.txt

docs/
  public_key_algorithm_submission_requirements.pdf

scripts/
  install_kat_template_to_instance.sh

examples/
  KEM_AlgorithmInstance/
```

## KEM Integration

For each real KEM algorithm instance, copy the official files into the instance directory:

```bash
bash scripts/install_kat_template_to_instance.sh /path/to/Implementations/Reference_Implementation/<AlgorithmInstance>
```

Then implement the following API functions in `KEM_AlgorithmInstance.c/h`:

- `kem_get_pk_len_bytes()`
- `kem_get_sk_len_bytes()`
- `kem_get_ct_len_bytes()`
- `kem_get_ss_len_bytes()`
- `kem_keygen(...)`
- `kem_enc(...)`
- `kem_dec(...)`

## KAT Baseline Policy

Reference-generated KAT files should be treated as golden baselines.

Workflow:

1. Generate KAT from `Reference_Implementation`.
2. Save the generated vectors under `Test_Vectors/`.
3. For `Optimized_Implementation`, regenerate KAT in a temporary directory.
4. Compare optimized KAT against the reference baseline using `diff`.

Do not overwrite `Test_Vectors/` unless the algorithm specification, parameters, official auxiliary-function mapping, or serialization format intentionally changes.

## Branch Role

- `KAT-template`: reusable official KAT/API template branch.
- `SM3`: official-submission-aligned Lore implementation branch.
- `SHAKE256`: comparison / legacy SHAKE branch.
