# EigenBridge

There are two quantum solvers. VQD can return several eigenvalues; QAOA returns only the ground state.

VQD -> `run_vqd_eigensolver(matrix, k)` -> `solve_vqd` -> lowest `k` eigenvalues (`k` defaults to all)
QAOA -> `run_qaoa_eigensolver(matrix, use_noise=false)` -> `solve_qaoa` -> ground state only

**Uncertainties (`uq_values` / `uq_vectors`)**
- Noiseless solvers (LAPACK, VQD, noiseless QAOA): `uq_* = 0`
- Noisy QAOA (`use_noise=true`): at the final circuit, `uq_values[0] = |E_noisy − E_exact|`; Unused slots and `uq_vectors` stay 0

The noisy QAOA unit test checks that `uq_values[0] = |E_noisy − E_exact|` (at the final circuit) and `|λ_noisy − λ_LAPACK|` agree within a factor of 10 (ratio < 10).
