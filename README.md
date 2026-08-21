# EigensolverAPI (name in progress)

There are two quantum solvers. VQD can return several eigenvalues; QAOA returns only the ground state.

**Setup**
```bash
python3 -m venv .venv && source .venv/bin/activate
pip install qiskit qiskit-algorithms numpy scipy pybind11
brew install lapack   # macOS only

cd build
cmake -DLAPACKE_DIR=/opt/homebrew/opt/lapack -DBUILD_TESTING=ON ..
make
PYTHONPATH="$VIRTUAL_ENV/lib/python3.*/site-packages" ./unit_test_eigensolverapi
```
**Noisy QAOA**
```bash
pip install qiskit-aer qiskit-ibm-runtime
```

VQD -> `run_vqd_eigensolver(matrix, k)` -> `solve_vqd` -> lowest `k` eigenvalues (`k` defaults to all)
QAOA -> `run_qaoa_eigensolver(matrix, use_noise=false)` -> `solve_qaoa` -> ground state only

**Uncertainties (`uq_values` / `uq_vectors`)**
- Noiseless solvers (LAPACK, VQD, noiseless QAOA): `uq_* = 0`
- Noisy QAOA (`use_noise=true`): at the final circuit, `uq_values[0] = |E_noisy − E_exact|`; Unused slots and `uq_vectors` stay 0

The noisy QAOA unit test checks that `uq_values[0] = |E_noisy − E_exact|` (at the final circuit) and `|λ_noisy − λ_LAPACK|` agree within a factor of 10 (ratio < 10). 