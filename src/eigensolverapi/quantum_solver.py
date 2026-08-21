import numpy as np
from qiskit import QuantumCircuit
from qiskit.quantum_info import SparsePauliOp, Statevector
from qiskit.circuit.library import EfficientSU2, QAOAAnsatz
from qiskit_algorithms import VQD
from qiskit_algorithms.algorithm_job import AlgorithmJob
from qiskit_algorithms.minimum_eigensolvers import VQE
from qiskit_algorithms.optimizers import COBYLA, SLSQP
from qiskit_algorithms.state_fidelities import BaseStateFidelity, StateFidelityResult
from qiskit.primitives import StatevectorEstimator


class _ExactStatevectorFidelity(BaseStateFidelity):

    def create_fidelity_circuit(self, circuit_1, circuit_2):
        return circuit_1.copy()

    def _run(self, circuits_1, circuits_2, values_1=None, values_2=None, *, shots=None):
        if isinstance(circuits_1, QuantumCircuit):
            circuits_1 = [circuits_1]
        if isinstance(circuits_2, QuantumCircuit):
            circuits_2 = [circuits_2]
        values_1 = self._preprocess_values(circuits_1, values_1)
        values_2 = self._preprocess_values(circuits_2, values_2)
        # _preprocess_values returns a single empty row when values is None
        if len(values_1) == 1 and len(circuits_1) > 1:
            values_1 = list(values_1) * len(circuits_1)
        if len(values_2) == 1 and len(circuits_2) > 1:
            values_2 = list(values_2) * len(circuits_2)

        def _call():
            fidelities = []
            for circuit_1, circuit_2, val_1, val_2 in zip(
                circuits_1, circuits_2, values_1, values_2
            ):
                bound_1 = circuit_1.assign_parameters(val_1) if val_1 else circuit_1
                bound_2 = circuit_2.assign_parameters(val_2) if val_2 else circuit_2
                overlap = Statevector(bound_1).inner(Statevector(bound_2))
                fidelities.append(float(np.abs(overlap) ** 2))
            return StateFidelityResult(
                fidelities=fidelities,
                raw_fidelities=fidelities,
                metadata=[{} for _ in fidelities],
                shots=0,
            )

        return AlgorithmJob(_call)


def _statevector_to_real_eigenvector(circuit, parameters, n):
    # Turn the VQD circuit into a real unit vector of length n, and dropping extra padding.
    bound = circuit.assign_parameters(parameters)
    vec = np.asarray(Statevector(bound).data[:n], dtype=complex)
    idx = int(np.argmax(np.abs(vec)))
    peak = vec[idx]
    if np.abs(peak) > 0:
        vec = vec * np.conj(peak) / np.abs(peak)
    vec = np.real(vec)
    if vec[idx] < 0:
        vec = -vec
    norm = np.linalg.norm(vec)
    if norm > 0:
        vec = vec / norm
    return vec


def _matrix_to_observable(flat_matrix, n):
    # Reshape into an n x n NumPy array and pad to the next power of two.
    mat = np.array(flat_matrix, dtype=float).reshape((n, n))
    next_pow2 = 1 << (n - 1).bit_length()
    if next_pow2 != n:
        padded_mat = np.zeros((next_pow2, next_pow2))
        padded_mat[:n, :n] = mat
        penalty = 10.0 * max(1.0, float(np.max(np.abs(mat))))
        for i in range(n, next_pow2):
            padded_mat[i, i] = penalty
        mat = padded_mat
    return SparsePauliOp.from_operator(mat), next_pow2


def _make_estimator(use_noise):
    if not use_noise:
        return StatevectorEstimator(), None
    try:
        from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
        from qiskit_aer import AerSimulator
        from qiskit_aer.noise import NoiseModel
        from qiskit_aer.primitives import EstimatorV2 as AerEstimatorV2
        from qiskit_ibm_runtime.fake_provider import FakeManilaV2
    except ImportError as exc:
        raise ImportError(
            "Noisy QAOA requires qiskit-aer and qiskit-ibm-runtime. "
            "Install with: pip install qiskit-aer qiskit-ibm-runtime"
        ) from exc

    fake_backend = FakeManilaV2()
    noise_model = NoiseModel.from_backend(fake_backend)
    aer_simulator = AerSimulator(noise_model=noise_model)
    pass_manager = generate_preset_pass_manager(
        optimization_level=1, backend=aer_simulator
    )
    
    estimator = AerEstimatorV2(
        options={"backend_options": {"noise_model": noise_model}}
    )
    return estimator, pass_manager


def _expect_energy(estimator, circuit, observable, parameters):
    op = observable
    if getattr(circuit, "layout", None) is not None:
        op = observable.apply_layout(circuit.layout)
    evs = np.asarray(
        estimator.run([(circuit, op, parameters)]).result()[0].data.evs,
        dtype=float,
    ).reshape(-1)
    return float(np.real(evs[0]))


