# EigensolverAPI (name in progress)

TODO: Write me.

## Quantum Solver (VQE)

**Setup**
```bash
python3 -m venv .venv && source .venv/bin/activate
pip install qiskit qiskit-algorithms numpy scipy pybind11
brew install lapack   # macOS only

cd build
cmake -DLAPACKE_DIR=/opt/homebrew/opt/lapack -DBUILD_TESTING=ON ..
make
PYTHONPATH="$VIRTUAL_ENV/lib/python3.*/site-packages" ./unit_test_eigensolverapi "Eigensolverapi quantum"
```
