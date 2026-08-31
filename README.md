<!--
  ~ Copyright 2026 QHARM
  ~
  ~ Licensed under the Apache License, Version 2.0 (the "License");
  ~ you may not use this file except in compliance with the License.
  ~ You may obtain a copy of the License at
  ~
  ~ http://www.apache.org/licenses/LICENSE-2.0
  ~
  ~ Unless required by applicable law or agreed to in writing, software
  ~ distributed under the License is distributed on an "AS IS" BASIS,
  ~ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  ~ See the License for the specific language governing permissions and
  ~ limitations under the License.
-->

# EigenBridge

There are two quantum solvers. VQD can return several eigenvalues; QAOA returns only the ground state.

VQD -> `run_vqd_eigensolver(matrix, k)` -> `solve_vqd` -> lowest `k` eigenvalues (`k` defaults to all)
QAOA -> `run_qaoa_eigensolver(matrix, use_noise=false)` -> `solve_qaoa` -> ground state only

**Uncertainties (`uq_values` / `uq_vectors`)**
- Noiseless solvers (LAPACK, VQD, noiseless QAOA): `uq_* = 0`
- Noisy QAOA (`use_noise=true`): at the final circuit, `uq_values[0] = |E_noisy − E_exact|`; Unused slots and `uq_vectors` stay 0

The noisy QAOA unit test checks that `uq_values[0] = |E_noisy − E_exact|` (at the final circuit) and `|λ_noisy − λ_LAPACK|` agree within a factor of 10 (ratio < 10).
