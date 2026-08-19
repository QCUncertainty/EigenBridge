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

VQD -> `run_vqd_eigensolver(matrix, k)` -> `solve_vqd` -> lowest `k` eigenvalues (`k` defaults to all) 
QAOA -> `run_qaoa_eigensolver(matrix)` -> `solve_qaoa` -> ground state only 

QAOA is noiseless by default. For a noisy Aer demo (`use_noise=true`):

```bash
pip install qiskit-aer qiskit-ibm-runtime
```

Since noisy results are approximate, the tests only exist for the noiseless solver.