def _energy_diff_uncertainty(circuit, parameters, observable, noisy_estimator):

    exact_energy = _expect_energy(
        StatevectorEstimator(), circuit, observable, parameters
    )
    noisy_energy = _expect_energy(
        noisy_estimator, circuit, observable, parameters
    )
    # Predicted noise = |E_noisy - E_exact| on the final circuit
    return abs(noisy_energy - exact_energy)


def solve_vqd(flat_matrix, n, k=None):
    """
    Takes a flat matrix of size n*n, and returns the lowest k eigenvalues
    and matching eigenvectors using VQD.
    """
    if k is None:
        k = n
    if k < 1 or k > n:
        raise ValueError(f"k must satisfy 1 <= k <= n, got k={k}, n={n}")

    observable, next_pow2 = _matrix_to_observable(flat_matrix, n)
    num_qubits = int(np.log2(next_pow2))

    ansatz = EfficientSU2(num_qubits, reps=1, entanglement="linear")
    optimizer = SLSQP(maxiter=1000, ftol=1e-9)
    estimator = StatevectorEstimator()
    fidelity = _ExactStatevectorFidelity()

    # Overlap weights on the scale of the original matrix, not the padding.
    beta = 10.0 * max(1.0, float(np.max(np.abs(np.array(flat_matrix, dtype=float)))))
    betas = np.full(k, beta)
    rng = np.random.default_rng(0)
    if k == 1:
        initial_points = np.zeros(ansatz.num_parameters)
    else:
        initial_points = [np.zeros(ansatz.num_parameters)]
        for _ in range(k - 1):
            initial_points.append(rng.uniform(-np.pi, np.pi, ansatz.num_parameters))

    vqd = VQD(estimator, fidelity, ansatz, optimizer, k=k, betas=betas)
    vqd.initial_point = initial_points

    result = vqd.compute_eigenvalues(observable)

    eigenvalues = [float(np.real(e)) for e in result.eigenvalues]
    eigenvectors = np.zeros((n, n), dtype=float)
    for i in range(k):
        eigenvectors[:, i] = _statevector_to_real_eigenvector(
            result.optimal_circuits[i], result.optimal_points[i], n
        )

    return eigenvalues, eigenvectors.ravel().tolist()


def solve_qaoa(flat_matrix, n, use_noise=False, reps=3):
    """
    Takes a flat matrix of size n*n, and returns the ground eigenvalue,
    and matching eigenvector using QAOA.
    """
    observable, _ = _matrix_to_observable(flat_matrix, n)
    ansatz = QAOAAnsatz(observable, reps=reps)
    estimator, pass_manager = _make_estimator(use_noise)
    if use_noise:
        optimizer = COBYLA(maxiter=50)
        vqe = VQE(estimator, ansatz, optimizer, transpiler=pass_manager)
    else:
        optimizer = SLSQP(maxiter=1000, ftol=1e-9)
        vqe = VQE(estimator, ansatz, optimizer)
    rng = np.random.default_rng(2)
    vqe.initial_point = rng.uniform(0, np.pi, ansatz.num_parameters)

    result = vqe.compute_minimum_eigenvalue(observable)

    eigenvalues = [float(np.real(result.eigenvalue))]
    eigenvectors = np.zeros((n, n), dtype=float)
    eigenvectors[:, 0] = _statevector_to_real_eigenvector(
        result.optimal_circuit, result.optimal_point, n
    )

    uq_values = [0.0] * len(eigenvalues)
    uq_vectors = [0.0] * (n * n)
    if use_noise:
        uq_values[0] = _energy_diff_uncertainty(
            result.optimal_circuit,
            result.optimal_point,
            observable,
            estimator
        )

    return eigenvalues, eigenvectors.ravel().tolist(), uq_values, uq_vectors


# Quick Test
if __name__ == "__main__":
    # The same 3x3 matrix from the C++ test file
    test_matrix = [3.0, 5.0, 2.0, 5.0, 1.0, 3.0, 2.0, 3.0, 2.0]
    values, vectors = solve_vqd(test_matrix, 3)
    print(f"VQD Eigenvalues: {values}")
    print(f"VQD Eigenvectors: {vectors}")
    qaoa_values, qaoa_vectors, qaoa_uq_v, qaoa_uq_vec = solve_qaoa(test_matrix, 3)
    print(f"QAOA Eigenvalue: {qaoa_values}")
    print(f"QAOA Eigenvector: {qaoa_vectors}")
    print(f"QAOA uq_values: {qaoa_uq_v}")
    try:
        qaoa_noisy_values, qaoa_noisy_vectors, noisy_uq_v, noisy_uq_vec = solve_qaoa(
            test_matrix, 3, use_noise=True
        )
        print(f"QAOA Eigenvalue (noise): {qaoa_noisy_values}")
        print(f"QAOA Eigenvector (noise): {qaoa_noisy_vectors}")
        print(f"QAOA uq_values (noise): {noisy_uq_v}")
    except ImportError as exc:
        print(f"Skipping noisy QAOA demo: {exc}")
